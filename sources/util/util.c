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

INT32 strcmp(const INT8 *str1, const INT8 *str2)
{
    register const UINT8 *s1 = (const UINT8 *) str1;
    register const UINT8 *s2 = (const UINT8 *) str2;
    register UINT8 c1, c2;
    
    do
    {
        c1 = (UINT8) *s1++;
        c2 = (UINT8) *s2++;
        if (c1 == '\0') break;
    } 
    while (c1 == c2); 
    
    return (c1 - c2);
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

void* memset(void * ptr, UINT32 ch, UINT32 len)
{
	UINT8 * p = ptr;
	UINT32 i;
	
	for (i = 0; i < len; i++) {
		p[i] = ch;
	}
	
	return ptr;
}

//////////////////////////////////////////////////////////////////////////////////////////
// This function finds an available resource given a bit mask of resource availability
// Uses binary search to find an unused bit index
//////////////////////////////////////////////////////////////////////////////////////////
INT32 GetFreeResIndex(UINT32 res_mask, UINT32 msb, UINT32 lsb)
{
	if(msb >= 32 || lsb >= 32 || lsb > msb) return -1;
	
	if(lsb == msb) 
	{
		return (~res_mask & (1 << lsb)) ? lsb : -1;
	}
	
	UINT32 midb = (msb + lsb) >> 1;
	
	UINT32 lmask = ((2 << midb) - 1) - ((1 << lsb) - 1);
	if(~res_mask & lmask) 
	{
		return GetFreeResIndex(res_mask, midb, lsb);
	}
	
	UINT32 hmask = ((2 << msb) - 1) - ((1 << midb) - 1);
	if(~res_mask & hmask) 
	{
		return GetFreeResIndex(res_mask, msb, midb);
	}
	
	// If we reach this code, we did not find any resource available
	return -1;	
}

void * memcpy(void *dst, const void *src, UINT32 len)
{
    UINT32 i;

    /*
     * memcpy does not support overlapping buffers, so always do it
     * forwards. (Don't change this without adjusting memmove.)
     *
     * For speedy copying, optimize the common case where both pointers
     * and the length are word-aligned, and copy word-at-a-time instead
     * of byte-at-a-time. Otherwise, copy by bytes.
     *
     * The alignment logic below should be portable. We rely on
     * the compiler to be reasonably intelligent about optimizing
     * the divides and modulos out. Fortunately, it is.
     */
    
    if((((UINT32)dst & 0x3) == 0) && (((UINT32)src & 0x3) == 0) && ((len & 0x3) == 0))
    {
        long * d = dst;
        const long *s = src;
        
        for(i = 0; i < (len >> 2); i++)
        {
            d[i] = s[i];
        }
    }
    else
    {
        char *d = dst;
        const char *s = src;
        
        for(i = 0; i < len; i++)
        {
            d[i] = s[i];
        }
    }
    
    return dst;
}
