/*
* que.h
*
*  Created on: January 10, 2016
*      Author: Melinda Robertson, Chetayana, Jason Hall, Shewangizaw Gebremariam
*     Version: January 10, 2016
*
*     Header file for Que.
*/

#include "pcb.h"

#ifndef MY_QUE
#define MY_QUE

typedef struct node_type {
	struct pcb_type * my_pcb;
	struct node_type * next_node;
} node;

typedef struct que_type {
	struct node_type * first_node;
	struct node_type * last_node;
	int node_count;
} que;

// pointer for a node
typedef node * node_ptr;
// pointer for a que
typedef que * que_ptr;
// Creates a que.
que_ptr que_constructor();
// to put stuff onto the que
int q_enqueue(que_ptr this, pcb_ptr new_node);
// get the first item from the que
pcb_ptr q_dequeue(que_ptr this);
// peek at the first item in the que
pcb_ptr q_peek(que_ptr this);
// Deallocates the memory dedicated to the que.
int q_destructor(que_ptr this);
// The information for the pcb's
char * q_toString(que_ptr this);
#endif