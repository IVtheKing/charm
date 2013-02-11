///////////////////////////////////////////////////////////////////////////////
//	
//						Copyright 2009-2013 xxxxxxx, xxxxxxx
//	File:	os_test.c
//	Author:	Bala bhat (bhat.balasubramanya@gmail.com)
//	Description: OS Test file
//	
///////////////////////////////////////////////////////////////////////////////

#include "os_core.h"
#include "target.h"

OS_PeriodicTask task1, task2, task3, task4;

extern volatile void * g_current_task;
extern void pi_main(void* data);
extern void srt_main(void* array);
extern int g_array[];

void task_fn1(void * ptr)
{
	static int count = 0;
// 	OS_PeriodicTask * task = OS_GetCurrentTask();
// 	UINT32 dm = task->TBE_count;

	user_led_toggle(*(int *)ptr);
	Syslog32("task_fn1 - ", count++);

// 	while(dm == task->TBE_count)
// 	{
// 	}
}

void task_fn(void * ptr)
{
	OS_PeriodicTask * task = (OS_PeriodicTask *) g_current_task;
	UINT32 dm;

	while(1)
	{
		dm = task->TBE_count;
		user_led_toggle(*(int *)ptr);

		while(dm == task->TBE_count)
		{
		}
	}
}

void task_fn2(void * ptr)
{
	volatile int x = (*(int *)ptr + 1) * 2000;
	while(x)
	{
		x--;
	}
}

UINT32 stack1 [0x400];
UINT32 stack2 [0x400];
UINT32 stack3 [0x400];
UINT32 stack4 [0x400];

int a = 0;
int b = 1;
int c = 2;
int d = 3;

int main(int argc, char *argv[])
{
	OS_Init();
	
	SyslogStr("Calling - ",  __func__);

//	OS_CreatePeriodicTask(1000, 1000, 500, 0, stack1, sizeof(stack1), &task1, task_fn2, &a);
//	OS_CreatePeriodicTask(2000, 2000, 600, 200, stack2, sizeof(stack2), &task2, task_fn2, &b);
//	OS_CreatePeriodicTask(5000, 5000, 1000, 500, stack3, sizeof(stack3), &task3, task_fn2, &c);
//	OS_CreatePeriodicTask(10000, 10000, 2500, 1000, stack4, sizeof(stack4), &task4, task_fn2, &d);

//	OS_CreatePeriodicTask( 100000, 100000, 50000, 10000, stack1, sizeof(stack1), "LED1", &task1, task_fn1, &a);
 	OS_CreatePeriodicTask( 200000, 200000, 20000, 45000, stack2, sizeof(stack2), "LED2", &task2, task_fn1, &b);
// 	OS_CreatePeriodicTask( 500000, 500000, 30000, 21000, stack3, sizeof(stack3), "LED3", &task3, task_fn1, &c);
// 	OS_CreatePeriodicTask(1000000, 1000000, 40000, 37000, stack4, sizeof(stack4), "LED4", &task4, task_fn1, &d);
	
//	OS_CreatePeriodicTask(10000, 8000, 2000, 0, stack1, sizeof(stack1), &task1, pi_main, (void*)&a);
//	OS_CreatePeriodicTask(10000, 8000, 2000, 0, stack2, sizeof(stack2), &task2, srt_main, (void*)g_array);

	OS_Start();

	return 0;
}
