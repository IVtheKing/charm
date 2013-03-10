///////////////////////////////////////////////////////////////////////////////
//	
//						Copyright 2009-2013 xxxxxxx, xxxxxxx
//	File:	os_task.c
//	Author:	Bala B. (bhat.balasubramanya@gmail.com)
//	Description: OS Task Related routines
//	
///////////////////////////////////////////////////////////////////////////////

#include "os_core.h"
#include "os_queue.h"
#include "os_timer.h"
#include "util.h"

// function prototype declaration
static BOOL ValidateNewThread(UINT32 period, UINT32 budget);

#ifdef _USE_STD_LIBS
	#define FAULT(x, ...) printf(x, ...);
#else
	#define FAULT(x, ...)
#endif

// Macro for calculating the thread usage
#define CALC_THREAD_CPU_USAGE(period, budget) ((FP32)budget / (FP32)period)
#define MIN(a,b) ((a)<(b)?(a):(b))
#define ALIGNED_ARRAY(ptr) ASSERT((int) ptr % 8 == 0)

// Variable declaration/definition
static UINT32 g_task_id_counter = 0;
static FP32 g_total_allocated_cpu = 0.0;

extern _OS_Queue g_ready_q;
extern _OS_Queue g_wait_q;
extern _OS_Queue g_ap_ready_q;
extern _OS_Queue g_block_q;
extern volatile UINT64 g_global_time;	// This variable gets updated everytime the Timer ISR is called.
extern volatile UINT64 g_next_wakeup_time; // This variable holds the next scheduled wakeup time in uSecs
extern void _OS_ReSchedule();
extern void _OS_SetAlarm(OS_PeriodicTask *task, UINT64 abs_time_in_us, BOOL is_new_job, BOOL update_timer);

UINT32 *_OS_BuildTaskStack(UINT32 * stack_ptr, void (*task_function)(void *), void * arg, UINT32 system_mode);
static void TaskEntryMain(void *pdata);
static void AperiodicTaskEntry(void *pdata);
void _OS_ReSchedule();

///////////////////////////////////////////////////////////////////////////////
// OS_CreatePeriodicTask - API to create periodic tasks
///////////////////////////////////////////////////////////////////////////////
OS_Error OS_CreatePeriodicTask(
	UINT32 period_in_us,
	UINT32 deadline_in_us,
	UINT32 budget_in_us,
	UINT32 phase_shift_in_us,
	UINT32 *stack,
	UINT32 stack_size_in_bytes,
#if OS_WITH_TASK_NAME==1
	const INT8 * task_name,
#endif // OS_WITH_TASK_NAME			
	OS_PeriodicTask *task,
	void (*periodic_entry_function)(void *pdata),
	void *pdata)
{
	FP32 this_thread_cpu;
	UINT32 stack_size;
	UINT32 intsts;

	if(!task)
	{
	    FAULT("Invalid Task");
		return INVALID_TASK;
	}

    if(!periodic_entry_function || !stack)
	{
	    FAULT("One or more invalid %s arguments", "task");
		return INVALID_ARG;
	}

	if(stack_size_in_bytes < ONE_KB / 4)	// Validate for minimum stack size 256 bytes
	{
		FAULT("Stack size should be at least %d bytes", ONE_KB / 4);
		return INSUFFICIENT_STACK;
	}

	if(period_in_us < TASK_MIN_PERIOD)
	{
		FAULT("The period should be greater than %d uSec\n", TASK_MIN_PERIOD);
		return INVALID_PERIOD;
	}
	if(deadline_in_us < budget_in_us)
	{
		FAULT("The deadline should be at least as much as its budget\n");
		return INVALID_DEADLINE;
	}
	if(deadline_in_us > period_in_us)
	{
		FAULT("The deadline should be less than or equal to the period\n");
		return INVALID_DEADLINE;
	}
	if(budget_in_us < TASK_MIN_BUDGET)
	{
		FAULT("The budget should be greater than %d uSec\n", TASK_MIN_BUDGET);
		return INVALID_BUDGET;
	}

	// Ensure that the stack is 8 byte aligned
	//ALIGNED_ARRAY(stack);

	// Conver the stack_size_in_bytes into number of words
	stack_size = stack_size_in_bytes >> 2; 

	task->attributes = (PERIODIC_TASK | USER_TASK);
#if OS_WITH_VALIDATE_TASK==1
	task->signature = TASK_SIGNATURE;
#endif
#if OS_WITH_TASK_NAME==1
	strncpy(task->name, task_name, OS_TASK_NAME_SIZE);
#endif // OS_WITH_TASK_NAME	

	task->budget = budget_in_us;
	task->period = period_in_us;
	task->deadline = deadline_in_us;
	task->phase = phase_shift_in_us;
	task->stack = stack;
	task->stack_size = stack_size;
	task->top_of_stack = stack + stack_size;	// Stack grows bottom up
	task->task_function = periodic_entry_function;
	task->pdata = pdata;
	task->remaining_budget = 0;
	task->accumulated_budget = 0;
	task->stored_release_time = 0;
	task->exec_count = 0;
	task->TBE_count = 0;
	task->dline_miss_count = 0;
	//task->next_release_time = phase_shift_in_us;
	task->alarm_time = 0;

	OS_ENTER_CRITICAL(intsts);
	//if(!ValidateNewThread(period_in_us, budget_in_us))
	if(!ValidateNewThread(MIN(period_in_us, deadline_in_us), budget_in_us))
	{
		OS_EXIT_CRITICAL(intsts); 
		FAULT("The total allocated CPU exceeds %d%", 100);
		return EXCEEDS_MAX_CPU;
	}

	task->id = ++g_task_id_counter;
	
	// Calculate the CPU usage of the current thread
	this_thread_cpu = CALC_THREAD_CPU_USAGE(MIN(period_in_us, deadline_in_us), budget_in_us);
	
	// Update the g_total_allocated_cpu
	g_total_allocated_cpu += this_thread_cpu;
	
	OS_EXIT_CRITICAL(intsts); 	// Exit the critical section

	// Build a Stack for the new thread
	task->top_of_stack = _OS_BuildTaskStack(task->top_of_stack, 
		TaskEntryMain, task, FALSE);

	OS_ENTER_CRITICAL(intsts);	// Enter critical section

	_OS_QueueInsert(&g_wait_q, task, phase_shift_in_us);		

	OS_EXIT_CRITICAL(intsts); 	// Exit the critical section

	if(_OS_IsRunning)
	{
		_OS_ReSchedule();
	}

  	return SUCCESS; 
}

///////////////////////////////////////////////////////////////////////////////
// The task creation routine for Aperiodic tasks
///////////////////////////////////////////////////////////////////////////////
OS_Error OS_CreateAperiodicTask(UINT16 priority, 
	UINT32 * stack, UINT32 stack_size_in_bytes,
#if OS_WITH_TASK_NAME==1
	const INT8 * task_name,
#endif //OS_WITH_TASK_NAME			
	OS_AperiodicTask * task,
	void(* task_entry_function)(void * pdata),
	void * pdata)
{
	UINT32 stack_size;
	UINT32 intsts;

	if(!task)
	{
		FAULT("Invalid Task");
		return INVALID_TASK;
	}

	if(!task_entry_function || !stack)
	{
		FAULT("One or more invalid %s arguments", "task");
		return INVALID_ARG;
	}

	if(stack_size_in_bytes < ONE_KB / 4) // Validate for minimum stack size 256 bytes
	{
		FAULT("Stack size should be at least %d bytes", ONE_KB / 4);
		return INSUFFICIENT_STACK;
	}

	if(priority > MIN_PRIORITY)
	{
		FAULT("The priority value %d should be within 0 and %d", priority, MIN_PRIORITY);
		return INVALID_PRIORITY;
	}


	// Conver the stack_size_in_bytes into number of words
	stack_size = stack_size_in_bytes >> 2;

	task->stack = stack;
	task->stack_size = stack_size;
	task->top_of_stack = stack + stack_size; // Stack grows bottom up
	task->top_of_stack = _OS_BuildTaskStack(task->top_of_stack, AperiodicTaskEntry, task, FALSE);
	task->task_function = task_entry_function;
	task->pdata = pdata;
	task->priority = priority;
	task->attributes = (APERIODIC_TASK | USER_TASK);
#if OS_WITH_VALIDATE_TASK==1
	task->signature = TASK_SIGNATURE;
#endif
#if OS_WITH_TASK_NAME==1
	strncpy(task->name, task_name, OS_TASK_NAME_SIZE);
#endif // OS_WITH_TASK_NAME	

	OS_ENTER_CRITICAL(intsts); // Enter critical section
	task->id = ++g_task_id_counter;
	_OS_QueueInsert(&g_ap_ready_q, task, priority); //add task to aperiodic wait queue
	OS_EXIT_CRITICAL(intsts); // Exit the critical section
	
	if(_OS_IsRunning)
	{
		_OS_ReSchedule();
	}

	return SUCCESS;
}


///////////////////////////////////////////////////////////////////////////////
// Validation for sufficient CPU Budget
// ASSUMPTION: The interrupts are disabled when this function is invoked
///////////////////////////////////////////////////////////////////////////////
static BOOL ValidateNewThread(UINT32 period, UINT32 budget)
{
	// The current threads budget in terms of CPU usage %ge
	FP32 this_thread_cpu = CALC_THREAD_CPU_USAGE(period, budget);

	// Dont allow allocation of more than 90% of thr CPU
	if(g_total_allocated_cpu + this_thread_cpu >= 1.0)
	{
		return FALSE;
	}

	return TRUE;
}

///////////////////////////////////////////////////////////////////////////////
// This is the main entry function for all periodic functions
///////////////////////////////////////////////////////////////////////////////
static void TaskEntryMain(void *pdata)
{
	OS_PeriodicTask * task = (OS_PeriodicTask *) pdata;
	UINT32 intsts;

#if OS_WITH_VALIDATE_TASK==1
	// Validate the task
	if(task->signature != TASK_SIGNATURE) {
		panic("Invalid Task %p", task);
	}
#endif // OS_WITH_VALIDATE_TASK

	while(1)
	{
		// Call the thread handler function
		task->task_function(task->pdata);

		OS_ENTER_CRITICAL(intsts);		
		task->exec_count++;

		// Suspend the current task. This task will be automatically 
		// woken up by the alarm.
		_OS_QueueDelete(&g_ready_q, task);
		if(task->alarm_time == g_next_wakeup_time)
		{
			// If the next wakeup time is same as the alarm time for the 
			// current task, just invalidate the next wakeup time so that
			// it is set again below
			g_next_wakeup_time = 0xFFFFFFFFFFFFFFFF;
		}
		
		// The accumulated_budget and remaining_budget will be updated by these calls
		if(task->deadline == task->period)
		{
			_OS_SetAlarm(task, task->alarm_time, FALSE, TRUE);
		}
		else
		{
			_OS_SetAlarm(task, task->alarm_time + task->period - task->deadline, FALSE, TRUE);
		}
		OS_EXIT_CRITICAL(intsts);

		// Now call reschedule function
		_OS_ReSchedule();	
	}
}

static void AperiodicTaskEntry(void *pdata)
{
	OS_AperiodicTask * task = (OS_AperiodicTask *) pdata;
	UINT32 intsts;

	// Call the thread handler function
	task->task_function(task->pdata);

	// If this function ever returns, just block this task by adding it to
	// block q
	OS_ENTER_CRITICAL(intsts);		
	_OS_QueueDelete(&g_ap_ready_q, task);

	// Insert into block q
	_OS_QueueInsert(&g_block_q, task, task->priority);
	OS_EXIT_CRITICAL(intsts);

	// Now call reschedule function
	_OS_ReSchedule();
}

///////////////////////////////////////////////////////////////////////////////
// The following function gets the total time taken by the current
// thread since the thread has begun. Note that this is not the global 
// time, this is just the time taken from only the current thread.
// Note that this function is defined only for periodic tasks
///////////////////////////////////////////////////////////////////////////////
UINT64 OS_GetThreadElapsedTime()
{
    UINT64 thread_elapsed_time = 0;
	UINT64 old_global_time;

	// First get the current task which is running...
	OS_PeriodicTask * task = (OS_PeriodicTask *)OS_GetCurrentTask();
		
	if(task && IS_PERIODIC_TASK(task)) 
	{
		do
		{
			old_global_time = g_global_time;
			thread_elapsed_time = task->accumulated_budget + _OS_GetTimerValue_us();
		} 
		while(old_global_time != g_global_time); // To ensure that the timer has not expired since we have read both g_global_time and OSW_GetTime		
	}
		
	return thread_elapsed_time;
}
