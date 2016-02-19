/*
 * pque_test.c
 *
 *  Created on: January 12 2016
 *      Author: Chetanya Chopra
 *     Version: January 13 2016
 *
 */

#include "pcb.h"
#include "que.h"
#include "pque.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


pque_ptr pque_constructor() {
	pque_ptr q = (pque_ptr)malloc(sizeof(pque));
	int i;
	for (i = 0; i < 16; i++) {
	q->priorityQue[i] = que_constructor();
	}
	return q;
}

ppush(pcb_ptr new_node, pque_ptr this) {
	int index = pcb_get_priority(new_node);
	que_ptr add_Here = this->priorityQue[index];
	q_add(add_Here, new_node);
	int p = pcb_get_priority(new_node);
	//printf("The PCB has been pushed with priority %d", p);
}

pcb_ptr ppop(pque_ptr this) {
	int index = 0;
	while (this->priorityQue[index]->node_count == 0) {
		index = index + 1;
	}
	pcb_ptr removed = q_remove((que_ptr) this->priorityQue[index]);
	enum state_type state = running;
	pcb_set_state(removed, state);
	return removed;
}

pcb_ptr ppeek(pque_ptr this) {
	int index = 0;
	while (this->priorityQue[index]->node_count == 0) {
		//printf(q_toString(q_peek(this->priorityQue[index])));
		index = index + 1;
	}
	return q_peek((que_ptr) this->priorityQue[index]);
}

char * toString2(pque_ptr this) {
	char * string = "";
	char * string2 = "";
	//char * string3 = "hello";
	while (ppeek(this) != NULL) {
		string = pcb_toString(ppeek(this));
		strcat(string2, string);
	}
	//printf(ppeek(this));

	return "";
}
