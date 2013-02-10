///////////////////////////////////////////////////////////////////////////////
//	
//						Copyright 2013 xxxxxxx, xxxxxxx
//	File:	target.c
//	Author:	Bala B. (bhat.balasubramanya@gmail.com)
//	Description: Board specific functions source file
//	
///////////////////////////////////////////////////////////////////////////////

#include "target.h"
#include "soc.h"

// The LED0-3 are connected to GPB5..8
void configure_user_led(User_led_type led)
{
	// Configure corresponding GPB bit as output
	rGPBCON &= ~(0x3 << ((5 + led) << 1));	// Clear the bits
	rGPBCON |= (0x1 << ((5 + led) << 1)); 	// 01 for output pin
	
	// Disable Pullups
	rGPBUP |= (1 << (5 +led));
}

void user_led_on(User_led_type led)
{
	rGPBDAT |= (1 << (5 +led));
}

void user_led_off(User_led_type led)
{
	rGPBDAT &= ~(1 << (5 +led));	
}

void user_led_toggle(User_led_type led)
{
	rGPBDAT ^= (1 << (5 +led));		
}
