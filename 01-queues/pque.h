/*
* pque.h
* Chetanya Chopra
*/

#ifndef MY_PQUE

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