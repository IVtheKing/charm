///////////////////////////////////////////////////////////////////////////////
//
//						Copyright 2013 xxxxxxx, xxxxxxx
//	File:	os_mutex.c
//	Author:	Bala B. (bhat.balasubramanya@gmail.com)
//	Description: OS Mutex implementation
//
///////////////////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////////////////
//Add function prototypes here
//mysem_init - initialises the semaphopre
//input : sem -  pointer of type OS_Sem
//        pShared - not used in current implementation
//        value - semaphore initial value
// returns: 0 for SUCCESS, non-zero for ERROR
///////////////////////////////////////////////////////////////////////
#include "os_core.h"
#include "os_sem.h"
#include "os_mutex.h"

extern OS_PeriodicTask g_cur_task;//temporary var

OS_Error OS_MutexInit(OS_Mutex *mutex)
{
	if(!mutex)
		return ARGUMENT_ERROR;

	return OS_SemInit(&mutex->binary_sem,0,1);
}

OS_Error OS_MutexLock(OS_Mutex *mutex)
{
	if(!mutex)
		return ARGUMENT_ERROR;
		
	return OS_SemWait(&mutex->binary_sem);
}

OS_Error OS_MutexUnlock(OS_Mutex *mutex)
{
	if(!mutex)
		return ARGUMENT_ERROR;

	return OS_SemPost(&mutex->binary_sem);
}

OS_Error OS_MutexDestroy(OS_Mutex *mutex)
{
	if(!mutex)
		return ARGUMENT_ERROR;

	return OS_SemDestroy(&mutex->binary_sem);
}
