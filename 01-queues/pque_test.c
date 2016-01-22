/*
 * pque_test.c
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

//http://stackoverflow.com/questions/2999075/generate-a-random-number-within-range/2999130#2999130
//by Jerry Coffin
int rand_lim(int limit) {
/* return a random number between 0 and limit inclusive.
 */
    int divisor = RAND_MAX/(limit+1);
    int retval;
    do {
        retval = rand() / divisor;
    } while (retval > limit);
    return retval;
}

pcb_ptr get_random(int pid) {
    pcb_ptr node = pcb_constructor();
    int r = rand_lim(15);
    enum state_type rstate = ready;
    int reg[NUMREGS];
    int i;
    for(i = 0; i < NUMREGS; i++) {
        reg[i] = rand();
    }
    pcb_initialize(node, pid, rstate, NULL, reg, r);
    return node;
}

int test_loop(pque_ptr list) {
    int i, k;
    pcb_ptr mypcb;
    for(i = 1; i < 102; i = i + 1) {
        mypcb = get_random(i);
        pq_add(list, mypcb);
        if (i % 10 == 0) {
            printf("Queue %d:\n", i);
            printf(pq_toString(list));
            k = ((rand()+4)%6)+1;
            printf("Dequeueing %d: \n", k);
            while(k > 0) {
                mypcb = pq_remove(list);
                printf("Removed %s\n", pcb_toString(mypcb));
                k = k - 1;
            }
        }
    }
    return 0;
}

int main() {
    printf("Priority Queue Test START\n");
	pque_ptr list = pque_constructor();

    test_loop(list);
    
    printf("Priority Queue Test END\n");

	return 0;
}
