///////////////////////////////////////////////////////////////////////////////
//	
//						Copyright 2009-2013 xxxxxxx, xxxxxxx
//	File:	os_timer.c
//	Author: Bala B.
//	Description: OS Timer implementation
//	
///////////////////////////////////////////////////////////////////////////////

#include "os_timer.h"
#include "os_core.h"
#include "soc.h"

//The S3C2440A has five 16-bit timers. Timer 0, 1, 2, and 3 have Pulse Width Modulation (PWM) function. 
// Timer 4 has an internal timer only with no output pins. The timer 0 has a dead-zone generator, 
// which is used with a large current device.
// For the purpose of OS, lets use Timer 0 & 1

// TODO: I need to make below calculations extremely efficient.
// Use 64 bit calculation for accuracy and always round up
#define CONVERT_us_TO_TICKS(us)		((((UINT64)TIMER01_TICK_RATE * (us)) + (1000000-1)) / 1000000)	
#define CONVERT_TICKS_TO_us(tick)	((((tick) * 1000000ull) + (TIMER01_TICK_RATE - 1)) / TIMER01_TICK_RATE)

#define MAX_TIMER_COUNT		0xffff

#define TIMER0_START		0x001
#define TIMER0_UPDATE		0x002
#define TIMER0_AUTORELOAD	0x008

#define TIMER1_START		0x100
#define TIMER1_UPDATE		0x200
#define TIMER1_AUTORELOAD	0x800


static UINT32 timer0_count_buffer;
static UINT32 timer1_count_buffer;

UINT32 _OS_Timer0ISRHook(void *arg);
UINT32 _OS_Timer1ISRHook(void *arg);
UINT32 _OS_SetBudgetTimer(UINT32 delay_in_us);

///////////////////////////////////////////////////////////////////////////////
// Function to initialize the timers 0 &       1
///////////////////////////////////////////////////////////////////////////////
void _OS_InitTimer ()
{
	static BOOL initialized = 0;

	if(!initialized)
	{
		// Place the timers 0 & 1 on hold 
		rTCON &= ~0xfff;
		
		// Clear any pending interrupts
		rSRCPND = (3 << 10);
		rINTPND = (3 << 10);
		
		timer0_count_buffer = 0;
		timer0_count_buffer = 0;
		
		// Configure the two prescalars
		rTCFG0 = (rTCFG0 & 0xffffff00) | TIMER_PRESCALAR_0;	 // Prescaler=1/256 for Timer0 & Timer1
		rTCFG1 = (rTCFG1 & 0xffffff00) | (TIMER1_DIVIDER << 4) | TIMER0_DIVIDER;  // Mux=1/8 for Timer0 & Timer1
		
		// Set the interrupt handlers. This also unmasks that interrupt
		OS_SetInterruptVector(_OS_Timer0ISRHook, TIMER0_INT_VECTOR_INDEX);
		OS_SetInterruptVector(_OS_Timer1ISRHook, TIMER1_INT_VECTOR_INDEX);

		// Set the initialized flag
		initialized = 1;
	}
}

///////////////////////////////////////////////////////////////////////////////
// Timer 0 & 1 Interrupt handler
///////////////////////////////////////////////////////////////////////////////
void _OS_TimerInterrupt(UINT32 timer)
{
	// We should keep the running without it generating an interrupt. 
	// Otherwise there will be a drift in the clock. 
	// Hence comment following code
	//rTCON &= ~(0x0f << (timer << 3));
	
	// Clear the interrupt flag in the SRCPND and INTPND registers
	rSRCPND = (BIT_TIMER0 << timer);
	rINTPND = (BIT_TIMER0 << timer);
	
	// Since there has been an interrupt, the timer must have reloaded MAX_TIMER_COUNT
	// rTCNTB register. So reload the right value to use.
	if(timer)
		timer1_count_buffer = rTCNTB1;
	else
		timer0_count_buffer = rTCNTB0;
}

///////////////////////////////////////////////////////////////////////////////
// Function to update the period in the timer
///////////////////////////////////////////////////////////////////////////////
BOOL _OS_UpdateTimer(UINT32 delay_in_us)
{
	Klog32(KLOG_OS_TIMER_SET, "OS Timer Set (us) - ", delay_in_us);
	
	if(delay_in_us == 0)
	{
		// Disable the timer. 
		rTCON &= ~0x0f;
		rTCNTB0 = timer0_count_buffer = 0; 
	}
	else
	{
		UINT32 req_count = CONVERT_us_TO_TICKS(delay_in_us);
	
		// Get the timer count that has already elapsed since the actual timer interrupt
		UINT32 elapsed_count = timer0_count_buffer ? (timer0_count_buffer - rTCNTO0) : 0;
		
		if(req_count > elapsed_count) 
		{
			// The requested timeout is in the future. Update the terminal Count
			// and just resume counting
			rTCNTB0 = timer0_count_buffer = (req_count - elapsed_count);
			Klog32(KLOG_OS_TIMER_SET, "OS Timer Set - ", timer0_count_buffer);
		}
		else
		{
			// Highly undesirable situation. You may need to adjust the task timings
			Syslog64("KERNEL WARNING: Requested timeout is in the past ", req_count);
			rTCNTB0 = timer0_count_buffer = 0;	
		}
			
		// Inform that the Timer 0 Buffer has changed by updating manual update bit
		rTCON = (rTCON & (~0x0f)) | TIMER0_UPDATE;
		
		// Once manual update is done, we can change rTCNTB without affecting the current
		// timeout. We want the next cycle to start from the max value so that it is 
		// easy to compute elapsed time later on. This value will be loaded during auto-reload
		// after the counter reaches zero
		rTCNTB0 = MAX_TIMER_COUNT;
		
		// If the timer is not running, then start the timer with auto reload
		if(!(rTCON & TIMER0_START))
		{
			rTCON = (rTCON & (~0x0f)) | (TIMER0_START | TIMER0_AUTORELOAD);	
		}
	}
	
	return 1;
}

///////////////////////////////////////////////////////////////////////////////
// Sets a new timeout for the budget timer 
///////////////////////////////////////////////////////////////////////////////
UINT32 _OS_SetBudgetTimer(UINT32 delay_in_us)
{
	UINT32 req_count = CONVERT_us_TO_TICKS(delay_in_us);
	UINT32 cur_count = rTCNTO1;
	
	Klog32(KLOG_BUDGET_TIMER_SET, "Budget Timer Set (us) - ", delay_in_us);
	
	if(delay_in_us == 0)
	{
		// Disable the timer. 
		rTCON &= ~0xf00;
		rTCNTB1 = timer1_count_buffer = 0; 
	}
	else 
	{
		// Get the timer count that has already elapsed since the actual timer interrupt
		UINT32 elapsed_count = timer1_count_buffer ? (timer1_count_buffer - cur_count) : 0;

		if(req_count > elapsed_count) 
		{
			// The requested timeout is in the future. Update the terminal Count
			// and just resume counting
			rTCNTB1 = timer1_count_buffer = (req_count - elapsed_count);
		}
		else
		{
			rTCNTB1 = timer1_count_buffer = 0;	
		}

		Klog32(KLOG_BUDGET_TIMER_SET, "Budget Timer Set - ", timer1_count_buffer);
		
		// Once manual update is done, we can change rTCNTB without affecting the current
		// timeout. We want the next cycle to start from the max value so that it is 
		// easy to compute elapsed time later on. This value will be loaded during auto-reload
		// after the counter reaches zero
		rTCNTB1 = MAX_TIMER_COUNT;
		
		// If the timer is not running, then start the timer with auto reload
		if(!(rTCON & TIMER1_START))
		{
			rTCON = (rTCON & (~0xf00)) | (TIMER1_START | TIMER1_AUTORELOAD);
		}
	}

	return CONVERT_TICKS_TO_us(cur_count);
}

///////////////////////////////////////////////////////////////////////////////
// Converts the current timer count into micro seconds and returns.
///////////////////////////////////////////////////////////////////////////////
UINT32 _OS_GetTime(UINT32 timer)
{
	return CONVERT_TICKS_TO_us(timer ? rTCNTO1 : rTCNTO0);
}
