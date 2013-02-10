///////////////////////////////////////////////////////////////////////////////
//	
//						Copyright 2009-2013 xxxxxxx, xxxxxxx
//	File:	os_init.c
//	Author:	Bala B. (bhat.balasubramanya@gmail.com)
//	Description: OS Initialization routines
//	
///////////////////////////////////////////////////////////////////////////////

#include "os_core.h"
#include "os_queue.h"
#include "mmu.h"

// External functions used in here
extern void _OS_InitTimer();
extern void _OS_SystemInit();
extern _OS_Queue g_ready_q;
extern _OS_Queue g_wait_q;
extern _OS_Queue g_ap_ready_q;
extern _OS_Queue g_block_q;

///////////////////////////////////////////////////////////////////////////////
// Initialization function for the OS
///////////////////////////////////////////////////////////////////////////////
void OS_Init(int argc, char *argv[])
{
	// Call system initializatoin routine
	_OS_SystemInit();
	
	// Initialize Instruction and Data Caches
#if ENABLE_INSTRUCTION_CACHE == 1
	MMU_flushICache();
	MMU_EnableICache();
#endif

#if ENABLE_DATA_CACHE == 1
	MMU_flushDCache();
	MMU_EnableDCache();
#endif

	// Start the scheduling timer
	_OS_InitTimer();

	// Initialize the global queue for timer waiting
	_OS_QueueInit(&g_ready_q); 
	_OS_QueueInit(&g_wait_q);
	_OS_QueueInit(&g_ap_ready_q);
	_OS_QueueInit(&g_block_q);
}


///////////////////////////////////////////////////////////////////////////////
// Initialization of Interrupts processing
///////////////////////////////////////////////////////////////////////////////
void _OS_InitInterrupts()
{
	UINT32 intsts = 0;
	
	// Enable IRQ and FIQ interrupts
	_enable_interrupt(intsts);
}
