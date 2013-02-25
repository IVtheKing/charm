///////////////////////////////////////////////////////////////////////////////
//
//						Copyright 2013 xxxxxxx, xxxxxxx
//	File:	os_sem.c
//	Author: Bala B.
//	Description: OS Statistics related functions
//
///////////////////////////////////////////////////////////////////////////////

#include "os_stat.h"

#if OS_ENABLE_CPU_STATS==1

OS_PeriodicTask g_stat_task;		// A TCB for the idle task
UINT32 g_stat_task_stack [OS_STAT_TASK_STACK_SIZE];

// Some statistics counters to keep track.
volatile UINT32 max_scheduler_elapsed_time;
volatile UINT32 scheduler_miss_counter;

// Variables to keep track of the idle task execution
volatile UINT32 g_idle_max_count;
volatile UINT32 g_idle_count;
volatile FP32 _OS_CPUUsage;

///////////////////////////////////////////////////////////////////////////////
// Statistics variable initialization
///////////////////////////////////////////////////////////////////////////////
void _OS_StatInit(void)
{
	max_scheduler_elapsed_time = 0;
	scheduler_miss_counter = 0;
	g_idle_max_count = 0;
	g_idle_count = 0;
}

///////////////////////////////////////////////////////////////////////////////
// Statistics task
// This runs every 100ms and recomputes the current CPU utilization
// 
///////////////////////////////////////////////////////////////////////////////
void _OS_StatisticsFn(void * ptr)
{
	
	Syslog32("STAT: max_scheduler_elapsed_time = ", max_scheduler_elapsed_time);
	Syslog32("STAT: scheduler_miss_counter = ", scheduler_miss_counter);
	
	// TODO: This logic is now outdated as the OS uses wait_for_interrupt in idle task
// 	static UINT64 prev_elapsed_time = 0;
// 	UINT64 new_elapsed_time;
// 	UINT32 diff_time;
// 	UINT32 intsts;
// 	FP32 usage;
// 	
// 	// Capture the initial idle count for STAT_TASK_PERIOD
// 	if(!g_idle_max_count) g_idle_max_count = g_idle_count * 10;	
// 
// 	new_elapsed_time = OS_GetElapsedTime();
// 	OS_ENTER_CRITICAL(intsts);	// Enter critical section
// 	diff_time = (UINT32)(new_elapsed_time - prev_elapsed_time);
// 	
// 	// _OS_CPUUsage now indicates the free time
// 	usage = ((FP32)g_idle_count * STAT_TASK_PERIOD) / ((FP32)diff_time * g_idle_max_count);	
// 	
// 	// Now _OS_CPUUsage stands for 
// 	_OS_CPUUsage = (usage > 1.0) ? 0 : 1.0 - usage;	
// 	g_idle_count = 0;
// 	prev_elapsed_time = new_elapsed_time;
// 	OS_EXIT_CRITICAL(intsts);	// Exit critical section
}

#endif // OS_ENABLE_CPU_STATS
