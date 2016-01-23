/*
*  Jason Hall
*/

#include "pcb.h"
#include "que.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//http://stackoverflow.com/questions/2999075/generate-a-random-number-within-range/2999130#2999130
//by Jerry Coffin
int rand_lim(int limit) {
	/* return a random number between 0 and limit inclusive.
	*/
	int divisor = RAND_MAX / (limit + 1);
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
    //int pid = rand();
    int reg[NUMREGS];
    int i;
    for(i = 0; i < NUMREGS; i++) {
        reg[i] = rand();
    }
    pcb_initialize(node, pid, rstate, NULL, reg, r);
    return node;
}

print_que(que_ptr list, pcb_ptr node) {
	printf(q_toString(list));
	printf("; ");
	printf(pcb_toString(node));
	printf("\n");
}

fill_que(que_ptr list, int min, int max) {
	int times = (rand() % (max - min)) + min;
	int x = 1;
	int r = 0;
	enum state_type rstate = ready;
	int reg[NUMREGS];
	int i;

	while(x < times) {
		pcb_ptr node = get_random(x);
		q_enqueue(list, node);
        printf("%s\n", q_toString(list));
		printf("; ");
		printf(pcb_toString(node));
		printf("\n");
		//printf("Last node: %s\n", pcb_toString(list->last_node));
		//print_que(list, node);
        x = x+1;
	}
    printf("%s\n", q_toString(list));
}

peek_que(que_ptr list) {
	printf(q_toString(list));
	printf("\n");
	printf(pcb_toString(q_peek(list)));
	printf("\n");
	printf(q_toString(list));
	printf("\n");
}

empty_que(que_ptr list) {
	while (list->node_count > 0) {
		printf(q_toString(list));
		printf("\n");
		printf(pcb_toString(q_dequeue(list)));
		printf("\n");
	}
	printf(q_toString(list));
	printf("\n");
}

int main(void) {
	/* Tests the process control block data structure.*/
	printf("Start QUE Test\n");
	pcb_ptr node = NULL;
	que_ptr list = que_constructor();
    char * str;
    str = q_toString(list);
    printf("Empty Queue: %s\n", str);
    
	printf("Fill que:\n");
	fill_que(list, 10, 30);
	printf("Peek que:\n");
	peek_que(list);
	printf("Empty que:\n");
	empty_que(list);
	printf("End QUE Test\n");
}