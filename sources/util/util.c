///////////////////////////////////////////////////////////////////////////////
//	
//						Copyright 2013 xxxxxxx, xxxxxxx
//	File:	util.c
//	Author:	Bala B. (bhat.balasubramanya@gmail.com)
//	Description: Utility functions
//	
///////////////////////////////////////////////////////////////////////////////

#include "util.h"
#include "os_config.h"
#include "os_core.h"

INT8 *strncpy(INT8 *dest, const INT8 *src, UINT32 n)
{
	INT32 i = 0;
	
	if(!dest || !n) return NULL;	

	if(src)
	{
		for (; i < n && src[i] != '\0'; i++)
		{
			dest[i] = src[i];
		}
	}
	for (; i < n; i++)
	{
	    dest[i] = '\0';
	}
	return dest;
}

INT8 *strcpy(INT8 *dest, const INT8 *src)
{
	INT32 i = 0;
	
	if(!dest) return NULL;	
	
	if(src)
	{
		for (; src[i] != '\0'; i++)
		{
			dest[i] = src[i];
		}	
	}
	
	dest[i] = '\0';

	return dest;
}

INT8 *itoa64(UINT64 value, INT8 *str)
{
	UINT32 i = 0;
	UINT8 nibble;
	UINT32 len;
	
	if(!str) return NULL;
	
	while(value) 
	{
		nibble = value & 0x0f;
		value >>= 4;
	
		if(nibble < 10) {
			str[i++] = '0' + nibble;
		}
		else {
			str[i++] = 'a' + nibble - 10;
		}
	}
	if(i == 0) {
		str[i++] = '0';		// Just in case the number is 0
	}
	str[i++] = 'x';
	str[i++] = '0';
	len = i;
	
	// Now we have to reverse the string
	for(i = len >> 1; i > 0; i--)
	{
		// Exchange i-1 & len - i
		INT8 temp = str[i - 1];
		str[i - 1] = str[len - i];
		str[len - i] = temp;
	}
	
	str[len] = '\x0';
	
	return str;
}

INT8 *itoa(UINT32 value, INT8 *str)
{	
	if(!str) return NULL;
	return itoa64(value, str);	
}

// The input string and output number are in bcd format
INT8 bcda2bcdi(const INT8 *str, UINT32 *value)
{
	if(!str || !value) return ARGUMENT_ERROR;

	INT8 ch;
	*value = 0;
	
	while((ch = *(str++)))
	{
		if(ch >= '0' && ch <= '9')
		{
			*value <<= 4;
			*value |= (ch - '0');
		}
		else
		{
			return FORMAT_ERROR;
		}
	}
	
	return SUCCESS;
}

// The input number and output string are in bcd format
INT8 bcdi2bcda(UINT32 value, INT8 *str)
{
	if(!str || !value) return ARGUMENT_ERROR;
	int len = 0;
	int i;

	while(value)
	{
		str[len++] = '0' + (value & 0xf);
		value >>= 4;		
	}
	
		// Now we have to reverse the string
	for(i = len >> 1; i > 0; i--)
	{
		// Exchange i-1 & len - i
		INT8 temp = str[i - 1];
		str[i - 1] = str[len - i];
		str[len - i] = temp;
	}
	
	str[len] = '\0';
	
	return SUCCESS;
}

