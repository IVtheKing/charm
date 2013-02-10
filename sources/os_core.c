///////////////////////////////////////////////////////////////////////////////
//	
//						Copyright 2012-2013 xxxxxxx, xxxxxxx
//	File:	os_core.c
//	Author:	Bala B. (bhat.balasubramanya@gmail.com)
//	Description: 
//	
///////////////////////////////////////////////////////////////////////////////

#include "os_core.h"
#include "os_config.h"
#include "uart.h"
#include "util.h"

void panic(const INT8 * format, ...)
{
	// TODO: Need to print whole formatted string
	Uart_Print(DEBUG_UART_CHANNEL, format);
	
	// TODO: Implement this function
	while(1);
}

void SyslogStr(const INT8 * str, const INT8 * value)
{
	// TODO: OS Log should be handled using a different buffer to be fast
	if(str) {
		Uart_Print(DEBUG_UART_CHANNEL, str);
	}
	
	Uart_Print(DEBUG_UART_CHANNEL, value);
	Uart_Print(DEBUG_UART_CHANNEL, "\n");
}

void Syslog32(const INT8 * str, UINT32 value)
{
	INT8 valueStr[12];
	
	if(str) {
		Uart_Print(DEBUG_UART_CHANNEL, str);
	}
	
	itoa(value, valueStr);
	Uart_Print(DEBUG_UART_CHANNEL, valueStr);	
	Uart_Print(DEBUG_UART_CHANNEL, "\n");
}

void Syslog64(const INT8 * str, UINT64 value)
{
	INT8 valueStr[20];

	if(str) {
		Uart_Print(DEBUG_UART_CHANNEL, str);
	}
	
	itoa64(value, valueStr);
	Uart_Print(DEBUG_UART_CHANNEL, valueStr);	
	Uart_Print(DEBUG_UART_CHANNEL, "\n");
}
