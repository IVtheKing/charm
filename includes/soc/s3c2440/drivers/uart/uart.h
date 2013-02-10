///////////////////////////////////////////////////////////////////////////////
//	
//						Copyright 2013 xxxxxxx, xxxxxxx
//	File:	uart.h
//	Author:	Bala B. (bhat.balasubramanya@gmail.com)
//	Description: S3C2440 UART Serial Driver
//	
///////////////////////////////////////////////////////////////////////////////

#ifndef _UART_H_
#define _UART_H_

#include "os_types.h"

typedef enum {
	UART0,
	UART1,
	UART2
	} UART_Channel;
	
#define UART_BAUD_RATE	115200
#define UART_LOOPBACK_MODE	0
#define UART_FIFO_SIZE	64

void Uart_Init(UART_Channel ch);
void Uart_Print(UART_Channel ch, const INT8 *buf);
void Uart_Write(UART_Channel ch, const INT8 *buf, UINT32 count);
void Uart_Read(UART_Channel ch, INT8 *buf, UINT32 count);

#endif // _UART_H_
