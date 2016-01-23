/*
 * pcb_test.c
 *
 *  Created on: January 12 2016
 *      Author: Melinda Robertson
 *     Version: January 12 2016
 *
 *      Tests the process control block data structure.
 */

#include "pcb.h"
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

pcb_ptr get_random() {
    pcb_ptr node = pcb_constructor();
    int r = rand_lim(15);
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

int main(void) {
/* Tests the process control block data structure.*/
    printf("Start PCB Test\n");
    pcb_ptr block;
    block = pcb_constructor();
    char * str;
    str = pcb_toString(block);
    printf("Location: %p\n", block);
    printf("Uninitialized Block: %s\n", str);
    
    //random integer between 0 and 15
    pcb_ptr node = get_random();
    str = pcb_toString(node);
    printf("Location: %p\n", node);
    printf("Randomly Generated Block: %s\n", str);
    
    pcb_set_next(block, node);
    str = pcb_toString(block->next_pcb);
    printf("Linked: %s\n", str);
    
    printf("End PCB Test\n");
}