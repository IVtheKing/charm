///////////////////////////////////////////////////////////////////////////////
//	
//						Copyright 2013 xxxxxxx, xxxxxxx
//	File:	util.c
//	Author:	Bala B. (bhat.balasubramanya@gmail.com)
//	Description: Utility functions
//	
///////////////////////////////////////////////////////////////////////////////

#include "Util.h"

INT8 *strncpy(INT8 *dest, const INT8 *src, UINT32 n)
{
	INT32 i;

   	for (i = 0; i < n && src[i] != '\0'; i++)
        dest[i] = src[i];
	for ( ; i < n; i++)
	    dest[i] = '\0';

	return dest;
}

INT8 *strcpy(INT8 *dest, const INT8 *src)
{
	INT32 i;

   	for (i = 0; src[i] != '\0'; i++)
        dest[i] = src[i];

	dest[i] = '\0';

	return dest;
}

INT8 *itoa64(UINT64 value, INT8 *str)
{
	UINT32 i = 0;
	UINT8 nibble;
	UINT32 len;
	
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
	return itoa64(value, str);	
}
