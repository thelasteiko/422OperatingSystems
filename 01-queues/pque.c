/*
 * pque.c
 *
 *  Created on: January 12 2016
 *      Author: Chetanya Chopra, Melinda Robertson
 *     Version: January 15 2016
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
	for (i = 0; i < MAXPRI+1; i++) {
        q->priorityQue[i] = que_constructor();
	}
	return q;
}

int pq_add(pque_ptr this, pcb_ptr new_node) {
	int index = pcb_get_priority(new_node);
	que_ptr add_Here = this->priorityQue[index];
	q_add(add_Here, new_node);
	int p = pcb_get_priority(new_node);
    return 0;
}

pcb_ptr pq_remove(pque_ptr this) {
	int index = 0;
	while (this->priorityQue[index]->node_count == 0) {
		index = index + 1;
	}
	pcb_ptr removed = q_remove((que_ptr) this->priorityQue[index]);
	enum state_type state = running;
	pcb_set_state(removed, state);
	return removed;
}

pcb_ptr pq_peek(pque_ptr this) {
	int index = 0;
	while (this->priorityQue[index]->node_count == 0) {
		index = index + 1;
	}
	return q_peek((que_ptr) this->priorityQue[index]);
}

char * pq_toString(pque_ptr this) {
/*  author: Melinda Robertson
    Prints priority queue.
*/
    int i, size = 0, max = 0;
    for (i = 0; i < MAXPRI+1; i = i + 1) {
        if (this->priorityQue[i]->node_count > 0) {
            size = size + ((this->priorityQue[i]->node_count)*64)+10;
            if (this->priorityQue[i]->node_count > max)
                max = this->priorityQue[i]->node_count;
        }
    }
    max = ((max*4)+10+64);
    size = max * size;
	char * str = (char *) malloc(sizeof(char) * size);
    char * cur = (char *) malloc(sizeof(char) * max);
    for (i = 0; i < MAXPRI+1; i = i + 1) {
        if (this->priorityQue[i]->node_count > 0) {
            sprintf(cur,"%d%s\n", i, q_toString(this->priorityQue[i]));
            strcat(str, cur);
        }
    }

	return str;
}
