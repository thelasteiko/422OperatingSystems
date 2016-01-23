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

//http://stackoverflow.com/questions/2999075/generate-a-random-number-within-range/2999130#2999130
//by Jerry Coffin
int rand_lim2(int limit) {
/* return a random number between 0 and limit inclusive.
 */
    int divisor = RAND_MAX/(limit+1);
    int retval;
    do {
        retval = rand() / divisor;
    } while (retval > limit);
    return retval;
}

pcb_ptr get_random2() {
    pcb_ptr node = pcb_constructor();
    int r = rand_lim2(15);
    enum state_type rstate = ready;
    int pid = rand();
    int reg[NUMREGS];
    int i;
    for(i = 0; i < NUMREGS; i++) {
        reg[i] = rand();
    }
    pcb_initialize(node, pid, rstate, NULL, reg, r);
    return node;
}

remove_randomly(pque_ptr prior) {
	int i;
	int r = (rand() % (6 - 4)) + 4;
	int index;
	for (i = 0; i <= r; i++) {
		pcb_ptr removed = ppop(prior);
		index = pcb_get_priority(removed);
		printf("Q%d:   --->", index);
		printf(pcb_toString(removed));
		printf("\n");
	}
}

print_pque(pque_ptr prior) {
	//printf("I MADE IT HERE");
	int i;
	char * que;
	for (i = 0; i < 16; i++) {
		que = q_toString(prior->priorityQue[i]);
		printf(que);
		printf("\n");
	}
}

int main() {
	pque_ptr priorque = pque_constructor();
	int i;
	int k;
	char * num = "";

	for (k = 1; k < 11; k++) {
		printf("Iteration ");
		sprintf(num, "%d", k);
		printf(num);
		printf(":\n");
		printf("Adding 10 PCB's to the Priority Queue:\n");
		for (i = 0; i < 10; i++) {
			ppush(get_random2(), priorque);
		}
		print_pque(priorque);
		printf("\n");
		printf("PCB's removed from the Priority Queue:\n");
		remove_randomly(priorque);
		printf("\n");
		printf("Priority Queue after removal:\n");
		print_pque(priorque);
		printf("\n");
	}
	return 0;
}
