///////////////////////////////////////////////////////////////////////////////
//	
//						Copyright 2009-2013 xxxxxxx, xxxxxxx
//	File:	os_task.h
//	Author:	Bala B. (bhat.balasubramanya@gmail.com)
//	Description: OS Task Related routines
//	
///////////////////////////////////////////////////////////////////////////////

#ifndef _OS_TASK_H
#define _OS_TASK_H

#include "os_types.h"

///////////////////////////////////////////////////////////////////////////////
// Constants
///////////////////////////////////////////////////////////////////////////////
#define ONE_KB	1024
#define ONE_MB	ONE_KB * ONE_KB

#if OS_WITH_VALIDATE_TASK==1
#define TASK_SIGNATURE	0xFEEDBACC
#endif

///////////////////////////////////////////////////////////////////////////////
// Type of Task
///////////////////////////////////////////////////////////////////////////////

enum {
	PERIODIC_TASK = 0,
	APERIODIC_TASK = 1,
	SYSTEM_TASK = 0,
	USER_TASK = 2
};

///////////////////////////////////////////////////////////////////////////////
// Task TCB 
///////////////////////////////////////////////////////////////////////////////

typedef struct OS_PeriodicTask OS_PeriodicTask;
struct OS_PeriodicTask
{
	// Attributes for maintaining a list/queue of threads.
	// IMPORTANT: Make sure that the below two attributes are the first two
	// attributes in the same order. This is done so that we can have a single
	// queue implementation for all types of objects as long as the below two
	// members are part of those objects.
	OS_PeriodicTask * next;	// Do NOT REORDER THESE TWO MEMBERS
	UINT64 alarm_time;	// To be used for maintaining a queue ordered based on this key value

	UINT32 *top_of_stack;	// Do NOT REORDER THIS MEMBER, THE OFFSET 'SP_OFFSET_IN_TCB' IS USED IN ASSEMBLY

	// Folliwing attributes are common in both type of tasks. They should be in the same order.
#if OS_WITH_TASK_NAME==1
	INT8 name[OS_TASK_NAME_SIZE];
#endif
	UINT8 type;	
	UINT8 mode;
	UINT16 id;
#if OS_WITH_VALIDATE_TASK==1
	UINT32 signature;		// A unique identifier to validate the task structure
#endif	// OS_WITH_VALIDATE_TASK

	// Following arguments are specific to Periodic task
	UINT32 period;
	UINT32 deadline;
	UINT32 budget;
	UINT32 phase;
	
	UINT32 *stack;
	UINT32 stack_size;
	void (*task_function)(void *pdata);
	void *pdata;
	
	// Following members are used by the scheduling algorithm
	UINT32 remaining_budget;
	UINT64 accumulated_budget;
	UINT64 stored_release_time;
	UINT32 exec_count;
	UINT32 TBE_count;
	UINT32 dline_miss_count;
	// UINT64 next_release_time;
};

typedef struct OS_AperiodicTask OS_AperiodicTask;
struct OS_AperiodicTask
{
	// Attributes for maintaining a list/queue of threads.
	// IMPORTANT: Make sure that the below two attributes are the first two
	// attributes in the same order. This is done so that we can have a single
	// queue implementation for all types of objects as long as the below two
	// members are part of those objects.
	OS_AperiodicTask * next;	// Do NOT REORDER THESE TWO MEMBERS
	UINT64 priority;	// To be used for maintaining a queue ordered based on this key value

	// Folliwing attributes are common in both type of tasks. They should be in the same order
	UINT32 *top_of_stack;	// Do NOT REORDER THIS MEMBER, THE OFFSET 'SP_OFFSET_IN_TCB' IS USED IN Assembly

#if OS_WITH_TASK_NAME==1
	INT8 name[OS_TASK_NAME_SIZE];
#endif
	UINT8 type;	
	UINT8 mode;
	UINT16 id;
#if OS_WITH_VALIDATE_TASK==1
	UINT32 signature;		// A unique identifier to validate the task structure
#endif	// OS_WITH_VALIDATE_TASK

	// Following arguments are specific to Aperiodic task
	UINT32 *stack;
	UINT32 stack_size;
	void (*task_function)(void *pdata);
	void *pdata;
};

#endif // _OS_TASK_H
