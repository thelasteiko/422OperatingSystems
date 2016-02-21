/*
 * pque.h
 *
 *  Created on: January 5, 2016
 *      Author: Melinda Robertson, Chetayana Chopra, Jason Hall, Shewangizaw Gebremariam
 *     Version: February 4 2016
 *
 *     Header file for priority queue.
 */

#ifndef MY_PQUE
#define MY_PQUE

#include "que.h"
#include "pcb.h"

typedef struct pque_type {
	 que_ptr priorityQue[MAXPRI+1];
     int node_count;
} pque;

typedef pque * pque_ptr;
// constructs a pque
pque_ptr pq_constructor();
// pushes in new node
int pq_enqueue(pque_ptr this, pcb_ptr new_node);
// returns and removes the next node in the pque
pcb_ptr pq_dequeue(pque_ptr this);
// returns the next node in the pque
pcb_ptr pq_peek(pque_ptr this);
// returns a string information regarding the
char * pque_toString(pque_ptr this);
// Deallocates memory dedicated to the pque.
int pque_destructor(pque_ptr this);
#endif