/*
 * Move the scheduler to a different class
 * for clarity. The dispatcher, scheduler and
 * queues should be here. dispat
 */

#include "pcb.h"
#include "que.h"
#include "pque.h"
#include "cpu.h"
#include "sch.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <time.h>

int cntx = 0;
int cntx2 = 0;
int pseudostack = 0;
int iop = 0;
//char * proConStart = "ProducerConsumerPair:";
//int numOfProCon; // the number of different Producer/Consumers that there are currently.

int random1(int min, int max) {
    /*Creates a random number, yay.*/
    return (rand() % (max-min)) + min;
}

sch_ptr sch_constructor () {
    /*Creates a schedule object that keeps track of
     * and manages processes. */
    sch_ptr sched = (sch_ptr) malloc(sizeof(sch));
    sched->enq = que_constructor();
    sched->rdyq = pq_constructor();
    sched->iowait1 = que_constructor();
    sched->iowait2 = que_constructor();
    sched->deadq = que_constructor();
    //sched->mutexes = (mutex_ptr) malloc(sizeof(mutex_ptr)*MAXPAIR+MAXMUTUAL);
    //sched->prod_var = (cond_ptr) malloc(sizeof(cond_ptr)
    sched->numreg = 0;
    sched->numbusy = 0;
    sched->numpair = 0;
    sched->nummutual = MAXPAIR;
    return sched;
}

int * create_list (int min, int max, int * list) {
  /* Create a unique set of NUMTRAPS integers
   * based on the range given. */
  unsigned int last, i, next, k;
  //int list[NUMTRAPS];
  last = min;
  k = max / 10;
  next = k;
  for (i = 0; i < NUMTRAPS; i = i + 1) {
    if (next > max) next = max;
    list[i] = random1(last, next);
    last = list[i] + 1;
    next = last + k;
  }
  return list;
}

pcb_ptr make_regular (unsigned int pid, long rawTime, int pri,
  int mpc, int t2) {
  pcb_ptr this = pcb_constructor();
  //,pid, pri, state, type, pc, maxpc, cre, term, io1, io2
  int * list = (int *) malloc(sizeof(int)*NUMTRAPS);
  pcb_initialize(this, pid, pri, ready, regular,
    mpc, rawTime, t2);
  create_list(0, mpc, list);
  pcb_set_io1(this, list);
  create_list(list[NUMTRAPS-1], mpc, list);
  pcb_set_io2(this, list);
  return this;
}
pcb_ptr make_busy (unsigned int pid, long rawTime, int pri,
  int mpc, int t2) {
  pcb_ptr this = pcb_constructor();
  pcb_initialize(this, pid, pri, ready, busy,
    mpc, rawTime, t2);
  return this;
}
pcb_ptr make_producer (unsigned int pid, long rawTime, int pri,
  int mpc, int t2, int numpair) {
  //When determing where to lock, have it lock between IO
  //for a time before the next IO
  pcb_ptr this = pcb_constructor();
  int io1[NUMTRAPS] = {10, 50, 100, 200};
  int io2[NUMTRAPS] = {30, 70, 150, 300};
  int mtx[NUMTRAPS] = {35, 75, 155, 305};
  int mtxlock[NUMTRAPS] = {numpair,numpair,numpair,numpair};
  pcb_initialize(this, pid, pri, ready, producer, mpc,
    rawTime, t2);
  this->pairnumber = numpair;
  pcb_set_io1(this, io1);
  pcb_set_io2(this, io2);
  pcb_set_mtx(this, mtx);
  pcb_set_mtxlock(this, mtxlock);
  //producer: 00 ---> 12 chars
  char * name = (char *) malloc(sizeof(char) * 12);
  sprintf(name, "Producer: %d", this->pairnumber);
  pcb_set_name(this, name);
  return this;
}
pcb_ptr make_consumer (pcb_ptr prod, unsigned int pid, long rawTime,
  int pri, int mpc, int t2) {
  pcb_ptr this = pcb_constructor();
  pcb_initialize(this, pid, pri, ready, consumer, mpc,
    rawTime, t2);
  pcb_set_io1(this, prod->IO_1_TRAPS);
  pcb_set_io2(this, prod->IO_2_TRAPS);
  pcb_set_mtx(this, prod->mtx);
  pcb_set_mtxlock(this, prod->mtx_lockon);
  this->pairnumber = prod->pairnumber;
  //consumer: 00 ---> 12 chars
  char * name = (char *) malloc(sizeof(char) * 12);
  sprintf(name, "Consumer: %d", this->pairnumber);
  pcb_set_name(this, name);
  return this;
}
pcb_ptr make_mutual (unsigned int pid, long rawTime, int pri,
  int mpc, int t2, int nummutual) {
  //Mutual should be using the same mtx list but
  //it locks two mutex instead of one
  pcb_ptr this = pcb_constructor();
  int io1[NUMTRAPS] = {10, 50, 100, 200};
  int io2[NUMTRAPS] = {30, 70, 150, 300};
  //each mtx lock it locks 0 and 1
  int mtx[NUMTRAPS] = {35, 75, 155, 305};
  int mtxlock[NUMTRAPS] = {nummutual, nummutual+1, nummutual, nummutual+1}; //mutex list
  pcb_initialize(this, pid, pri, ready, mutual, mpc,
    rawTime, t2);
  pcb_set_io1(this, io1);
  pcb_set_io2(this, io2);
  pcb_set_mtx(this, mtx);
  pcb_set_mtxlock(this, mtxlock);
  this->pairnumber = nummutual;
  return this;
}
//Condition variables are created when a thread is blocked.
//mutexes are created when the pairs are
int make_pcb(sch_ptr this, cpu_ptr that, unsigned int pid) {
  /* Determine the priority and type then
   * call the appropriate making method.*/
  int pri = -1;
  int prob = random1(1, 100);
  if (prob <= 5) pri = 0;
  else if (prob <= 85) pri = 1;
  else if (prob <= 95) pri = 2;
  else pri = 3;
  unsigned int mpc = (unsigned int) random1(MAXTIME * 3, MAXTIME * 4);
  int t2 = random1(2, 15);
  //types: regular, busy, producer, consumer, mutual resource
  prob = random1(1, 100);
  if (pri == 0 && this->numbusy < MAXBUSY) {
    q_enqueue(this->enq, make_busy(pid, that->totaltime,
      pri, mpc, t2));
    pid = pid + 1;
    this->numbusy = this->numbusy + 1;
  } else if (prob <= 40) {
    if (this->numpair < MAXPAIR) {
      pcb_ptr prod = make_producer(pid, that->totaltime,
        pri, mpc, t2, this->numpair);
      pid = pid + 1;
      pcb_ptr cons = make_consumer(prod, pid,
        that->totaltime, pri, mpc, t2);
      pid = pid + 1;
      q_enqueue(this->enq, prod);
      q_enqueue(this->enq, cons);
      mutex_ptr m = mutex_constructor(this->numpair);
      //printf("Mtx: %p\n", this->mutexes);
      this->mutexes[this->numpair] = m;
      this->numpair = this->numpair + 1;
    } /*else if (this->nummutual < MAXMUTUAL) {
      q_enqueue(this->enq, make_mutual(pid, that->totaltime, pri, mpc, t2));
      pid = pid + 1;
      q_enqueue(this->enq, make_mutual(pid, that->totaltime, pri, mpc, t2));
      pid = pid + 1;
      mutex_ptr m = mutex_constructor(this->nummutual);
      this->mutexes[this->nummutual] = m;
      this->nummutual = this->nummutual + 1;
    }*/
  } else if (this->numreg < MAXREG) {
    q_enqueue(this->enq, make_regular(pid, that->totaltime, pri, mpc, t2));
    pid = pid + 1;
    this->numreg = this->numreg + 1;
  }
  return pid;
}
int sch_enqueue(sch_ptr this, cpu_ptr that, unsigned int pid) {
    /*Initialize some PCBs to be run.*/
    unsigned int i = random1(1, 5);
    //printf("Making: %d\n", i);
    while(i) {
        //printf("Index: %d\n", i);
        //printf("Diff: %d\n", p-pid);
        pid = make_pcb(this, that, pid);
        i = i - 1;
    }
    return pid;
}
int sch_ready (sch_ptr this) {
    /*Move all items from the enqueueing queue to the ready queue.*/
	printf("\r\nEnqueing %d pcb's.\r\n", this->enq->node_count);
	while(this->enq->node_count > 0) {
        pcb_ptr node = q_dequeue(this->enq);
        printf("Process has been enqueued --> PCB Contents: %s\r\n", pcb_toString(node));
        pq_enqueue(this->rdyq, node);
  }
  return 0;
}
pcb_ptr sch_init(sch_ptr this, cpu_ptr that, unsigned int * pid) {
    time_t seed;
    seed = time(NULL);
    srand((unsigned int)seed);
    *pid = sch_enqueue(this, that, *pid);
    //printf("After enqueue.");
    sch_ready(this);
    //printf("After ready.");
    pcb_ptr current = pq_dequeue(this->rdyq);
    pseudostack = current->pc;
    that->pc = pseudostack;
    return current;
}
pcb_ptr idle_process () {
    /*An idle process to keep the CPU busy.*/
    pcb_ptr idle = pcb_constructor();
    pcb_initialize(idle,-1,MAXPRI, running, busy, 1, 0, 1);
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
    
    //printf("To %s\r\n", q_toString(to));
    //printf("From: %s\r\n", q_toString(from));
    
    pcb_ptr next = NULL;
    if(from->node_count > 0)
        next = q_dequeue(from);
    if (!next) next = idle_process();
    
    pseudostack = next->pc;
    if (cntx2 >= 3) {
		printf("Switching to: %s\r\n", pcb_toString(next));
		printf("\r\n%s\r\n\r\n", q_toString(from));
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
    //printf("%s\n", pq_toString(this->rdyq));
    printf("Switching...%d : %d\r\n\r\n", current->state, current->type);
    int pri = pcb_get_priority(current);
    que_ptr from = pq_minpri(this->rdyq);
    que_ptr to = this->rdyq->priorityQue[pri];
    switch(inter) {
        case running:
        next = current;
        break;
        
        case ready:
        pq_enqueue(this->rdyq, current);
        break;
        
        case interrupted:
        current->state = ready;
        if (from->node_count <= 0)
          next = current;
        else
          next = dispatcher(to, from, current);
        next->state = running;
        //pseudostack = next->pc;
        break;
        
        case wait1:
        next = dispatcher(this->iowait1, from, current);
        next->state = running;
        //pseudostack = next->pc;
        break;
        
        case wait2:
        next = dispatcher(this->iowait2, from, current);
        next->state = running;
        //pseudostack = next->pc;
        break;
        
        case ioready1:
        next = q_dequeue(this->iowait1);
        next->state = ready;
        pq_enqueue(this->rdyq, next);
        current->state = running;
        next = current;
        break;
        
        case ioready2:
        next = q_dequeue(this->iowait2);
        next->state = ready;
        pq_enqueue(this->rdyq, next);
        current->state = running;
        next = current;
        break;
        
        //means it's in a mutex q
        //don't need to enqueue, just get next
        case blocked: //TODO send the mutex list to dispatcher
        current->pc = pseudostack;
        next = pq_dequeue(this->rdyq);
        next->state = running;
        pseudostack = next->pc;
        break;
        
        case dead:
        //put in deadq and return next rdyq pcb
        switch(current->type) {
          case regular:
          this->numreg = this->numreg - 1;
          break;
          case busy:
          this->numbusy = this->numbusy - 1;
          break;
          break;
          case consumer:
          this->numpair = this->numpair-1;
          break;
          case mutual:
          this->nummutual = this->nummutual-1;
          break;
        }
        next = dispatcher(this->deadq, from, current);
        next->state = running;
        //pseudostack = next->pc;
        break;
    }
    if (next) pcb_set_marker(next);
    that->pc = pseudostack;
    return next;
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
    pque_destructor(this->rdyq);
    q_destructor(this->iowait1);
    q_destructor(this->iowait2);
    q_destructor(this->deadq);
    free(this);
    return 0;
}
