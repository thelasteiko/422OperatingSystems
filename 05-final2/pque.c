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

pque_ptr pq_constructor(int size) {
    /*Create a new priority queue.*/
	pque_ptr q = (pque_ptr)malloc(sizeof(pque));
	int i;
	for (i = 0; i < size; i = i+1) {
        q->priorityQue[i] = que_constructor();
	}
  q->node_count = 0;
  q->size = size;
	return q;
}

int pq_enqueue(pque_ptr this, void * new_node, int index) {
    /*Adds a PCB to the appropriate priority queue.*/
  //printf("Enqueing %s\r\n", pcb_toString(new_node));
  if (!new_node) {
    printf("Error: No node for priq.\r\n");
    return 1;
  }
  if (!this) {
    printf("Error: No priq.\r\n");
  }
	//int index = pcb_get_priority(new_node);
  if (index > this->size-1 || index < 0) {
      printf("Error: Wrong index for priq; %d\r\n", index);
      return 1;
  }
	que_ptr add_Here = this->priorityQue[index];
	q_enqueue(add_Here, new_node);
  this->node_count = this->node_count + 1;
  return 0;
}

void * pq_dequeue(pque_ptr this) {
  /*Dequeues and returns a PCB pointer.*/
  //printf("Dequeing...\r\n");
  void * removed = NULL;
	int index = 0;
	while (this->priorityQue[index]->node_count <= 0
        && index < this->size-1) {
		index = index + 1;
	}
  if (index >= this->size) {
    return 0;
  }
	removed = q_dequeue(this->priorityQue[index]);
  this->node_count = this->node_count - 1;
	return removed;
}

void * pq_peek(pque_ptr this) {
    /*Returns what will be removed upon dequeueing.*/
	int index = 0;
	while (this->priorityQue[index]->node_count == 0) {
		index = index + 1;
	}
	return q_peek(this->priorityQue[index]);
}

que_ptr pq_minpri(pque_ptr this) {
    int i;
    for (i = 0; i < this->size; i = i + 1) {
        if (this->priorityQue[i]->node_count > 0) {
            return this->priorityQue[i];
        }
    }
    return this->priorityQue[0];
}

que_ptr pq_getq(pque_ptr this, int index) {
  return this->priorityQue[index];
}

int pq_getcount(pque_ptr this) {
  int i, sum = 0;
  for (i = 0; i < this->size; i = i + 1) {
    sum = sum + this->priorityQue[i]->node_count;
  }
  this->node_count = sum;
  return sum;
}

char * pq_toString(pque_ptr this) {
/*Prints the priority queue by calling individual queueu toStrings.*/
	char * str = (char *) malloc(sizeof(char) * 1000);
    char * cur = (char *) malloc(sizeof(char) * 200);
    int i;
    strcat(str, "PQue:\r\n");
    for (i = 0; i < this->size; i = i + 1) {
        //if (this->priorityQue[i]->node_count > 0) {
            sprintf(cur,"%d%s\r\n", i, q_toString(this->priorityQue[i]));
            strcat(str, cur);
        //}
    }
	return str;
}

int pque_destructor (pque_ptr this) {
    /*Deallocates the memory used for a priority queue.*/
    int i;
    for (i = 0; i < this->size; i = i + 1) {
        q_destructor(this->priorityQue[i]);
    }
    free(this);
    return 0;
}
