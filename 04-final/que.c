/*
* que.c
*
*  Created on: January 10, 2016
*      Author: Melinda Robertson, Chetayana, Jason Hall, Shewangizaw Gebremariam
*     Version: January 10, 2016
*
*		Used for a FiFo que.
*/

#include "pcb.h"
#include "que.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAXTIME 300

/*Creates a que.*/
que_ptr que_constructor() {
	que_ptr q = (que_ptr)malloc(sizeof(que));
	node_ptr n = (node_ptr)malloc(sizeof(node));
	//create a null node
	n -> my_pcb = NULL; 
	n -> next_node = NULL;
	// set the intial first and last to the null node;
	q -> first_node = n;
	q -> last_node = n;
	q -> node_count = 0;
	return q;
}
/*to put stuff onto the que*/
int q_enqueue(que_ptr this, pcb_ptr new_node) {
	//Make the new node to put in the que
	node_ptr n = (node_ptr)malloc(sizeof(node));
	n -> my_pcb = new_node;
	n -> next_node = NULL;

	// Check to see if the first node is null
	if (this->node_count == 0) {
		this -> first_node = n;
		this -> last_node = n;
		this->node_count = 1;	
	}
	else {
		this->last_node -> next_node = n;
		this->last_node = n;
		this->node_count = this->node_count + 1;
	}
    return 0;
}
/*get the first item from the que*/
pcb_ptr q_dequeue(que_ptr this) {
	pcb_ptr p = (pcb_ptr) this->first_node->my_pcb;
	if (p != NULL) {
		this->first_node = this->first_node->next_node;
		this->node_count = this->node_count - 1;
	}
	return p;
}
/*peek at the first item in the que*/
pcb_ptr q_peek(que_ptr this) {
	return (pcb_ptr) this -> first_node -> my_pcb;
}

pcb_ptr q_updatepri (que_ptr this) {
    node_ptr prev = (node_ptr) this->first_node;
    node_ptr p = NULL;
    if (prev->next_node) {
        node_ptr p = (node_ptr) prev->next_node;
    }
    while (p && prev) {
        pcb_ptr current = p->my_pcb;
        int k = current->pritimeout;
        int t = current->pridown;
        if (current->origpri != current->priority
            && k <= current->origpri * MAXTIME) {
            t = t - 1;
            if (t <= 0) {
                pcb_set_priority(current, current->origpri);
            }
           prev->next_node = p->next_node;
           this->node_count = this->node_count - 1;
           free (p);
           return current;
        }
        k = k - 1;
        if (k <= 0) {
           pcb_set_priority(current, current->priority + 1);
           //remove from list
           prev->next_node = p->next_node;
           this->node_count = this->node_count - 1;
           free (p);
           return current;
        }
        current->pritimeout = k;
        current->pridown = t;
        prev = p;
        p = p->next_node;
    }
    return NULL;
}
/*Deallocates the memory dedicated to the que.*/
int q_destructor(que_ptr this) {
	free(this);
    return 0;
}
/*The information for the pcb's*/
char * q_toString(que_ptr this) {
	//pcb_ptr curr_pcb = NULL;
	node_ptr node = NULL;

    if (!this) return "NO Q";
    char * str = (char *) malloc(sizeof(char) * ((this->node_count * (60+4))+10));
    strcat(str, "Q: ");

	if (this->node_count > 0) {
        char * cur = (char *) malloc(sizeof(char) * 5);
        node = this->first_node;
        sprintf(cur, "P%d-", pcb_get_pid(node -> my_pcb));
        strcat(str, cur);
		node = node -> next_node;
		while(node) {
            strcat(str, ">");
            sprintf(cur, "P%d-", pcb_get_pid(node -> my_pcb));
            strcat(str, cur);
			node = node->next_node;
        }
        strcat(str, "*");
		node = this->first_node;
	} else return "Q: EMPTY";
	return str;
}
