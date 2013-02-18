///////////////////////////////////////////////////////////////////////////////
//	
//						Copyright 2009-2013 xxxxxxx, xxxxxxx
//	File:	os_queue.c
//	Author: Bala B.
//	Description: Main Scheduling function implementation
//	
///////////////////////////////////////////////////////////////////////////////

#include "os_core.h"
#include "os_queue.h"
#include "os_timer.h"
#include "util.h"
#include "mmu.h"

_OS_Queue g_ready_q;
_OS_Queue g_wait_q;
_OS_Queue g_ap_ready_q;
_OS_Queue g_block_q;

// This global variable can be accessed from outside
volatile BOOL _OS_IsRunning = FALSE;

volatile UINT64 g_global_time;		// This variable gets updated everytime the Timer ISR is called.
volatile UINT64 g_next_wakeup_time; // This variable holds the next scheduled wakeup time in uSecs
volatile void * g_current_task;

OS_AperiodicTask g_TCB_idle_task;	// A TCB for the idle task
UINT32 g_idle_task_stack [OS_IDLE_TASK_STACK_SIZE];

#ifdef OS_ENABLE_CPU_STATS
OS_PeriodicTask g_stat_task;		// A TCB for the idle task
UINT32 g_stat_task_stack [OS_STAT_TASK_STACK_SIZE];
#endif

// External functions used in here
extern void _OS_InitInterrupts();
extern void _OS_ContextRestore(void *new_task);
extern void _OS_ContextSw(void * new_task);
extern UINT32 *_OS_BuildTaskStack(UINT32 * stack_ptr, 
							void (*task_function)(void *), 
							void * arg, BOOL system_task);

void _OS_Timer0ISRHook(void *arg);
void _OS_Timer1ISRHook(void *arg);
void _OS_SetAlarm(OS_PeriodicTask *task, 
					UINT64 abs_time_in_us, 
					BOOL is_new_job,
					BOOL update_timer);
static void _OS_SetNextTimeout(void);
static void _OS_UpdateTaskBudget (OS_PeriodicTask *task);
static void _OS_StatisticsFn(void * ptr);
void _OS_ReSchedule();

// TODO: Make all unnecessary functions as static
// Static methods
static void _OS_idle_task(void * ptr);

// Variables to keep track of the idle task execution
UINT32 g_idle_max_count = 0;
volatile UINT32 g_idle_count = 0;
volatile FP64 _OS_CPUUsage;

///////////////////////////////////////////////////////////////////////////////
// The following funcstion starts the OS scheduling
///////////////////////////////////////////////////////////////////////////////
void OS_Start()
{
	// Check if the OS is already running
	if(!_OS_IsRunning)
	{
		// Reset the global timer
		g_global_time = 0;
		g_next_wakeup_time = 0;

		// Reset the current task
		g_current_task = 0;
		
		// Initialize the IDLE task TCB. This is done here so that the 
		g_TCB_idle_task.id = 0;
		g_TCB_idle_task.priority = MIN_PRIORITY + 1;
		g_TCB_idle_task.top_of_stack = 0;
		g_TCB_idle_task.type = APERIODIC_TASK;
		g_TCB_idle_task.mode = SYSTEM_TASK;
#if OS_WITH_VALIDATE_TASK==1
		g_TCB_idle_task.signature = TASK_SIGNATURE;
#endif
#if OS_WITH_TASK_NAME==1
		strcpy(g_TCB_idle_task.name, "IDLE");
#endif // OS_WITH_TASK_NAME	

		g_TCB_idle_task.top_of_stack = g_idle_task_stack + OS_IDLE_TASK_STACK_SIZE;
		g_TCB_idle_task.top_of_stack = _OS_BuildTaskStack(g_TCB_idle_task.top_of_stack, 
				_OS_idle_task, &g_TCB_idle_task, FALSE);
		
		// Add the idle task to the Aperiodic task queue
		_OS_QueueInsert(&g_ap_ready_q, &g_TCB_idle_task, MIN_PRIORITY + 1);		

#if OS_ENABLE_CPU_STATS == 1
		OS_CreatePeriodicTask(STAT_TASK_PERIOD, STAT_TASK_PERIOD, 
			STAT_TASK_PERIOD / 50, 0, g_stat_task_stack, 
			sizeof(g_stat_task_stack), "STATISTICS", &g_stat_task, 
			_OS_StatisticsFn, 0);
#else
		_OS_QueuePeek(&g_wait_q, NULL, &g_next_wakeup_time);
#endif

		// Start scheduling by creating the first interrupt
		_OS_UpdateTimer(OS_FIRST_SCHED_DELAY);	// First timer after 10ms

		_OS_IsRunning = TRUE;	// No need to protect this line and the above
		// 'if' condition as this is called from the initialization for the
		// first time and there is only single thread at that point of time.

		// Start processing interrupts
		_OS_InitInterrupts();
		
		// Call reschedule. 
		_OS_ReSchedule();
		
		// TODO: At this point, I can shutdown the device.
		
		// We would never return from the above call. 
		// The current stack continues as SVC stack handling all interrupts
		panic("Unexpected System stack unwind");
	}
}

///////////////////////////////////////////////////////////////////////////////
// IDLE Task
///////////////////////////////////////////////////////////////////////////////
static void _OS_idle_task(void * ptr)
{
	while(1)
	{
		// Wait for interrupt at lower power state
		MMU_WaitForInterrupt();
		g_idle_count++;
	}
}

///////////////////////////////////////////////////////////////////////////////
// The following function schedules the first task from the ready queue 
///////////////////////////////////////////////////////////////////////////////
void _OS_ReSchedule()
{
	void * task;
	UINT32 intsts;

	OS_ENTER_CRITICAL(intsts);	// Enter critical section

	if(_OS_QueuePeek(&g_ready_q, (void**) &task, 0) != SUCCESS)
	{
		_OS_QueuePeek(&g_ap_ready_q, (void**) &task, 0);
	}

	_OS_UpdateTaskBudget(task);
	
	KlogStr(KLOG_CONTEXT_SWITCH, "ContextSW To - ", ((OS_AperiodicTask *)task)->name);
		
	OS_EXIT_CRITICAL(intsts);	// Exit critical section
	_OS_ContextSw(task);	// This has the affect of g_current_task = task;
}

///////////////////////////////////////////////////////////////////////////////
// This function is invoked when the timer expires. This function should calculate
// and set the new delay required for the next timer expiry in number of
// microseconds.
///////////////////////////////////////////////////////////////////////////////
void _OS_Timer0ISRHook(void *arg)
{
	OS_PeriodicTask * task;
	UINT64 new_time = 0;
	
	KlogStr(KLOG_OS_TIMER_ISR, "OS Timer ISR - ", ((OS_AperiodicTask *)arg)->name);
		
	// Acknowledge the interrupt
	_OS_TimerInterrupt(0);

	g_global_time = g_next_wakeup_time;	
	g_next_wakeup_time = 0xFFFFFFFFFFFFFFFF;
	
	while(_OS_QueuePeek(&g_ready_q, (void**) &task, &new_time) == SUCCESS)
	{
		// First Check the deadline expiry for each job in the ready queue
		if(new_time == g_global_time)
		{
			// Now get the new task from the queue.
			_OS_QueueGet(&g_ready_q, (void**) &task, 0);

			// Deadline has expired
			// TODO: Take necessary action for deadline miss
			task->dline_miss_count ++;

			// If the relative deadline and the period are same, then reintroduce a job
			// in the ready queue because the next period for the task is to begin now
			if(task->deadline == task->period)
			{
				// ReSet the remaining budget to full budget
				task->remaining_budget = task->budget;
				_OS_SetAlarm(task, g_global_time + task->period, TRUE, FALSE);
			}
			// Else put it in the wait_q so that we can reintroduce the task to ready
			// when the next period begins some time later.
			else
			{
				_OS_SetAlarm(task, g_global_time + task->period - task->deadline, FALSE, FALSE);
			}

			continue;
		}
		else
		{
			break;
		}
	}

	// Now consider new jobs to be introduced from the wait queue
	while(_OS_QueuePeek(&g_wait_q, (void**) &task, &new_time) == SUCCESS)
	{
		// First Check the deadline expiry for each job in the ready queue
		if(new_time == g_global_time)
		{
			// Now get the new task from the queue.
			_OS_QueueGet(&g_wait_q, (void**) &task, 0);
			
			// ReSet the remaining budget to full budget
			task->remaining_budget = task->budget;
			_OS_SetAlarm(task, g_global_time + task->deadline, TRUE, FALSE);
			continue;
		}
		else
		{
			break;				
		}
	}

	// Update the timeout
	_OS_SetNextTimeout();

	// Select the next task to schedule. First from the periodic ready queue
	// then from the Aperiodic ready queue
	if(_OS_QueuePeek(&g_ready_q, (void**) &task, 0) != SUCCESS)
	{
		_OS_QueuePeek(&g_ap_ready_q, (void**) &task, 0);
	}
	
	_OS_UpdateTaskBudget(task);
	
	KlogStr(KLOG_CONTEXT_SWITCH, "ContextSW To - ", task->name);
	
	_OS_ContextRestore(task);	// This has the affect of g_current_task = task;					
}

///////////////////////////////////////////////////////////////////////////////
// ISR for the Timer1 (Budget)Timer
// Some thread has exceded its budget. This function handles this case.
///////////////////////////////////////////////////////////////////////////////
void _OS_Timer1ISRHook(void *arg)
{
	// The argument in this case is the g_current_task before the interrupt
	OS_PeriodicTask * task = (OS_PeriodicTask *)arg;
	UINT32 timeout = 0;

	KlogStr(KLOG_BUDGET_TIMER_ISR, "Budget Timer ISR - ", task->name);
	
	_OS_TimerInterrupt(1);

	if(task && (task->type == PERIODIC_TASK ))
	{
		// The thread budget has exceeded
		task->TBE_count++;

		// TODO: Raise the flag saying that the thread has exceeded its
		// budget
		
		// Add to the thread's accumulated budget
		task->accumulated_budget += task->remaining_budget;
		task->remaining_budget = 0;

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
		if(task->deadline == task->period)
		{
			_OS_SetAlarm(task, task->alarm_time, FALSE, FALSE);
		}
		else
		{
			_OS_SetAlarm(task, task->alarm_time + task->period - task->deadline, FALSE, FALSE);
		}
		
		// Update the Timer 0 timeout
		_OS_SetNextTimeout();

		// Select the next task to schedule. First from the periodic ready queue
		// then from the Aperiodic ready queue
		if(_OS_QueuePeek(&g_ready_q, (void**) &task, 0) != SUCCESS)
		{
			_OS_QueuePeek(&g_ap_ready_q, (void**) &task, 0);
		}

		// Set the budget timer
		timeout = (task->type == PERIODIC_TASK) ? task->remaining_budget : 0;
		
		Klog64(KLOG_DEBUG_BUDGET, "Set Budget - ", timeout);
		
		_OS_SetBudgetTimer(timeout);			
		
		KlogStr(KLOG_CONTEXT_SWITCH, "ContextSW To - ", task->name);
		
		_OS_ContextRestore(task);	// This has the affect of g_current_task = task;					
	}
}

///////////////////////////////////////////////////////////////////////////////                                           TASK SWITCH HOOK
//
// Description: This function is called before a task switch is performed.  
// This allows you to perform other operations during a context switch.
//
// Arguments  : none
//
// Note(s)    : 1) Interrupts shall be disabled before making this function call
///////////////////////////////////////////////////////////////////////////////
void _OS_UpdateTaskBudget (OS_PeriodicTask *task)
{
	OS_PeriodicTask * old_task = (OS_PeriodicTask *)g_current_task;
	UINT32 old_time = 0, new_timeout;

	// If the new and the old tasks are same, then just return.
	if(old_task == task) return;

	// Check if the new task is a valid periodic thread. 
	new_timeout = (task->type == PERIODIC_TASK) ? task->remaining_budget : 0;

	Klog64(KLOG_DEBUG_BUDGET, "Set Budget - ", new_timeout);
	
	old_time = _OS_SetBudgetTimer(new_timeout);			

	// Check if the old task is a valid PERIODIC thread. 
	if(old_task->type == PERIODIC_TASK)
	{
		old_task->remaining_budget -= old_time;

		// Add to the thread's accumulated budget
		old_task->accumulated_budget += old_time;
	}
}

///////////////////////////////////////////////////////////////////////////////
// This function puts the current thread to sleep and sets up an alarm for
// waking up at a specified time instant (relative to the starting time).
// ASSUMPTION: The interrupts are disabled when this function is invoked
///////////////////////////////////////////////////////////////////////////////
void _OS_SetAlarm(OS_PeriodicTask *task, 
	UINT64 abs_time_in_us, 
	BOOL is_new_job,
	BOOL update_timer)
{
	// If it was a past time, then return immediately.
	if(g_global_time >= abs_time_in_us) 
	{
		return;
	}

	// Insert the task into the g_ready_q / g_wait_q
	task->alarm_time = abs_time_in_us;
	_OS_QueueInsert(((is_new_job) ? &g_ready_q : &g_wait_q), (void *) task, abs_time_in_us);
	
	if(update_timer) 
	{
		_OS_SetNextTimeout();
	}
}

///////////////////////////////////////////////////////////////////////////////
// Set the timer to next earliest timeout requested in either the
// ready queue or the wait queue
///////////////////////////////////////////////////////////////////////////////
void _OS_SetNextTimeout(void)
{
	UINT64 t1 = 0xFFFFFFFFFFFFFFFF;
	UINT64 t2 = 0xFFFFFFFFFFFFFFFF;
	UINT64 time_in_us;

	_OS_QueuePeek(&g_ready_q, 0, &t1);
	_OS_QueuePeek(&g_wait_q, 0, &t2);
	time_in_us = (t1 < t2) ? t1 : t2;
		
	// Check if we want a shorter timeout than the one currently set
	if((time_in_us > g_global_time) && \
		(time_in_us < g_next_wakeup_time))	
	{
		// Also check if the timeout is > maximum timeout the clock can support
		UINT32 diff_time = (UINT32)(time_in_us - g_global_time);
		if(diff_time > MAX_TIMER0_INTERVAL_uS)
		{
			diff_time = MAX_TIMER0_INTERVAL_uS;
		}
					
		// Schedule the next interrupt
		g_next_wakeup_time = g_global_time + diff_time;
		_OS_UpdateTimer(diff_time);
	}
}

///////////////////////////////////////////////////////////////////////////////
// Statistics task
// This runs every 100ms and recomputes the current CPU utilization
// 
///////////////////////////////////////////////////////////////////////////////
void _OS_StatisticsFn(void * ptr)
{
	static UINT64 prev_elapsed_time = 0;
	UINT64 new_elapsed_time;
	UINT32 diff_time;
	UINT32 intsts;
	FP64 usage;
	if(!g_idle_max_count) g_idle_max_count = g_idle_count * 10;	// Capture the initial idle count for STAT_TASK_PERIOD

	new_elapsed_time = OS_GetElapsedTime();
	OS_ENTER_CRITICAL(intsts);	// Enter critical section
	diff_time = (UINT32)(new_elapsed_time - prev_elapsed_time);
	usage = ((FP64)g_idle_count * STAT_TASK_PERIOD) / ((FP64)diff_time * g_idle_max_count);	// _OS_CPUUsage now indicates the free time
	_OS_CPUUsage = (usage > 1.0) ? 0 : 1.0 - usage;	// Now _OS_CPUUsage stands for 
	g_idle_count = 0;
	prev_elapsed_time = new_elapsed_time;
	OS_EXIT_CRITICAL(intsts);	// Exit critical section
}

///////////////////////////////////////////////////////////////////////////////
// The below function, gets the total elapsed time since the beginning
// of the system in microseconds.
///////////////////////////////////////////////////////////////////////////////
UINT64 OS_GetElapsedTime()
{
	UINT64 elapsed_time, old_global_time;

	// NOTE: The below loop for GetElapsedTime is very important. The design 
	// for this function is due to:
	// 1. We want to ensure that there is no interruption b/w reading
	// g_global_time and _OS_GetTimerValue_us. Otherwise we will have old g_global_time 
	// and latest timer count, which is not correct. 
	// 2. We cannot use disable/enable interrupts to avoid looping. This is because
	// the timer is designed to keep running in the background even if we have
	// disabled the interrupts. So if we disable the interrupts, it is possible
	// that the timer has reached the terminal value and then it reloads 0 again
	// and starts running. And since we have disabled the interrupts, we would get
	// older g_global_time and newer timer count, which is not correct.
	do
	{
		old_global_time = g_global_time;
		elapsed_time = g_global_time + _OS_GetTimerValue_us(1);
	} 
	while(old_global_time != g_global_time); // To ensure that the timer has not expired since we have read both g_global_time and OSW_GetTime

	return elapsed_time; 
}
