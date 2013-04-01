/********************************************************************************
	
						Copyright 2012-2013 xxxxxxx, xxxxxxx
	File:	memmap_$(APP).ld
	Author:	Bala B. (bhat.balasubramanya@gmail.com)
	Description: Linker script for the Application image
	
********************************************************************************/

OUTPUT_ARCH(arm)
ENTRY(_start)

MEMORY 
{
	OS_MEM		: ORIGIN = 0x30000000,  LENGTH = 0x100000	
	APP_MEM		: ORIGIN = 0x30100000,  LENGTH = 0x100000	
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
		
	} > APP_MEM : code_seg
	
	. = ALIGN(4096);
	.rodata :
	{
		*(.rodata)
		*(.rodata.*)
			
	} > APP_MEM : rodata_seg
	
	. = ALIGN(4096);
	.data :
	{
		*(.data)
		
	} > APP_MEM : data_seg
	
	.bss :
	{
		. = ALIGN(4);
		__bss_start__ = .;
		*(.bss)
		*(COMMON)
		. = ALIGN(4);
		__bss_end__ = .;
		
	} > APP_MEM : data_seg
	
	.stack :
	{
		*(.stack)
						
	} > APP_MEM : data_seg
}
