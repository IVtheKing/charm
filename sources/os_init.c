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
extern void OS_InitTimer();
extern void S3C2440_SystemInit();
extern OS_Queue g_ready_q;
extern OS_Queue g_wait_q;
extern OS_Queue g_ap_ready_q;
extern OS_Queue g_block_q;

///////////////////////////////////////////////////////////////////////////////
// Initialization function for the OS
///////////////////////////////////////////////////////////////////////////////
void OS_Init(int argc, char *argv[])
{
	// Call system initializatoin routine
	S3C2440_SystemInit();
	
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
	OS_InitTimer();

	// Initialize the global queue for timer waiting
	OS_QueueInit(&g_ready_q); 
	OS_QueueInit(&g_wait_q);
	OS_QueueInit(&g_ap_ready_q);
	OS_QueueInit(&g_block_q);
}


///////////////////////////////////////////////////////////////////////////////
// Initialization of Interrupts processing
///////////////////////////////////////////////////////////////////////////////
void OS_InitInterrupts()
{
	UINT32 intsts = 0;
	
	// Enable IRQ and FIQ interrupts
	_enable_interrupt(intsts);
}
