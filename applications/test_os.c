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

void task_period(void * ptr)
{
	static int count = 0;
	user_led_toggle(*(int *)ptr);
	Syslog32("task_fn1 - ", count++);
}

void task_short_period(void * ptr)
{
	user_led_toggle(*(int *)ptr);
}

void task_long_period(void * ptr)
{
	user_led_toggle(*(int *)ptr);
}

void task_long_budget(void * ptr)
{
	static int count = 0;
 	OS_PeriodicTask * task = OS_GetCurrentTask();
 	UINT32 dm = task->TBE_count;

	user_led_toggle(*(int *)ptr);
	Syslog32("task_long_budget - ", count++);

 	while(dm == task->TBE_count);
}

void task_TBE(void * ptr)
{
	static int count = 0;
 	OS_PeriodicTask * task = OS_GetCurrentTask();
 	UINT32 dm = task->TBE_count;

	user_led_toggle(*(int *)ptr);
//	Syslog32("task_fn1 - ", count++);

 	while(dm == task->TBE_count);
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

UINT32 stack1 [0x400];
UINT32 stack2 [0x400];
UINT32 stack3 [0x400];
UINT32 stack4 [0x400];

int a = 0;
int b = 1;
int c = 2;
int d = 3;


int test_short_intervals(int argc, char *argv[])
{
	OS_Init();
	
	SyslogStr("Calling - ",  __func__);

	OS_CreatePeriodicTask( 1000, 1000, 500, 100, stack1, sizeof(stack1), "LED1", &task1, task_short_period, &a);
 	OS_CreatePeriodicTask( 1200, 1200, 200, 450, stack2, sizeof(stack2), "LED2", &task2, task_short_period, &b);
 	OS_CreatePeriodicTask( 5000, 5000, 300, 210, stack3, sizeof(stack3), "LED3", &task3, task_short_period, &c);
 	OS_CreatePeriodicTask(20000, 20000, 900, 370, stack4, sizeof(stack4), "LED4", &task4, task_short_period, &d);
	
	OS_Start();

	return 0;
}

int test_long_intervals(int argc, char *argv[])
{
	OS_Init();
	
	SyslogStr("Calling - ",  __func__);

	OS_CreatePeriodicTask( 1000000, 1000000, 50000, 0, stack1, sizeof(stack1), "LED1", &task1, task_long_period, &a);
 	OS_CreatePeriodicTask( 1200000, 1200000, 20000, 0, stack2, sizeof(stack2), "LED2", &task2, task_long_period, &b);
 	OS_CreatePeriodicTask( 5000000, 5000000, 30000, 0, stack3, sizeof(stack3), "LED3", &task3, task_long_period, &c);
 	OS_CreatePeriodicTask( 2000000, 2000000, 90000, 0, stack4, sizeof(stack4), "LED4", &task4, task_long_period, &d);
	
	OS_Start();

	return 0;
}

int test_casual(int argc, char *argv[])
{
	OS_Init();
	
	SyslogStr("Calling - ",  __func__);

	OS_CreatePeriodicTask( 100000, 100000, 30000, 5000, stack1, sizeof(stack1), "LED1", &task1, task_long_budget, &a);
  	OS_CreatePeriodicTask( 120000, 120000, 20000, 10000, stack2, sizeof(stack2), "LED2", &task2, task_long_budget, &b);
  	OS_CreatePeriodicTask( 500000, 500000, 30000, 15000, stack3, sizeof(stack3), "LED3", &task3, task_long_budget, &c);
  	OS_CreatePeriodicTask( 200000, 200000, 40000, 20000, stack4, sizeof(stack4), "LED4", &task4, task_long_budget, &d);
	
	OS_Start();

	return 0;
}

int test_long_budget(int argc, char *argv[])
{
	OS_Init();
	
	SyslogStr("Calling - ",  __func__);

	OS_CreatePeriodicTask( 1000000, 1000000, 300000, 0, stack1, sizeof(stack1), "LED1", &task1, task_long_budget, &a);
  	OS_CreatePeriodicTask( 1200000, 1200000, 200000, 0, stack2, sizeof(stack2), "LED2", &task2, task_long_budget, &b);
  	OS_CreatePeriodicTask( 5000000, 5000000, 300000, 0, stack3, sizeof(stack3), "LED3", &task3, task_long_budget, &c);
  	OS_CreatePeriodicTask( 2000000, 2000000, 400000, 0, stack4, sizeof(stack4), "LED4", &task4, task_long_budget, &d);
	
	OS_Start();

	return 0;
}

int test_TBE(int argc, char *argv[])
{
	OS_Init();
	
	SyslogStr("Calling - ",  __func__);

	OS_CreatePeriodicTask( 1000, 1000, 500, 100, stack1, sizeof(stack1), "LED1", &task1, task_TBE, &a);
 	OS_CreatePeriodicTask( 1200, 1200, 200, 450, stack2, sizeof(stack2), "LED2", &task2, task_TBE, &b);
 	OS_CreatePeriodicTask( 5000, 5000, 300, 300, stack3, sizeof(stack3), "LED3", &task3, task_TBE, &c);
 	OS_CreatePeriodicTask(400000, 400000, 40000, 10000, stack4, sizeof(stack4), "LED4", &task4, task_TBE, &d);
	
	OS_Start();

	return 0;
}


int main(int argc, char *argv[])
{
//	test_casual(argc, argv);
//	test_short_intervals(argc, argv);
//	test_long_intervals(argc, argv);
	test_long_budget(argc, argv);
//	test_TBE(argc, argv);
	return 0;
}


