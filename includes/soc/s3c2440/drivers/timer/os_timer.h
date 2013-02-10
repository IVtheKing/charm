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

void OS_InitTimer ();
BOOL OS_UpdateTimer(UINT32 delay_in_us);
UINT32 OS_SetBudgetTimer(UINT32 delay_in_us);
void Handle_TimerInterrupt(UINT32 timer);
UINT32 OSGetTime();

#endif // _OS_TIMER_H_