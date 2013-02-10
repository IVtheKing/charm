///////////////////////////////////////////////////////////////////////////////
//	
//						Copyright 2009-2013 xxxxxxx, xxxxxxx
//	File:	os_queue.c
//	Author:	Bala B. (bhat.balasubramanya@gmail.com)
//	Description: OS Queue Implementation
//	
///////////////////////////////////////////////////////////////////////////////

#include "os_core.h"

// A #define redefinition of OSW_QueueNode for ease of use
typedef OS_QueueNode Node;  

// Function to initialize the queue 								 
OS_Error OS_QueueInit(OS_Queue * q)
{
	if(!q) return ARGUMENT_ERROR;
	q->head = q->tail = NULL;
	q->count = 0;
	 
	return SUCCESS;
}

// Function to insert an element into the queue. The key value determines the 
// location at which it will be inserted. This is a sorted queue on key value.
OS_Error OS_QueueInsert(OS_Queue * q, void * item, UINT64 key)
{
	volatile Node *node, *new_node, *prev = 0;
    if(!q) return ARGUMENT_ERROR;
	if(!item) return ARGUMENT_ERROR;

	node = q->head;
	while(node)
	{
		if(node->key > key) break;
		prev = node;
		node = node->next;
	}

	new_node = (Node*) item;
	new_node->key = key;
	new_node->next = node;	
	if(!node)q->tail = new_node;	
	if(prev)
	{
	 	prev->next = new_node;
	}
	else
	{
		q->head = new_node;		
	}	  
	q->count++;
	return SUCCESS;
}

// Function to insert an element into the tail end of the queue.
OS_Error OS_QueueInsertTail(OS_Queue * q, void * item)
{
	volatile Node *new_node;
    if(!q) return ARGUMENT_ERROR;
	if(!item) return ARGUMENT_ERROR;

	new_node = (Node*)item;
	new_node->next = NULL;

	if(q->tail)
	{
		(q->tail)->next = new_node;	
	}
	else
	{
		q->head = new_node; //first node
	}
	q->tail = new_node;

	return SUCCESS;
}
// Function to delete an item from the queue.
OS_Error OS_QueueDelete(OS_Queue * q, void * item)
{
    volatile Node * node, * prev = 0;
	volatile Node * item_node = (Node*)item;
    if(!q) return ARGUMENT_ERROR;
	
	node = q->head;
	while(node) 
	{
		if(node == item_node) break;
		prev = node;
		node = node->next;
	}
	
	if(node)   // Item exists
	{
		if(prev)	// Not the head node.
			prev->next = node->next;			
		else	// This is the head node
			q->head = node->next;	
		if(q->tail == node) // This is the last node
			q->tail = prev;

		q->count--;
		node->next = 0;		
	}
	else
		return NO_DATA;

	return SUCCESS;
}

// Function to get the first element from the Queue. 
OS_Error OS_QueueGet(OS_Queue * q, void ** item, UINT64 * key)
{
    volatile Node * node;
    if(!q) return ARGUMENT_ERROR;
	
	node = q->head;
	if(item) *item = (void *)node;
	if(node) 
	{
		q->head = node->next;		
		if(q->tail == node) q->tail = 0;
		if(key) *key = node->key;
		node->next = 0;
		q->count--;
	}
	else
	{
		return NO_DATA;
	}
	return SUCCESS;
}

OS_Error OS_QueuePeek(OS_Queue * q, void ** item, UINT64 * key)
{
	if(!q) return ARGUMENT_ERROR;	
	if(item) *item = (void*) q->head;
	if(q->head)
	{
		if(key) *key = q->head->key;
		return SUCCESS;
	}
	return NO_DATA;
}
