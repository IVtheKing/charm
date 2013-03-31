///////////////////////////////////////////////////////////////////////////////
//	
//						Copyright 2013 xxxxxxx, xxxxxxx
//	File:	os_process.h
//	Author:	Bala B. (bhat.balasubramanya@gmail.com)
//	Description: OS Process Related routines
//	
///////////////////////////////////////////////////////////////////////////////

#ifndef _OS_PROCESS_H
#define _OS_PROCESS_H

#include "os_types.h"
#include "os_config.h"

///////////////////////////////////////////////////////////////////////////////
// Process PCB 
///////////////////////////////////////////////////////////////////////////////
typedef struct OS_Process
{
	INT8 name[OS_PROCESS_NAME_SIZE];
	void (*process_entry_function)(void *pdata);
	void * pdata;
	UINT16 id;
		
	// Pointer to next process in the list
	struct OS_Process *next;	
} OS_Process;

extern OS_Process * g_process_list_head;
extern OS_Process * g_process_list_tail;

extern OS_Process * g_current_process;
extern UINT16 g_process_id_counter;

extern OS_Process	g_kernel_process;	// Kernel process


#endif // _OS_PROCESS_H
