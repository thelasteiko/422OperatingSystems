/*
 * Move the scheduler to a different class
 * for clarity. The dispatcher, scheduler and
 * queues should be here.
 */

#include "pcb.h"
#include "que.h"
#include "cpu.h"
#include <stdlib.h>
#include <time.h>

int cntx = 0;
int cntx2 = 0;

int random1(int min, int max) {
    return (rand() % (max-min)) + min;
}

sch_ptr sch_constructor () {
    sch_ptr sched = (sch_ptr) malloc(sizeof(sch));
    sched->enq = que_constructor();
    sched->rdyq = que_constructor();
    sched->iowait1 = que_constructor();
    sched->iowait2 = que_constructor();
    sched->deadq = que_constructor();
    return sched;
}

int time_inter (sch_ptr this) {
    if (this->timer == 0) {
        this->timer = MAXTIME;
        return 1;
    }
    else this->timer = this->timer - 1;
    return 0;
}

int io_1_inter (sch_ptr this) {
    /*Track if there is a interrupt for
     * IO device 1.*/
    if (this->iotime1 == 0) {
        this->iotime1 = random1(MAXTIME * 2, MAXTIME * 3);
        return 1;
    }
    else if (this->iowait1->node_count > 0)
        this->iotime1 = iotime1 - 1;
    return 0;
}

int io_2_inter (sch_ptr this) {
    /*Track if there is a interrupt for
     * IO device 2.*/
    if (this->iotime2 == 0) {
        this->iotime2 = random1(MAXTIME * 2, MAXTIME * 3);
        return 1;
    }
    else if (this->iowait2->node_count > 0)
        this->iotime2 = iotime2 - 1;
    return 0;
}

pcb_ptr make_pcb(int pid) {
    /*Create a randomized pcb with the pid.
    Need to include priority flag and mutex array.*/
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

int sch_init_pcb(sch_ptr this, int pid) {
    /*Initialize some PCBs to be run.*/
    int i = (rand()) % 6;
    while(i) {
        pcb_ptr node = make_pcb(pid);
        q_enqueue(this->enq, node);
        pid = pid + 1;
        i = i - 1;
    }
    return pid;
}

int sch_ready (sch_ptr this) {
    /*Move all items from the enqueueing queue to the ready queue.*/
	printf("\r\nEnqueing %d pcb's.\r\n", enq->node_count);
	while(enq->node_count) {
        pcb_ptr node = q_dequeue(enq);
        printf("Process has been enqueued --> PCB Contents: %s\r\n", pcb_toString(node));
        q_enqueue(rdyq, node);
    }
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
    that->pseudostack = that->pc;
    enum state_type inter = current->state;
    switch(inter) {
        case interrupted:
        current->state = ready;
        next = dispatcher(rdyq, rdyq, current);
        next->state = running;
        that->pseudostack = next->pc;
        break;
        
        case wait1:
        next = dispatcher(iowait1, rdyq, current);
        next->state = running;
        that->pseudostack = next->pc;
        break;
        
        case wait2:
        next = dispatcher(iowait2, rdyq, current);
        next->state = running;
        that->pseudostack = next->pc;
        break;
        
        case ioready1:
        next = q_dequeue(iowait1);
        next->state = ready;
        q_enqueue(rdyq, next);
        current->state = running;
        next = current;
        break;
        
        case ioready2:
        next = q_dequeue(iowait2);
        next->state = ready;
        q_enqueue(rdyq, next);
        current->state = running;
        next = current;
        break;
        
        case dead:
        //put in deadq and return next rdyq pcb
        next = dispatcher(deadq, rdyq, current);
        next->state = running;
        that->pseudostack = next->pc;
        break;
    }
    that->pc = that->pseudostack;
    return next;
}

#endif
