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

PHDRS
{
   code_seg		PT_LOAD;
   rodata_seg	PT_LOAD;
   data_seg		PT_LOAD;
}

SECTIONS
{
	.text :
	{
		*(.text.vector)
		*(.text.startup)
		*(.text)
		*(.text.*)	
		
	} > DRAM : code_seg
	
	.rodata :
	{
		*(.rodata)
		*(.rodata.*)
			
	} > DRAM : rodata_seg
	
	.data :
	{
		*(.data)
		
	} > DRAM : data_seg
	
	.bss :
	{
		. = ALIGN(4);
		__bss_start__ = .;
		*(.bss)
		*(COMMON)
		. = ALIGN(4);
		__bss_end__ = .;
		
	} > DRAM : data_seg
	
	.stack :
	{
		*(.stack)
						
	}   > DRAM : data_seg
}
