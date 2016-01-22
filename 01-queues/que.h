/*
* que.h
*
*  Created on: January 10, 2016
*      Author: Jason Hall
*     Version: January 10, 2016
*
*     Header file for Que.
*/

#include "pcb.h"

#ifndef MY_QUE
#define MY_QUE

typedef struct que_type {
	struct pcb_type * first_node;
	struct pcb_type * last_node;
	int node_count;
} que;

typedef que * que_ptr;
// Creates a que.
que_ptr que_constructor();
// to put stuff onto the que
int q_add(que_ptr this, pcb_ptr new_node);
// get the first item from the que
pcb_ptr q_remove(que_ptr this);
// peek at the first item in the que
pcb_ptr q_peek(que_ptr this);
// Deallocates the memory dedicated to the que.
int que_destructor(que_ptr this);
// The information for the pcb's
char * q_toString(que_ptr this);
#endif