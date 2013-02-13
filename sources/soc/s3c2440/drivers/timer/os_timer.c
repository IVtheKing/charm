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
	// TODO: This is bit early to stop the timer, somehow I should keep it running
	// without it generating an interrupt. Otherwise there will be a drift in the clock.
	// First Stop the Timer	
	rTCON &= ~(0x0f << (timer << 3));		
	
	// Clear the interrupt flag in the SRCPND and INTPND registers
	rSRCPND = (BIT_TIMER0 << timer);
	rINTPND = (BIT_TIMER0 << timer);
}

///////////////////////////////////////////////////////////////////////////////
// Function to update the period in the timer
///////////////////////////////////////////////////////////////////////////////
BOOL _OS_UpdateTimer(UINT32 delay_in_us)
{
	Klog32(KLOG_OS_TIMER_SET, "OS Timer Set (ms) - ", delay_in_us);
	
	if(delay_in_us == 0)
	{
		// Disable the timer
		rTCNTB0 = 0; // The Timer is already in Hold State. Just reset the count to zero
		rTCON = (rTCON & (~0x0f)) | 0x02;
	}
	else
	{
		UINT32 req_count = CONVERT_us_TO_TICKS(delay_in_us);
	
		// Get the timer count that has already elapsed since the actual timer interrupt
		UINT32 elapsed_count = (rTCNTB0 - rTCNTO0);
		
		if(req_count > elapsed_count) 
		{
			// The requested timeout is in the future. Update the terminal Count
			// and just resume counting
			rTCNTB0 =  (req_count - elapsed_count);
			Klog32(KLOG_OS_TIMER_SET, "OS Timer Set - ", rTCNTB0);
		}
		else
		{
			// Highly undesirable situation. You may need to adjust the task timings
			Syslog64("KERNEL WARNING: Requested timeout is in the past ", req_count);
			
			// Request a very small timeout so that we will be interrupted immediately.
			rTCNTB0 = 1;	
		}
			
		// Inform that the Timer 0 Buffer has changed
		rTCON = (rTCON & (~0x0f)) | 0x02;
		
		// Timer 0 Start | auto reload
		rTCON = (rTCON & (~0x0f)) | 0x09;	
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
	
	Klog32(KLOG_BUDGET_TIMER_SET, "Budget Timer Set (ms) - ", delay_in_us);
	
	if(delay_in_us == 0)
	{
		// Disable the timer
		rTCNTB1 = 0; // The Timer is already in Hold State. Just reset the count to zero
		rTCON = (rTCON & (~0xf00)) | 0x200;
	}
	else 
	{
		// Get the timer count that has already elapsed since the actual timer interrupt
		UINT32 elapsed_count = (rTCNTB1 - cur_count);

		// The requested timeout is in the future. Update the terminal Count
		// and just resume counting
		rTCNTB1 = (req_count > elapsed_count) ? (req_count - elapsed_count) : 1;
		Klog32(KLOG_BUDGET_TIMER_SET, "Budget Timer Set - ", rTCNTB1);
		
		// Inform that the Timer 1 Buffer has changed
		rTCON = (rTCON & (~0xf00)) | 0x200;
		
		// Timer 1 Start | auto reload
		rTCON = (rTCON & (~0xf00)) | 0x900;		
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
