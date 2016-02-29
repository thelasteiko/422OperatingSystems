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

pque_ptr pq_constructor() {
    /*Create a new priority queue.*/
	pque_ptr q = (pque_ptr)malloc(sizeof(pque));
	int i;
	for (i = 0; i < TOTALQUE; i = i+1) {
        q->priorityQue[i] = que_constructor();
	}
    q->node_count = 0;
	return q;
}

int pq_enqueue(pque_ptr this, pcb_ptr new_node) {
    /*Adds a PCB to the appropriate priority queue.*/
	int index = pcb_get_priority(new_node);
    if (index > TOTALQUE) {
        printf("Error enqueueing in priq.");
        return 1;
    }
	que_ptr add_Here = this->priorityQue[index];
	q_enqueue(add_Here, new_node);
    this->node_count = this->node_count + 1;
    return 0;
}

pcb_ptr pq_dequeue(pque_ptr this) {
    /*Dequeues and returns a PCB pointer.*/
	int index = 0;
	while (this->priorityQue[index]->node_count == 0
        && index < TOTALQUE) {
		index = index + 1;
	}
    if (index > TOTALQUE) return NULL;
	pcb_ptr removed = q_dequeue((que_ptr) this->priorityQue[index]);
    this->node_count = this->node_count - 1;
	return removed;
}

pcb_ptr pq_peek(pque_ptr this) {
    /*Returns what will be removed upon dequeueing.*/
	int index = 0;
	while (this->priorityQue[index]->node_count == 0) {
		index = index + 1;
	}
	return q_peek((que_ptr) this->priorityQue[index]);
}

int pq_updatepri(pque_ptr this) {
    int i;
    for (i = 1; i < TOTALQUE; i = i + 1) {
        pcb_ptr p = q_updatepri(this->priorityQue[i]);
        if (p) {
            pq_enqueue(this, p);
        }
    }
    return 0;
}

que_ptr pq_minpri(pque_ptr this) {
    int i;
    for (i = 0; i < TOTALQUE; i = i + 1) {
        if (this->priorityQue[i]->node_count > 0) {
            return this->priorityQue[i];
        }
    }
    return this->priorityQue[0];
}

char * pq_toString(pque_ptr this) {
/*Prints the priority queue by calling individual queueu toStrings.*/
	char * str = (char *) malloc(sizeof(char) * 10000);
    char * cur = (char *) malloc(sizeof(char) * 200);
    int i;
    for (i = 0; i < TOTALQUE; i = i + 1) {
        if (this->priorityQue[i]->node_count > 0) {
            sprintf(cur,"%d%s\n", i, q_toString(this->priorityQue[i]));
            strcat(str, cur);
        }
    }
	return str;
}

int pque_destructor (pque_ptr this) {
    /*Deallocates the memory used for a priority queue.*/
    int i;
    for (i = 0; i < TOTALQUE; i = i + 1) {
        q_destructor(this->priorityQue[i]);
    }
    free(this);
    return 0;
}
