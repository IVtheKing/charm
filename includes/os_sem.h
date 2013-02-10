///////////////////////////////////////////////////////////////////////////////
//	
//						Copyright 2009-2013 xxxxxxx, xxxxxxx
//	File:	os_sem.h
//	Author: Balasubramanya Bhat (bhat.balasubramanya@gmail.com)
//	Description: Header file for the OS Semaphore APIs
//	
///////////////////////////////////////////////////////////////////////////////

#ifndef _OS_SEM_H
#define _OS_SEM_H

#include "os_queue.h"

typedef struct OS_Sem
{
	INT32 count;
	_OS_Queue periodic_task_queue;    
	_OS_Queue aperiodic_task_queue;  
    
} OS_Sem;

#endif //_OS_SEM_H
