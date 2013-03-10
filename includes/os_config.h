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

#define FIN 	(12000000)				// 12MHz Crystal

#define FCLK (405000000)				// Main Processor clock	405 MHz
#define HCLK (FCLK/3)					// AHB Clock	135 MHz
#define PCLK (HCLK/2)					// APB Clock	67.5 MHz
#define UCLK (48000000)					// USB Clock

#define	TIMER_PRESCALAR_0	(0x3f)		// PCLK/64
#define	TIMER0_DIVIDER		(0x00)		// PCLK/PRESCALAR0/2
#define	TIMER1_DIVIDER		(0x03)		// PCLK/PRESCALAR0/16
#define	TIMER0_TICK_FREQ	(527344)	// (PCLK/(TIMER_PRESCALAR_0+1)/2) - Resolution 1.8963 uSec per tick
#define	TIMER1_TICK_FREQ	(65918)		// (PCLK/(TIMER_PRESCALAR_0+1)/16) - Resolution 15.1705 uSec per tick

// (65535 * 1000000) / TIMER0_TICK_FREQ = 124273.78 uSec. Lets use 100ms for this.
// This way there will be at least one interrupt every 100ms
#define	MAX_TIMER0_INTERVAL_uS		100000		

// (65535 * 1000000) / TIMER0_TICK_FREQ = 124273.78 uSec. Lets use 120ms for this.
// Note that this value should be > MAX_TIMER0_INTERVAL_uS in order to prevent race between
// timer0 & timer1 interrupts. 
#define	MAX_TIMER1_INTERVAL_uS		120000		

// Instruction and Data Cache related
#define ENABLE_INSTRUCTION_CACHE	1
#define ENABLE_DATA_CACHE			1

// Process related
#define OS_PROCESS_NAME_SIZE		16

// MMU Related
#define ENABLE_MMU					1

// Drivers to include
#define ENABLE_RTC					1
#define ENABLE_RTC_ALARM			1

// Since the OS timer is not a fixed interval timer, there can be a very small drift
// over a long period of time. The RTOS actually calculates the time spent in the 
// timer ISR and deducts it from the next task budget. However, there can be a drift 
// worth few of instructions b/w the time we read the timer and adjust and write it back.
// Using another fixed interval timer, we can resynchronize periodically.
#define ENABLE_SYNC_TIMER			1
#define SYNC_TIMER_INTERVAL			800000		// in Microseconds. 
// NOTE: Above interval should be <= TIMER1 Max Interval

// Task related configuration parameters
#define MIN_PRIORITY				255
#define OS_IDLE_TASK_STACK_SIZE		0x40		// In Words
#define OS_STAT_TASK_STACK_SIZE		0x40		// In Words
#define STAT_TASK_PERIOD			5000000	// 5 sec
#define OS_WITH_TASK_NAME			1
#define OS_TASK_NAME_SIZE			16

// Time for the first scheduling interrupt to begin. This should be <= MAX_TIMER0_INTERVAL_uS
// Just few micro seconds are enough. This is to adjust the time gap b/w the timer setup
// and the OS to be ready to handle interrupts. If this delay is not enough, it will eat from
// the budget of the first job. Few microseconds are enough actually.
#define OS_FIRST_SCHED_DELAY		10000		// 10 ms

// Following values depend a lot on the timer resolution which is not very good in this case
#define TASK_MIN_PERIOD		100	// 100 uSec
#define TASK_MIN_BUDGET		100 // 100 uSec

// TODO: I should handle the case where these are bigger values. Should be possible by taking care while setting 
// timers
#define TASK_MAX_BUDGET		MAX_TIMER1_INTERVAL_uS

// Debug & Info related

// Define the Debug masks
typedef enum
{
	KLOG_CONTEXT_SWITCH 	= (1 << 0),
	KLOG_OS_TIMER_ISR 		= (1 << 1),
	KLOG_TBE_EXCEPTION		= (1 << 2),
	KLOG_OS_TIMER_SET 		= (1 << 3),
	KLOG_SYNC_TIMER_ISR 	= (1 << 4),
	
	KLOG_MISC 				= (1 << 31)
	
} Klog_MaskType;

#define OS_ENABLE_ASSERTS			1
#define OS_ENABLE_CPU_STATS			1		// Enable OS & CPU Stats
#define OS_WITH_VALIDATE_TASK		1

#define	OS_KERNEL_LOGGING			0
#define	OS_KLOG_MASK				(KLOG_SYNC_TIMER_ISR)
#define DEBUG_UART_CHANNEL			0

#endif // _OS_CONFIG_H
