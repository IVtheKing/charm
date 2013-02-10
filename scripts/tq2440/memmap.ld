/********************************************************************************
	
						Copyright 2012-2013 xxxxxxx, xxxxxxx
	File:	os_task.h
	Author:	Bala B. (bhat.balasubramanya@gmail.com)
	Description: OS Task Related routines
	
********************************************************************************/

OUTPUT_ARCH(arm)
ENTRY(_start)

MEMORY 
{
/*	SRAM		: ORIGIN = 0x40000000,  LENGTH = 0x1000		*/
	DRAM		: ORIGIN = 0x30000000,  LENGTH = 0x4000000
}

SECTIONS
{
	.text :
	{
		*(.text.vector)
		
	} > DRAM

	.text :
	{
		*(.text.startup)
		
	} > DRAM

	.text :
	{
		*(.text)
		*(.text.*)		
		
	} > DRAM
	
	.data :
	{
		*(.data)
		
	} > DRAM
	
	.bss :
	{
		. = ALIGN(4);
		__bss_start__ = .;
		*(.bss)
		. = ALIGN(4);
		__bss_end__ = .;
		
	} > DRAM
	
	.stack :
	{
		*(.stack)
						
	}   > DRAM
}
