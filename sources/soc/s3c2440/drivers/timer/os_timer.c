///////////////////////////////////////////////////////////////////////////////
//	
//						Copyright 2009-2013 xxxxxxx, xxxxxxx
//	File:	os_timer.c
//	Author: Bala B.
//	Description: OS Timer implementation
//	
///////////////////////////////////////////////////////////////////////////////

#include "os_timer.h"
#include "soc.h"

//The S3C2440A has five 16-bit timers. Timer 0, 1, 2, and 3 have Pulse Width Modulation (PWM) function. 
// Timer 4 has an internal timer only with no output pins. The timer 0 has a dead-zone generator, 
// which is used with a large current device.
// For the purpose of OS, lets use Timer 0 & 1

// TODO: I need to make below calculations extremely efficient.
// Use 64 bit calculation for accuracy and always round up
#define CONVERT_us_TO_TICKS(us)		((((UINT64)TIMER01_TICK_RATE * (us)) + (1000000-1)) / 1000000)	
#define CONVERT_TICKS_TO_us(tick)	((((tick) * 1000000ull) + (TIMER01_TICK_RATE - 1)) / TIMER01_TICK_RATE)

#define MAX_TIMER_COUNT		0

UINT32 OS_Timer0ISRHook(void *arg);
UINT32 OS_Timer1ISRHook(void *arg);
UINT32 OS_SetBudgetTimer(UINT32 delay_in_us);

///////////////////////////////////////////////////////////////////////////////
// Function to initialize the timers 0 &       1
///////////////////////////////////////////////////////////////////////////////
void OS_InitTimer ()
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
		OS_SetInterruptVector(OS_Timer0ISRHook, TIMER0_INT_VECTOR_INDEX);
		OS_SetInterruptVector(OS_Timer1ISRHook, TIMER1_INT_VECTOR_INDEX);

		// Set the initialized flag
		initialized = 1;
	}
}

///////////////////////////////////////////////////////////////////////////////
// Timer 0 & 1 Interrupt handler
///////////////////////////////////////////////////////////////////////////////
void Handle_TimerInterrupt(UINT32 timer)
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
BOOL OS_UpdateTimer(UINT32 delay_in_us)
{
	UINT32 req_count = CONVERT_us_TO_TICKS(delay_in_us);
	UINT32 cur_count;
	
	// Get the Terminal Value and the Current Counter
	cur_count = rTCNTO0;	 
	
	if(delay_in_us == 0)
	{
		rTCNTB0 = 0; // The Timer is already in Hold State. Just reset the count to zero
		rTCON = (rTCON & (~0x0f)) | 0x02;
	}
	else if(req_count > cur_count)
	{
		// The requested timeout is in the future. Update the terminal Count
		// and just resume counting
		rTCNTB0 = req_count;
		
		// Configure these as one shot timer and auto reload
		rTCON = (rTCON & (~0x0f)) | 0x02;
		rTCON = (rTCON & (~0x0f)) | 0x09;	// Timer 0 Start | auto reload
	}
	else
	{
		return 0; // The requested timeout has already passed or too close.
	}

	return 1;
}

///////////////////////////////////////////////////////////////////////////////
// Sets a new timeout for the budget timer 
///////////////////////////////////////////////////////////////////////////////
UINT32 OS_SetBudgetTimer(UINT32 delay_in_us)
{
	UINT32 req_count = CONVERT_us_TO_TICKS(delay_in_us);
	UINT32 cur_count;
	
	// Get the Terminal Value and the Current Counter
	cur_count = CONVERT_TICKS_TO_us(rTCNTO1);	 
	
	if(delay_in_us == 0)
	{
		rTCNTB1 = 0; // The Timer is already in Hold State. Just reset the count to zero
		rTCON = (rTCON & (~0xf00)) | 0x200;
	}
	else 
	{
		// The requested timeout is in the future. Update the terminal Count
		// and just resume counting
		rTCNTB1 = req_count;
		rTCON = (rTCON & (~0xf00)) | 0x200;
		rTCON = (rTCON & (~0xf00)) | 0x900;		// Timer 1 Start | auto reload
	}

	return cur_count;
}

///////////////////////////////////////////////////////////////////////////////
// Converts the current timer count into micro seconds and returns.
///////////////////////////////////////////////////////////////////////////////
UINT32 OS_GetTime()
{
	return CONVERT_TICKS_TO_us(rTCNTO0);
}

///////////////////////////////////////////////////////////////////////////////
// Get the timer value in microseconds 
///////////////////////////////////////////////////////////////////////////////
UINT32 OSGetTime()
{
	return CONVERT_TICKS_TO_us(rTCNTO1);
}
