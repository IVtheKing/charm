///////////////////////////////////////////////////////////////////////////////
//	
//						Copyright 2013 xxxxxxx, xxxxxxx
//	File:	util.h
//	Author:	Bala B. (bhat.balasubramanya@gmail.com)
//	Description: Utility function header file
//	
///////////////////////////////////////////////////////////////////////////////

#ifndef _UTIL_H
#define _UTIL_H

#include "os_types.h"

INT8 *strncpy(INT8 *dest, const INT8 *src, UINT32 n);
INT8 *strcpy(INT8 *dest, const INT8 *src);

INT8 *itoa64(UINT64 value, INT8 *str);
INT8 *itoa(UINT32 value, INT8 *str);

#endif // _UTIL_H
