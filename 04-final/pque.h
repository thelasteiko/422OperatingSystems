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

typedef struct pque_type {
	 que_ptr priorityQue[16];
} pque;

typedef pque * pque_ptr;
// constructs a pque
pque_ptr pque_constructor();
// pushes in new node
ppush(pcb_ptr new_node, pque_ptr this);
// returns and removes the next node in the pque
pcb_ptr ppop(pque_ptr this);
// returns the next node in the pque
pcb_ptr ppeek(pque_ptr this);
// returns a string information regarding the
char * toString2();
// Deallocates memory dedicated to the pque.
int pque_destructor(pque_ptr this);
#endif