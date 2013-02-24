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
#include "os_stat.h"
#include "soc.h"

//The S3C2440A has five 16-bit timers. Timer 0, 1, 2, and 3 have Pulse Width Modulation (PWM) function. 
// Timer 4 has an internal timer only with no output pins. The timer 0 has a dead-zone generator, 
// which is used with a large current device.
// For the purpose of OS, lets use Timer 0 & 1

// TODO: I need to make below calculations extremely efficient.
// Use 64 bit calculation for accuracy and always round up
#define CONVERT_us_TO_TICKS(us)		((((UINT64)TIMER01_TICK_RATE * (us)) + (1000000-1)) / 1000000)	
#define CONVERT_TICKS_TO_us(tick)	((((tick) * 1000000ull) + (TIMER01_TICK_RATE - 1)) / TIMER01_TICK_RATE)

// For testing
// #define CONVERT_us_TO_TICKS(us)		((us) >> 1)	
// #define CONVERT_TICKS_TO_us(tick)	((tick) << 1)

#define MAX_TIMER_COUNT		0xffff

#define TIMER0_START		0x001
#define TIMER0_UPDATE		0x002
#define TIMER0_AUTORELOAD	0x008

#define TIMER1_START		0x100
#define TIMER1_UPDATE		0x200
#define TIMER1_AUTORELOAD	0x800

static UINT32 timer0_count_buffer;

UINT32 _OS_Timer0ISRHook(void *arg);

///////////////////////////////////////////////////////////////////////////////
// Function to initialize the timers 0 & 1
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
		
#if OS_ENABLE_CPU_STATS==1
		scheduler_miss_counter = 0;
		max_scheduler_elapsed_time = 0;
#endif
		
		// Configure the two prescalars
		rTCFG0 = (rTCFG0 & 0xffffff00) | TIMER_PRESCALAR_0;	 // Prescaler=1/256 for Timer0 & Timer1
		rTCFG1 = (rTCFG1 & 0xffffff00) | (TIMER1_DIVIDER << 4) | TIMER0_DIVIDER;  // Mux=1/8 for Timer0 & Timer1
		
		// Set the interrupt handlers. This also unmasks that interrupt
		OS_SetInterruptVector(_OS_Timer0ISRHook, TIMER0_INT_VECTOR_INDEX);

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
	if(timer == 0) 
	{
		timer0_count_buffer = rTCNTB0;
	}
}

///////////////////////////////////////////////////////////////////////////////
// Function to update the period in the timer
// The following function sets up the OS timer.
// Input:
// 		delay_in_us: Is the delay requested
// Output:
//		delay_in_us: Is the actual delay setup. Since every timer has a maximum interval,
//		the delay setup may be less than the delay requested.
// Return Value:
//		The budget spent if applicable
///////////////////////////////////////////////////////////////////////////////
UINT32 _OS_UpdateTimer(UINT32 * delay_in_us)
{
	UINT32 elapsed_count;
	UINT32 budget_spent_us;
	
	ASSERT(delay_in_us);
	
	// Each timer has a maximum interval. Adjust the budget timeout accordingly
	if(*delay_in_us > MAX_TIMER0_INTERVAL_uS)
	{
		*delay_in_us = MAX_TIMER0_INTERVAL_uS;
	}

	UINT32 req_count = CONVERT_us_TO_TICKS(*delay_in_us);

	Klog32(KLOG_OS_TIMER_SET, "OS Timer Set (us) - ", *delay_in_us);
	
	// Clear the interrupt flag in the SRCPND and INTPND registers
	rSRCPND = BIT_TIMER0;
	rINTPND = BIT_TIMER0;
	
	// Calculate the elapsed time. There are following 4 cases:
	// 1. The OS timer has expired and the ISR has finished.
	//		In this case, timer0_count_buffer = MAX_TIMER_COUNT
	// 2. The OS timer has expired and but the ISR has not finished yet
	//		In this case, timer0_count_buffer < MAX_TIMER_COUNT but rTCNTO0 is > timer0_count_buffer
	// 3. Budget timer is still running	(not expired)
	// 4. The budget timer may have been disabled in the last period. 
	if(timer0_count_buffer == MAX_TIMER_COUNT)
	{
		// Case 1
		elapsed_count = (MAX_TIMER_COUNT - rTCNTO0);	// Take this time from the new task
		budget_spent_us = 0;							// Extra time is accounted for in the new task
	}
	else if(timer0_count_buffer > 0)
	{
		UINT32 tcount = rTCNTO0;
		if(tcount > timer0_count_buffer)
		{
			// Case 2
			elapsed_count = (MAX_TIMER_COUNT - tcount);		// Take this time from the new task
			budget_spent_us = 0;							// Extra time is accounted for in the new task
		}
		else
		{
			// Case 3. We must have requested shorter timeout.
			elapsed_count = 0;								// Extra time is accounted for in the old task
			tcount = (timer0_count_buffer - tcount);		// The time spent till now should be accounted for in the old task
			budget_spent_us = CONVERT_TICKS_TO_us(tcount);	// Convert it to us right away.
		}
	}
	else
	{
		// Case 4. The previous task must have been an Aperiodic task
		elapsed_count = 0;
		budget_spent_us = 0;
	}

	if(*delay_in_us == 0)
	{
		// Disable the timer. 
		rTCON &= ~0x0f;
		rTCNTB0 = timer0_count_buffer = 0; 
	}
	else
	{
		if(req_count > elapsed_count) 
		{
			// The requested timeout is in the future. Update the terminal Count
			// and just resume counting
			rTCNTB0 = timer0_count_buffer = (req_count - elapsed_count);
			
			Klog32(KLOG_OS_TIMER_SET, "OS Timer Set - ", timer0_count_buffer);
		}
		else
		{
			// Highly undesirable situation. May need to adjust the task timings
			// In order to recover from this bad situation, give some non-zero time for the timer
			rTCNTB0 = timer0_count_buffer = 10;
			
#if OS_ENABLE_CPU_STATS==1			
			scheduler_miss_counter ++;
#endif
			Syslog32("KERNEL WARNING: Requested timeout is in the past ", elapsed_count);
		}
		
		// Inform that Timer 0 Buffer has changed by updating manual update bit
		rTCON = (rTCON & (~0x0f)) | TIMER0_UPDATE;
		rTCON = (rTCON & (~0x0f)) | (TIMER0_START | TIMER0_AUTORELOAD); 
		
		// Once manual update is done, we can change rTCNTB without affecting the current
		// timeout. We want the next cycle to start from the max value so that it is 
		// easy to compute elapsed time later on. This value will be loaded during auto-reload
		// after the counter reaches zero
		rTCNTB0 = MAX_TIMER_COUNT;		
	}
	
	// Keep track of max_scheduler_elapsed_time
#if OS_ENABLE_CPU_STATS==1	
	if((elapsed_count > 0) && (max_scheduler_elapsed_time < elapsed_count))
	{
		max_scheduler_elapsed_time = elapsed_count;
	}
#endif	
	
	return budget_spent_us;
}

///////////////////////////////////////////////////////////////////////////////
// Converts the current timer count into micro seconds and returns.
///////////////////////////////////////////////////////////////////////////////
UINT32 _OS_GetTimerValue_us(UINT32 timer)
{
	return CONVERT_TICKS_TO_us(timer ? rTCNTO1 : rTCNTO0);
}
