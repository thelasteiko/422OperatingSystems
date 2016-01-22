/*
* pque.h
* Chetanya Chopra
*/

#include "pcb.h"
#include "que.h"

#ifndef MY_PQUE
#define MY_PQUE

typedef struct pque_type {
	 que_ptr priorityQue[MAXPRI];
} pque;

typedef pque * pque_ptr;
// constructs a pque
pque_ptr pque_constructor();
// pushes in new node
int pq_add(pque_ptr this, pcb_ptr new_node);
// returns and removes the next node in the pque
pcb_ptr pq_remove(pque_ptr this);
// returns the next node in the pque
pcb_ptr pq_peek(pque_ptr this);
// returns a string information regarding the
char * pq_toString(pque_ptr this);
// Deallocates memory dedicated to the pque.
int pque_destructor(pque_ptr this);
#endif