///////////////////////////////////////////////////////////////////////////////
//	
//						Copyright 2013 xxxxxxx, xxxxxxx
//	File:	os_timer.h
//	Author:	Bala B. (bhat.balasubramanya@gmail.com)
//	Description: OS Timer implementation
//	
///////////////////////////////////////////////////////////////////////////////

#ifndef _OS_TIMER_H_
#define _OS_TIMER_H_

#include "os_core.h"
#include "os_types.h"

// Initializes both timer 0 & 1
void _OS_InitTimer ();

// The following function sets up the OS timer.
// Input:
// 		delay_in_us: Is the delay requested
// Output:
//		delay_in_us: Is the actual delay setup. Since every timer has a maximum interval,
//		the delay setup may be less than the delay requested.
BOOL _OS_UpdateTimer(UINT32 * delay_in_us);

// The following function sets up the budget timer.
// Input:
// 		delay_in_us: Is the delay requested
// Output:
//		delay_in_us: Is the actual delay setup. Since every timer has a maximum interval,
//		the delay setup may be less than the delay requested.
UINT32 _OS_SetBudgetTimer(UINT32 * delay_in_us);

// This function should be called whenever the timer o/1 interrupt happens.
// This function clears the interrupt bits
// Input:
//		timer: 0/1
void _OS_TimerInterrupt(UINT32 timer);

// Read the timer value in uSec
// Input:
//		timer: 0/1
UINT32 _OS_GetTimerValue_us(UINT32 timer);

#endif // _OS_TIMER_H_