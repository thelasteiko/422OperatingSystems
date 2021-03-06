/* cpu.c
 *
 *  Created on: January 21 2016
 *      Author: Melinda Robertson, Chetayana
 *     Version: January 25 2016
 *
 *      Has a loop to simulate a CPU.
 */

#include "pcb.h"
#include "que.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
//holds the PC value when changing PCBs
unsigned int pseudostack;
int cntx = 0;
int cntx2 = 0;

typedef struct cpu_type {
    int pc;
} cpu;

typedef cpu * cpu_ptr;

pcb_ptr make_pcb(int pid) {
    /*Create a randomized pcb with the pid.*/
    pcb_ptr node = pcb_constructor();
    int r = (rand() % 15);
    enum state_type rstate = ready;
    int reg[NUMREGS];
    int i;
    for(i = 0; i < NUMREGS; i++) {
        reg[i] = rand();
    }
    int pc = rand();
    pcb_initialize(node, pid, rstate, pc, reg, r);
    return node;
}

cpu_ptr cpu_constructor() {
    /*Construct a CPU that holds a PC and register values.*/
    cpu_ptr cpu = (cpu_ptr) malloc(sizeof(cpu));
    cpu->pc = 0;
    int reg[NUMREGS];
    int i;
    for(i = 0; i < NUMREGS; i = i+1) {
        reg[i] = 0;
    }
    memcpy(cpu->reg_file, reg, NUMREGS);
    return cpu;
}

int sch_init_pcb(que_ptr enq, int pid) {
    /*Initialize some PCBs to be run.*/
    int i = (rand()) % 6;
    while(i) {
        pcb_ptr node = make_pcb(pid);
        q_enqueue(enq, node);
        pid = pid + 1;
        i = i - 1;
    }
    return pid;
}

int sch_ready(que_ptr enq, que_ptr rdyq) {
    /*Move all items from the enqueueing queue to the ready queue.*/
	printf("\r\nEnqueing %d pcb's.\r\n", enq->node_count);
	while(enq->node_count) {
        pcb_ptr node = q_dequeue(enq);
        printf("Process has been enqueued --> PCB Contents: %s\r\n", pcb_toString(node));
        q_enqueue(rdyq, node);
    }
    return 0;
}

pcb_ptr dispatcher(cpu_ptr this, que_ptr rdyq, pcb_ptr that) {
    /*Move the current PCB to the ready queue and return the next one.*/
    that->pc = this->pc;
    memcpy(that->reg_file, this->reg_file, NUMREGS);
    q_enqueue(rdyq, that);
    
    if (cntx >= 3) {
		printf("\r\n%s Switching to: %s", pcb_toString(that), pcb_toString(q_peek(rdyq)));
		cntx = 0;
	} else if (cntx <= 3) {
		cntx = cntx + 1;
	}

    pcb_ptr current = q_dequeue(rdyq);
    memcpy(this->reg_file, current->reg_file, NUMREGS);
    current->state = running;
    pseudostack = current->pc;

    if (cntx2 >= 3) {
		printf("\r\n%s Switching to: %s\r\n", pcb_toString(that), pcb_toString(current));
		printf("%s\r\n", q_toString(rdyq));
		cntx2 = 0;
	} else if (cntx2 <= 3) {
		cntx2 = cntx2 + 1;
	}

    return current;
}

pcb_ptr scheduler(cpu_ptr this, que_ptr rdyq, pcb_ptr that, enum state_type inter) {
    /*Determine what to do based on state.*/
    switch(inter) {
        case interrupted:
        that->state = ready;
        return dispatcher(this, rdyq, that);
        break;
        case ready:
        //something far in the future
        break;
    }
    //something went wrong
    return NULL;
}

pcb_ptr isr(cpu_ptr this, que_ptr rdyq, pcb_ptr current) {
    //save state
    current->pc = this->pc;
    //set to interrupted
    current->state = interrupted;
    //call scheduler
    pcb_ptr newcurrent = scheduler(this, rdyq, current, current->state);
    //pseudo push to stack
    this->pc = pseudostack;
    return newcurrent;
}

int cpu_loop(cpu_ptr this) {
     //make a loop, runs ?? times
     int run = 100;
     unsigned int pid = 0;
     unsigned int addto;
     //queues
     que_ptr enq = que_constructor();
     que_ptr rdyq = que_constructor();
     //create some initial values (PCBs)
     pid = sch_init_pcb(enq, pid);
     sch_ready(enq, rdyq);
     //set an initial PCB
     pcb_ptr current = make_pcb(pid);
     pid = pid + 1;
     pseudostack = current->pc;
     this->pc = pseudostack;

     while(run) {
         if(pid < 28) {
            //create 0-5 processes; add to enqueueing queue
            pid = sch_init_pcb(enq, pid);
            //move processes from the enqueueing queue to the ready queue
            sch_ready(enq, rdyq);
         }
         //pseudo-run the process; ie add 3000-4000 to the PC
         addto = ((rand() % 1000) + 3000);
         this->pc = this->pc + addto;
         //call isr -> scheduler -> dispatcher
         current = isr(this, rdyq, current);
         //timer interrupt
         run = run - 1;
     }

 }
int main() {
	 cpu_ptr cpu1 = cpu_constructor();
	 cpu_loop(cpu1);
	 return 0;
}
