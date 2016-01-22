/*
 * que.c
 *
 *  Created on: January 10 2016
 *      Author: Jason Hall, Melinda Robertson
 *     Version: January 15 2016
 *
 */

#include "pcb.h"
#include "que.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*Creates a que.*/
que_ptr que_constructor() {
	que_ptr q = (que_ptr)malloc(sizeof(que));
	q -> first_node = NULL;
	q -> last_node = NULL;
	q -> node_count = 0;
	return q;
}
/*to put stuff onto the que*/
int q_add(que_ptr this, pcb_ptr new_node) {
	if (!this->first_node) {
		this -> first_node = new_node;
		this -> last_node = new_node;
		this->node_count = 1;	
	}
	else {
		pcb_set_next(this->last_node, new_node);
		this->last_node = new_node;
		this->node_count = this->node_count + 1;
	}
    return 0;
}
/*get the first item from the que*/
pcb_ptr q_remove(que_ptr this) {
	pcb_ptr p = (pcb_ptr) this->first_node;
	if (p != NULL) {
		this -> first_node = pcb_get_next(p);
		this -> node_count = this->node_count - 1;
	}
	return p;
}
/*peek at the first item in the que*/
pcb_ptr q_peek(que_ptr this) {
	return (pcb_ptr) this -> first_node;
}
/*Deallocates the memory dedicated to the que.*/
int que_destructor(que_ptr this) {
	free(this);
    return 0;
}
/*The information for the pcb's*/
char * q_toString(que_ptr this) {
/*  author: Melinda Robertson
    Prints the PIDs of each PCB and the last node.
*/
	pcb_ptr node = NULL;
    if (!this) return "NO Q";
    char * str = (char *) malloc((sizeof(char) * this->node_count * (4))+10+60);
    strcat(str, "Q: ");
	if (this->node_count > 0) {
        char * cur = (char *) malloc(sizeof(char) * 5);
        node = this->first_node;
        sprintf(cur, "P%d-", pcb_get_pid(node));
        strcat(str, cur);
        node = pcb_get_next(node);
        while(node) {
            strcat(str, ">");
            sprintf(cur, "P%d-", pcb_get_pid(node));
            strcat(str, cur);
            node = pcb_get_next(node);
        }
        strcat(str, "* : contents: ");
        node = this->last_node;
        strcat(str, pcb_toString(node));
	} else return "Q: EMPTY";
	return str;
}