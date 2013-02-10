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


OS_Error OS_MutexInit(OS_Mutex *mutex);
OS_Error OS_MutexLock(OS_Mutex *mutex);
OS_Error OS_MutexUnlock(OS_Mutex *mutex);
OS_Error OS_MutexDestroy(OS_Mutex *mutex);

#endif //_OS_MUTEX_H
