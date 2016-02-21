/*
 * Move the scheduler to a different class
 * for clarity. The dispatcher, scheduler and
 * queues should be here.
 */

#include "pcb.h"
#include "que.h"
#include "cpu.h"
#include "sch.h"
#include <stdlib.h>
#include <stdio.h>
#include <time.h>

int cntx = 0;
int cntx2 = 0;
int pseudostack = 0;
int iop = 0;

int random1(int min, int max) {
    /*Creates a random number, yay.*/
    time_t seed;
    seed = time(NULL);
    srand((unsigned int)seed);
    return (rand() % (max-min)) + min;
}

sch_ptr sch_constructor () {
    /*Creates a schedule object that keeps track of
     * and manages processes. */
    sch_ptr sched = (sch_ptr) malloc(sizeof(sch));
    sched->enq = que_constructor();
    sched->rdyq = que_constructor();
    sched->iowait1 = que_constructor();
    sched->iowait2 = que_constructor();
    sched->deadq = que_constructor();
    return sched;
}

pcb_ptr make_pcb(int pid, long rawTime) {
    /*Create a randomized pcb with the pid.
    Need to include priority flag and mutex array.*/
    pcb_ptr node = pcb_constructor();
    int prob = random1(1, 100);
    int pri = -1;
    if (prob <= 5) pri = 0;
    else if (prob <= 85) pri = 1;
    else if (prob <= 95) pri = 2;
    else pri = 3;
    int st = ready;
    unsigned int pc = 0;
    unsigned int mpc = (unsigned int) random1(MAXTIME * 3, MAXTIME * 4);
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
        0, t2, io1, io2);
    return node;
}

pcb_ptr sch_init(sch_ptr this, cpu_ptr that, int * pid) {
    *pid = sch_enqueue(this, that, *pid);
    sch_ready(this);
    pcb_ptr current = q_dequeue(this->rdyq);
    pseudostack = current->pc;
    that->pc = pseudostack;
    return current;
}

int sch_enqueue(sch_ptr this, cpu_ptr that, int pid) {
    /*Initialize some PCBs to be run.*/
    int i = random1(1, 5);
    while(i) {
        pcb_ptr node = make_pcb(pid, that->totaltime);
        q_enqueue(this->enq, node);
        pid = pid + 1;
        i = i - 1;
    }
    return pid;
}
int sch_ready (sch_ptr this) {
    /*Move all items from the enqueueing queue to the ready queue.*/
	printf("\r\nEnqueing %d pcb's.\r\n", this->enq->node_count);
	while(this->enq->node_count) {
        pcb_ptr node = q_dequeue(this->enq);
        printf("Process has been enqueued --> PCB Contents: %s\r\n", pcb_toString(node));
        q_enqueue(this->rdyq, node);
    }
    return 0;
}

pcb_ptr idle_process () {
    /*An idle process to keep the CPU busy.*/
    pcb_ptr idle = pcb_constructor();
    int io1[NUMTRAPS];
    int io2[NUMTRAPS];
    int i;
    for (i = 0; i < NUMTRAPS; i = i + 1) {
        io1[i] = -1;
        io2[i] = -1;
    }
    pcb_initialize(idle,-1,15, running, 0, 10,
        0, 1, io1, io2);
    return idle;
}

pcb_ptr dispatcher(que_ptr to, que_ptr from, pcb_ptr current) {
    /*Switches the current PCB.*/
    current->pc = pseudostack;
    if (cntx >= 3) {
        printf("\r\nSwitching from: %s\r\n", pcb_toString(current));
        cntx = 0;
    } else if (cntx <= 3) {
        cntx = cntx + 1;
    }
    
    if (current->pid == -1)
        pcb_destructor(current);
    else q_enqueue(to, current);
    
    printf("To %s\r\n", q_toString(to));
    
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

pcb_ptr scheduler(sch_ptr this, cpu_ptr that, pcb_ptr current) {
    /*Determine what to do based on state.*/
    pcb_ptr next = NULL;
    pseudostack = that->pc;
    enum state_type inter = current->state;
    switch(inter) {
        case interrupted:
        current->state = ready;
        next = dispatcher(this->rdyq, this->rdyq, current);
        next->state = running;
        //pseudostack = next->pc;
        break;
        
        case wait1:
        next = dispatcher(this->iowait1, this->rdyq, current);
        next->state = running;
        //pseudostack = next->pc;
        break;
        
        case wait2:
        next = dispatcher(this->iowait2, this->rdyq, current);
        next->state = running;
        //pseudostack = next->pc;
        break;
        
        case ioready1:
        next = q_dequeue(this->iowait1);
        next->state = ready;
        q_enqueue(this->rdyq, next);
        current->state = running;
        next = current;
        break;
        
        case ioready2:
        next = q_dequeue(this->iowait2);
        next->state = ready;
        q_enqueue(this->rdyq, next);
        current->state = running;
        next = current;
        break;
        
        case dead:
        //put in deadq and return next rdyq pcb
        next = dispatcher(this->deadq, this->rdyq, current);
        next->state = running;
        //pseudostack = next->pc;
        break;
    }
    that->pc = pseudostack;
    return next;
}

int sch_updatepri (sch_ptr this) {
    /*Update the priority levels of processes
     *to prevent starvation.*/
    return pq_updatepri(this->rdyq);
}

int sch_dumptrash(sch_ptr this) {
    /*Delete all the things in deadq.*/
    pcb_ptr current = q_dequeue(this->deadq);
    while (current) {
        pcb_destructor(current);
        current = q_dequeue(this->deadq);
    }
    return 0;
}

int sch_destructor(sch_ptr this) {
    /*Deallocates a schedule object.*/
    q_destructor(this->enq);
    q_destructor(this->rdyq);
    q_destructor(this->iowait1);
    q_destructor(this->iowait2);
    q_destructor(this->deadq);
    free(this);
    return 0;
}
