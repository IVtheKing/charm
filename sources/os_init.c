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
#include "os_process.h"
#include "mmu.h"

// External functions used in here
extern void _OS_InitTimer();
extern void _OS_SystemInit();
extern _OS_Queue g_ready_q;
extern _OS_Queue g_wait_q;
extern _OS_Queue g_ap_ready_q;
extern _OS_Queue g_block_q;

OS_Process	g_kernel_process;	// Kernel process

void _OS_Init();
void _OS_Exit();
void kernel_process_entry(void * pdata);

///////////////////////////////////////////////////////////////////////////////
// Initialization function for the OS
///////////////////////////////////////////////////////////////////////////////
void _OS_Init()
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
	
	// Initialize the Kernel process
	OS_CreateProcess(&g_kernel_process, "kernel", &kernel_process_entry, NULL);

	// Calling main which will start the scheduling by calling OS_Start
	main(1, " chARM");
	
	// The main function should never return if it called OS_Start. 
	// If it does not call we may return from main.
	_OS_Exit();
}

///////////////////////////////////////////////////////////////////////////////
// This function terminates the OS and shuts down the system
///////////////////////////////////////////////////////////////////////////////
void _OS_Exit()
{
	// TODO: Stop scheduling & shutdown the system
	panic("Shutdown");
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
