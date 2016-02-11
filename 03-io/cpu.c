/* cpu.c
 *
 *  Created on: January 21 2016
 *      Author: Melinda Robertson, Chetayana, Jason Hall, Shewangizaw Gebremariam
 *     Version: February 4 2016
 *
 *      Has a loop to simulate a CPU.
 */

#include "pcb.h"
#include "que.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define MAXTIME 300
//holds the PC value when changing PCBs
unsigned int pseudostack;
unsigned int pc = 0;
int cntx = 0;
int cntx2 = 0;
int timer = MAXTIME;
int iotime1 = 0, iotime2 = 0;

int random1(int min, int max) {
    return (rand() % (max-min)) + min;
}

pcb_ptr make_pcb(int pid) {
    /*Create a randomized pcb with the pid.*/
    pcb_ptr node = pcb_constructor();
    int pri = random1(0, MAXPRI);
    int st = ready;
    int pc = random1(MAXTIME, MAXTIME * 5);
    int mpc = random1(pc, pc * 5);
    time_t cre;
    time(&cre);
    int t2 = random1(0, 15);
    unsigned int last, i, next, k;
    int io1[NUMTRAPS];
    int io2[NUMTRAPS];
    last = pc;
    if (mpc - last < 1000) next = 50;
    else next = 100;

    for (i = 0; i < NUMTRAPS; i = i + 1) {
    	if (last > next) {
    		next = next + last;
    	}
        io1[i] = random1(last, next);
        //printf("last = %d next = %d random # produced = %d\n", last, next,io1[i]);
        k = random1(last, next);
        while(k == io1[i]) k = random1(last, next);
        io2[i] = k;
        last = next;
        if (last > mpc-1000) next = next+50;
        else next = next+100;
    }
    pcb_initialize(node, pid, pri, st, pc, mpc,
        cre, t2, io1, io2);
    return node;
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

int interruptTimer() {
    if (timer == 0) {
        timer = MAXTIME;
        return 1;
    }
    else timer = timer - 1;
    return 0;
}

int io_interrupt(que_ptr waiting) {
    if (iotime1 == 0) {
        if (waiting->node_count > 0)
            iotime1 = random1(MAXTIME * 3, MAXTIME * 4);
        return 1;
    }
    if (iotime2 == 0) {
        if (waiting->node_count > 0)
            iotime1 = random1(MAXTIME * 3, MAXTIME * 4);
        return 1;
    }
    else {
        iotime1 = iotime1 - 1;
        iotime2 = iotime2 - 1;
    }
    return 0;
}

pcb_ptr dispatcher(que_ptr rdyq, pcb_ptr that) {
    /*Move the current PCB to the ready queue and return the next one.*/
    if (that) {
        that->pc = pc;
        q_enqueue(rdyq, that);

        if (cntx >= 3) {
            printf("\r\n%s Switching to: %s", pcb_toString(that), pcb_toString(q_peek(rdyq)));
            cntx = 0;
        } else if (cntx <= 3) {
            cntx = cntx + 1;
        }
    }

    pcb_ptr current = q_dequeue(rdyq);
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



pcb_ptr scheduler(que_ptr rdyq, pcb_ptr that,
		enum state_type inter) {
    /*Determine what to do based on state.*/
    switch(inter) {
        case interrupted:
        that->state = ready;
        return dispatcher(rdyq, that);
        break;
        case waiting:
        return dispatcher(rdyq, NULL);
        break;
        case dead:
        //put in deadq and return next rdyq pcb
        return dispatcher(rdyq, NULL);
        break;
    }
    //something went wrong
    return NULL;
}


pcb_ptr io_trap_handle(que_ptr rdyq, que_ptr waiting_que, pcb_ptr node, int device) {
    node->state = waiting;
    if (device == 1 && iotime1 == 0)
        iotime1 = random1(MAXTIME * 3, MAXTIME * 4);
    else if (device == 2 && iotime2 == 0)
        iotime2 = random1(MAXTIME * 3, MAXTIME * 4);
    node->pc = pc;
    q_enqueue(waiting_que, node);
    return scheduler(rdyq, node, node->state);
}


pcb_ptr isr(que_ptr rdyq, pcb_ptr current) {
    //save state
    current->pc = pc;
    //set to interrupted
    current->state = interrupted;
    //call scheduler
    pcb_ptr newcurrent = scheduler(rdyq, current, current->state);
    //pseudo push to stack
    pc = pseudostack;
    return newcurrent;
}

int cpu_loop() {
	 printf("I STARTED\n");
     //make a loop, runs ?? times
     int run = 100000;
     unsigned int pid = 0;
     //queues
     que_ptr enq = que_constructor();
     que_ptr rdyq = que_constructor();
     que_ptr waiting = que_constructor();
     que_ptr deadq = que_constructor();
     time_t rawTime;
     struct tm * timeinfo;
     time(&rawTime);
     timeinfo = localtime (&rawTime);
     //create some initial values (PCBs)
     printf("Created queues\n");

     pid = sch_init_pcb(enq, pid);

     printf("Process created: PID %d at %s", pid, asctime  (timeinfo));

     sch_ready(enq, rdyq);

     //set an initial PCB
     pcb_ptr current = make_pcb(pid);
     pid = pid + 1;
     pseudostack = current->pc;
     pc = pseudostack;

     while(run) {
         if(pid < 28) {
            //create 0-5 processes; add to enqueueing queue
            pid = sch_init_pcb(enq, pid);
            //move processes from the enqueueing queue to the ready queue
            sch_ready(enq, rdyq);
         }
         pc = pc + 1;
         //if the current->pc is = maxPc then termcount++, c->pc = 0
         //if the termcount == terminate then current = isr(rdyq,current)
         printf("Timer: %d\n", timer);
         if(interruptTimer()) {
             //switch process
         //call isr -> scheduler -> dispatcher
             current = isr(rdyq, current);
             printf("Timer interrupt: PID %d at %s", pid, asctime (timeinfo));
         }
         if (current->pc == current->max_pc) {
        	 current->termcount = current->termcount + 1;
        	 current->pc = 0;

         }
         //printf("HERE\n");
         if (current->termcount == current->terminate) {
        	 current->state = dead;
        	 q_enqueue(deadq, current);
        	 current = scheduler(rdyq, NULL, current->state);
        	 printf("Process terminated: PID %d at %s", pid, asctime (timeinfo));
         }
         printf("after term\n");
         if (io_interrupt(waiting)) {
        	 if (waiting->node_count > 0) {
        		 printf("I/O completion interrupt: PID %d is running, PID %d put in ready queue\n",
        		             		current->pid, q_peek(waiting)->pid);
        	 }
             q_enqueue(rdyq, q_dequeue(waiting));
         }
         int i;
         for (i = 0; i < NUMTRAPS; i = i + 1) {
            if (current->IO_1_TRAPS[i] == pc) {
                current = io_trap_handle(rdyq, waiting, current, 1);
                printf("");
                break;
            }
            if (current->IO_2_TRAPS[i] == pc) {
                current = io_trap_handle(rdyq, waiting, current, 2);
                break;
            }
         }
         run = run - 1;
     }
     return 1;
 }
int main() {

	 cpu_loop();
	 return 0;
}
