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
unsigned int pseudostack = 0;
unsigned int pc = 0;
int cntx = 0;
int cntx2 = 0;
int timer = MAXTIME;
int iotime1 = 0, iotime2 = 0;

que_ptr enq;
que_ptr rdyq;
que_ptr iowait1;
que_ptr iowait2;
que_ptr deadq;

int random1(int min, int max) {
    return (rand() % (max-min)) + min;
}

pcb_ptr make_pcb(int pid) {
    /*Create a randomized pcb with the pid.*/
    pcb_ptr node = pcb_constructor();
    int pri = random1(0, MAXPRI);
    int st = ready;
    unsigned int pc = 0;//(unsigned int) random1(MAXTIME, MAXTIME * 5);
    unsigned int mpc = (unsigned int) random1(MAXTIME * 3, MAXTIME * 8);
    //pc = 0;
    time_t cre;
    time(&cre);
    int t2 = random1(2, 15);
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

int sch_init_pcb(int pid) {
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

int sch_ready () {
    /*Move all items from the enqueueing queue to the ready queue.*/
	printf("\r\nEnqueing %d pcb's.\r\n", enq->node_count);
	while(enq->node_count) {
        pcb_ptr node = q_dequeue(enq);
        printf("Process has been enqueued --> PCB Contents: %s\r\n", pcb_toString(node));
        q_enqueue(rdyq, node);
    }
    return 0;
}

int time_inter () {
    if (timer == 0) {
        timer = MAXTIME;
        return 1;
    }
    else timer = timer - 1;
    return 0;
}

int io_1_inter () {
    //printf("IO 1 Time: %d\n", iotime1);
    if (iotime1 == 0) {
        iotime1 = random1(MAXTIME * 2, MAXTIME * 3);
        return 1;
    }
    else if (iowait1->node_count > 0)
        iotime1 = iotime1 - 1;
    return 0;
}

int io_2_inter () {
    if (iotime2 == 0) {
        iotime2 = random1(MAXTIME * 2, MAXTIME * 3);
        return 1;
    }
    else if (iowait2->node_count > 0)
        iotime2 = iotime2 - 1;
    return 0;
}

pcb_ptr idle_process () {
    pcb_ptr idle = pcb_constructor();
    time_t cre;
    time(&cre);
    int io1[NUMTRAPS];
    int io2[NUMTRAPS];
    int i;
    for (i = 0; i < NUMTRAPS; i = i + 1) {
        io1[i] = -1;
        io2[i] = -1;
    }
    pcb_initialize(idle,-1,15, running, 0, 10,
        cre, 1, io1, io2);
    return idle;
}

pcb_ptr dispatcher(que_ptr to, que_ptr from, pcb_ptr current) {
    current->pc = pseudostack;
    if (cntx >= 3) {
    printf("Switching from: %s\r\n", pcb_toString(current));
        cntx = 0;
    } else if (cntx <= 3) {
        cntx = cntx + 1;
    }
    
    if (current->pid == -1)
        pcb_destructor(current);
    else q_enqueue(to, current);
    printf("%s\r\n", q_toString(to));
    
    pcb_ptr next = NULL;
    if(from->node_count > 0)
        next = q_dequeue(from);
    if (!next) next = idle_process();
    
    pseudostack = next->pc;
    if (cntx2 >= 3) {
		printf("Switching to: %s\r\n", pcb_toString(next));
		printf("%s\r\n\r\n", q_toString(from));
		cntx2 = 0;
	} else if (cntx2 <= 3) {
		cntx2 = cntx2 + 1;
	}
    return next;
}

pcb_ptr scheduler(pcb_ptr current, enum state_type inter) {
    /*Determine what to do based on state.*/
    pcb_ptr next = NULL;
    pseudostack = pc;
    switch(inter) {
        case interrupted:
        //printf("Interrupted: %s\n", pcb_toString(current));
        current->state = ready;
        next = dispatcher(rdyq, rdyq, current);
        next->state = running;
        pseudostack = next->pc;
        break;
        
        case wait1:
        next = dispatcher(iowait1, rdyq, current);
        next->state = running;
        //printf("Waiting: %s\n", pcb_toString(next));
        pseudostack = next->pc;
        break;
        
        case wait2:
        next = dispatcher(iowait2, rdyq, current);
        next->state = running;
        //printf("Waiting: %s\n", pcb_toString(next));
        pseudostack = next->pc;
        break;
        
        case ioready1:
        next = q_dequeue(iowait1);
        next->state = ready;
        q_enqueue(rdyq, next);
        //printf("IO 1 Ready: PID %d\n", q_peek(rdyq)->pid);
        //printf("%s\n", q_toString(rdyq));
        break;
        
        case ioready2:
        next = q_dequeue(iowait2);
        next->state = ready;
        q_enqueue(rdyq, next);
        break;
        
        case dead:
        //put in deadq and return next rdyq pcb
        //printf("Dead: %s\n", pcb_toString(current));
        next = dispatcher(deadq, rdyq, current);
        next->state = running;
        pseudostack = next->pc;
        break;
    }
    pc = pseudostack;
    return next;
}

pcb_ptr io_trap_handle(pcb_ptr current, int device) {
    if (device == 1) {
        if (iotime1 == 0)
            iotime1 = random1(MAXTIME * 3, MAXTIME * 4);
        current->state = wait1;
    }
    else if (device == 2) {
        if (iotime2 == 0)
            iotime2 = random1(MAXTIME * 3, MAXTIME * 4);
        current->state = wait2;
    }
    return scheduler(current, current->state);
    //return current;
}

pcb_ptr time_inter_handle(pcb_ptr current) {
    current->state = interrupted;
    pcb_ptr next = scheduler(current, current->state);
    //printf("Time Returning: %s\n", pcb_toString(next));
    return next;
}

pcb_ptr term_inter_handle (pcb_ptr current) {
    current->state = dead;
    time(&current->termination);
    //pcb_ptr next = scheduler(current, current->state);
    //printf("Terminate Returning: %s\n", pcb_toString(next));
    return scheduler(current, current->state);
    //return current;
}

pcb_ptr io_inter_handle (pcb_ptr current, enum state_type device) {
    scheduler(current, device);
    return current;
}

int cpu_loop () {
    printf("Starting...\r\n");
    int run = 10000;
    unsigned int pid = 0;
    time_t rawTime;
    struct tm * timeinfo;
    time(&rawTime);
    timeinfo = localtime(&rawTime);
    pid = sch_init_pcb(pid);
     printf("Process created: PID %d at %s\r\n", pid, asctime  (timeinfo));
     sch_ready();
     pcb_ptr current = make_pcb(pid);
     pid = pid+1;
     pseudostack = current->pc;
     pc = pseudostack;
     
     while (run) {
        time(&rawTime);
        timeinfo = localtime(&rawTime);
         if(pid < 30) {
            pid = sch_init_pcb(pid);
            sch_ready();
         }
         pc = pc + 1;
         //printf("Timer: %d\n", timer);
         if(time_inter()) {
             printf("Timer interrupt: PID %d at %s\r\n", current->pid, asctime (timeinfo));
             current = time_inter_handle(current);
         }
         //current->pc = pc;
         if (current->pc >= current->max_pc) {
             current->termcount = current->termcount + 1;
             current->pc = 0;
         }
         if (current->termcount >= current->terminate) {
             printf("Process terminated: PID %d at %s\r\n", current->pid, asctime (timeinfo));
             current = term_inter_handle(current);
         }
         if (io_1_inter()) {
             printf("IO 1 Complete: PID %d at %s\r\n", q_peek(iowait1)->pid, asctime (timeinfo));
             current = io_inter_handle(current, ioready1);
             //printf("%s\n", q_toString(rdyq));
         }
         if (io_2_inter()) {
             printf("IO 2 Complete: PID %d at %s\r\n", q_peek(iowait2)->pid, asctime (timeinfo));
             current = io_inter_handle(current, ioready2);
             //printf("%s\n", q_toString(rdyq));
         }
         int i;
         for (i = 0; i < NUMTRAPS; i = i + 1) {
            if (current->IO_1_TRAPS[i] == pc) {
                printf("IO 1 Trap: PID %d at %s\r\n", current->pid, asctime (timeinfo));
                current = io_trap_handle(current, 1);
                break;
            }
            if (current->IO_2_TRAPS[i] == pc) {
                printf("IO 2 Trap: PID %d at %s\r\n", current->pid, asctime (timeinfo));
                current = io_trap_handle(current, 2);
                break;
            }
         }
         run = run - 1;
         //if (rdyq->node_count == 0) break;
     }
     
     printf("Ready: %s\r\n", q_toString(rdyq));
     printf("IO1: %s\r\n", q_toString(iowait1));
     printf("IO2: %s\r\n", q_toString(iowait2));
     printf("Dead: %s\r\n", q_toString(deadq));
     return 0;
}

int main (void) {
    iotime1 = random1(MAXTIME * 3, MAXTIME * 4);
    iotime2 = random1(MAXTIME * 3, MAXTIME * 4);
    enq = que_constructor();
    rdyq = que_constructor();
    iowait1 = que_constructor();
    iowait2 = que_constructor();
    deadq = que_constructor();
    return cpu_loop();
}



