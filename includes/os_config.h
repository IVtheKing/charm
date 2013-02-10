///////////////////////////////////////////////////////////////////////////////
//	
//						Copyright 2009-2013 xxxxxxx, xxxxxxx
//	File:	os_config.h
//	Author:	Bala B. (bhat.balasubramanya@gmail.com)
//	Description: OS Configuration options
//	
///////////////////////////////////////////////////////////////////////////////

#ifndef _OS_CONFIG_H
#define _OS_CONFIG_H


#define FIN 	(12000000)			// 12MHz Crystal

#define FCLK (405000000)			// Main Processor clock	405 MHz
#define HCLK (FCLK/3)				// AHB Clock	135 MHz
#define PCLK (HCLK/2)				// APB Clock	67.5 MHz
#define UCLK (48000000)				// USB Clock

#define	TIMER_PRESCALAR_0	(0xff)	// PCLK/256
#define	TIMER0_DIVIDER		(0x02)	// PCLK/PRESCALAR0/8
#define	TIMER1_DIVIDER		(0x02)	// PCLK/PRESCALAR0/8
#define	TIMER01_TICK_RATE	(PCLK/(TIMER_PRESCALAR_0+1)/8)		// Resolution 30.340 uSec per tick
#define	MAX_TIMER_INTERVAL_uS	1988439		// TIMER01_TICK_RATE/65535 = 1.988439832514109 seconds

// Instruction and Data Cache related
#define ENABLE_INSTRUCTION_CACHE	1
#define ENABLE_DATA_CACHE			1

// Task related
#define MIN_PRIORITY				255
#define OS_IDLE_TASK_STACK_SIZE		0x40		// In Words
#define OS_STAT_TASK_STACK_SIZE		0x40		// In Words
#define OS_WITH_TASK_NAME			1
#define OS_TASK_NAME_SIZE			8
#define OS_FIRST_SCHED_DELAY		10000	// Time for the first scheduling interrupt

// Following values depend a lot on the timer resolution which is not very good in this case
#define TASK_MIN_PERIOD		100	// 100 uSec
#define TASK_MIN_BUDGET		100 // 100 uSec
#define TIME_RESOLUTION_IN_US	250	// 250uSec

// TODO: I should handle the case where these are bigger values. Should be possible by taking care while setting 
// timers
#define TASK_MAX_PERIOD		MAX_TIMER_INTERVAL_uS
#define TASK_MAX_BUDGET		MAX_TIMER_INTERVAL_uS

// Debug & Info related
#define OS_ENABLE_CPU_STATS			0		// TODO: Enable CPU Stats
#define OS_WITH_VALIDATE_TASK		1
#define	OS_KERNEL_LOGGING			0
#define	OS_KLOG_MASK				0xFFFFFFFF
#define DEBUG_UART_CHANNEL			0

#endif // _OS_CONFIG_H
