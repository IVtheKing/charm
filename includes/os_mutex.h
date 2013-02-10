///////////////////////////////////////////////////////////////////////////////
//	
//						Copyright 2009-2013 xxxxxxx, xxxxxxx
//	File:	os_sem.h
//	Author:	Bala B. (bhat.balasubramanya@gmail.com)
//	Description: Header file for the OS Semaphore APIs
//	
///////////////////////////////////////////////////////////////////////////////

#ifndef _OS_MUTEX_H
#define _OS_MUTEX_H

#include "os_core.h"
#include "os_queue.h"
#include "os_sem.h"

typedef struct OS_Mutex
{
	OS_Sem binary_sem; 
	
} OS_Mutex;

#endif //_OS_MUTEX_H
