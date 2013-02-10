///////////////////////////////////////////////////////////////////////////////
//	
//						Copyright 2009-2013 xxxxxxx, xxxxxxx
//	File:	test_aperiodic.c
//	Author:	Bala B. (bhat.balasubramanya@gmail.com)
//	Description: Sorting test
//	
///////////////////////////////////////////////////////////////////////////////

#include "os_core.h"
#include "target.h"

#define WORSTCASE 1
#define FALSE 0
#define TRUE 1
#define NUMELEMS 200
#define MAXDIM   (NUMELEMS+1)

int g_array[NUMELEMS];


/* BUBBLESORT BENCHMARK PROGRAM:
 * This program tests the basic loop constructs, integer comparisons,
 * and simple array handling of compilers by sorting 10 arrays of
 * randomly generated integers.
 */

//int Array[MAXDIM];

void Initialize(Array)
	int Array[];
	/*
	 * Initializes given array with randomly generated integers.
	 */

{
	int  Index, factor;

#ifdef WORSTCASE
	factor = -1;
#else
	factor = 1;
#endif

	for (Index = 0; Index < NUMELEMS; Index ++)
		Array[Index] = Index*factor;
}



void BubbleSort(Array)
	int Array[];
	/*
	 * Sorts an array of integers of size NUMELEMS in ascending order.
	 */
{
	int Sorted = FALSE;
	int Temp, Index, i;

	/* Created by Sibin M on May 11, 2005.
	 * to reflect true bubble sort.
	 */
	for (i = 0; i < NUMELEMS; i++) 
	{ 
		Sorted = TRUE; 
		for (Index = 0; Index < ( NUMELEMS - ( i + 1 ) ) ; Index ++) { 
			/*
			   if (Index > NUMELEMS-i) 
			   break; 
			 */

			if (Array[Index] > Array[Index + 1]) 
			{ 
				Temp = Array[Index]; 
				Array[Index] = Array[Index+1]; 
				Array[Index+1] = Temp; 
				Sorted = FALSE; 
			} 
		} 

		if (Sorted) 
			break; 
	} 
}

int TestResults(Array)
	int Array[];
	/*
	 * Initializes given array with randomly generated integers.
	 */

{
	int  Index;
	int result = 1;
	for (Index = 1; Index < NUMELEMS; Index ++)
		result = result && (Array[Index-1] <= Array[Index]);

	return result;
}

void srt_main(Array)
	int Array[];
{
	int ret = 1;
	Initialize(Array);
	BubbleSort(Array);

	ret = TestResults(Array);

	if(ret)
	{	
		// Correct Value
		user_led_toggle(3);
	}
	else
	{
		// The LED should stop blinking
		while(1);
	}
}


/*
   int ttime()*/
/*
 * This function returns in milliseconds the amount of compiler time
 * used prior to it being called.
 */
/*
   {
   struct tms buffer;
   int utime;

   times(&buffer);
   utime = (buffer.tms_utime / 60.0) * 1000.0;
   return(utime);
   }

 */
