///////////////////////////////////////////////////////////////////////////////
//	
//						Copyright 2013 xxxxxxx, xxxxxxx
//	File:	os_process.c
//	Author:	Bala B. (bhat.balasubramanya@gmail.com)
//	Description: OS Task Related routines
//	
///////////////////////////////////////////////////////////////////////////////

#include "os_process.h"
#include "os_core.h"

OS_Process * g_process_list_head;
OS_Process * g_process_list_tail;

OS_Process * g_current_process;

OS_Error OS_CreateProcess(
	OS_Process *process,
	const INT8 *process_name,
	void (*process_entry_function)(void *pdata),
	void *pdata)
{
	UINT32 intsts;
	
	if(!process)
	{
	    FAULT("Invalid process");
		return INVALID_ARG;
	}

    if(!process_entry_function)
	{
	    FAULT("One or more invalid %s arguments", "process");
		return INVALID_ARG;
	}
	
	// Copy process name
	strncpy(process->name, process_name, OS_PROCESS_NAME_SIZE);
	
	process->process_entry_function = process_entry_function;
	process->pdata = pdata;
	process->next = NULL;
	
	// Add to the process list
	OS_ENTER_CRITICAL(intsts); 	// Enter the critical section
	if(g_process_list_tail)
	{
		g_process_list_tail->next = process;
	}
	else
	{
		g_process_list_head = g_process_list_tail = process;
	}
	OS_EXIT_CRITICAL(intsts); 	// Exit the critical section
}
