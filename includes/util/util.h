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
#include "uart.h"

INT8 *strncpy(INT8 *dest, const INT8 *src, UINT32 n);
INT8 *strcpy(INT8 *dest, const INT8 *src);

INT8 *itoa64(UINT64 value, INT8 *str);
INT8 *itoa(UINT32 value, INT8 *str);

INT8 bcda2bcdi(const INT8 *str, UINT32 *value);
INT8 bcdi2bcda(UINT32 value, INT8 *str);

// Non Blocking single ASCII character read. Returns 0 if there is no data
#define getchar() Uart_GetChar(DEBUG_UART_CHANNEL)

// Non Blocking single ASCII character write. Returns 0 if FIFO is full, 1 if character is written.
#define putchar(ch) Uart_PutChar(DEBUG_UART_CHANNEL, ch)

#endif // _UTIL_H
