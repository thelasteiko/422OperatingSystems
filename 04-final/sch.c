
/* sch.c
 *
 *  Created on: February 21 2016
 *      Author: Melinda Robertson, Chetayana, Jason Hall, Shewangizaw Gebremariam
 *     Version: March 8 2016
 *
 *      The scheduler has many methods for making PCBs of
 *      various types. The type of PCB created is dependent on
 *      probability.
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
int nummutualIncrease = 0;
int callAmountMake_Mutual = 0;
int increaseForNumMutual = 0;
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
    //sched->iowait = pq_constructor(2);
    //sched->threads = pq_constructor(4);
    sched->iowait1 = que_constructor();
    sched->iowait2 = que_constructor();
    sched->deadq = que_constructor();
    sched->numreg = 0;
    sched->numbusy = 0;
    sched->numpair = 0;
    sched->nummutual = 0;
    return sched;
}

int * create_list (int min, int max, int * list) {
  /* Create a unique set of NUMTRAPS integers
   * based on the range given. */
  int last, i, next, k;
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

pcb_ptr make_regular (int pid, long rawTime, int pri,
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
pcb_ptr make_busy (int pid, long rawTime, int pri,
  int mpc, int t2) {
  pcb_ptr this = pcb_constructor();
  pcb_initialize(this, pid, pri, ready, busy,
    mpc, rawTime, t2);
  return this;
}
pcb_ptr make_producer (int pid, long rawTime, int pri,
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
pcb_ptr make_consumer (pcb_ptr prod, int pid, long rawTime,
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
pcb_ptr make_mutual (int pid, long rawTime, int pri,
  int mpc, int t2, int nummutual) {
  //Mutual should be using the same mtx list but
  //it locks two mutex instead of one
  pcb_ptr this = pcb_constructor();
  int io1[NUMTRAPS] = {10, 60, 100, 200};
  int io2[NUMTRAPS] = {50, 70, 150, 300};
  //each mtx lock it locks 0 and 1
  int mtx[NUMTRAPS] = {35, 45, 155, 165};
  
  int mtxlock[NUMTRAPS] = {nummutual * 2, nummutual * 2 + 1,
    nummutual * 2, nummutual * 2 + 1};
  //mutex list
  pcb_initialize(this, pid, pri, ready, mutual, mpc,
    rawTime, t2);
  pcb_set_io1(this, io1);
  pcb_set_io2(this, io2);
  pcb_set_mtx(this, mtx);
  pcb_set_mtxlock(this, mtxlock);
  this->pairnumber = nummutual;
  callAmountMake_Mutual = callAmountMake_Mutual + 1;
  return this;
}
//Condition variables are created when a thread is blocked.
//mutexes are created when the pairs are
int make_pcb(sch_ptr this, cpu_ptr that, int pid) {
  /* Determine the priority and type then
   * call the appropriate making method.*/
  int pri = -1;
  int prob = random1(1, 100);
  if (prob <= 20) pri = 0;
  else if (prob <= 85) pri = 1;
  else if (prob <= 95) pri = 2;
  else pri = 3;
  int mpc = (int) random1(MAXTIME * 3, MAXTIME * 4);
  int t2 = random1(2, 15);
  //types: regular, busy, producer, consumer, mutual resource
  prob = random1(1, 100);
  if (pri == 0 && this->numbusy < MAXBUSY) {
    q_enqueue(this->enq, make_busy(pid, that->totaltime,
      pri, mpc, t2));
    pid = pid + 1;
    this->numbusy = this->numbusy + 1;
  } else if (prob <= 20 && this->numpair < MAXPAIR) {
      int pairnumber = sch_pairnumber(this);
      pcb_ptr prod = make_producer(pid, that->totaltime,
        pri, mpc, t2, pairnumber);
      pid = pid + 1;
      
      pcb_ptr cons = make_consumer(prod, pid,
        that->totaltime, pri, mpc, prod->terminate);
      pid = pid + 1;
      
      q_enqueue(this->enq, prod);
      q_enqueue(this->enq, cons);
      mutex_ptr m = mutex_constructor(pairnumber);
      this->p_mtx[pairnumber] = m;
      this->p_pairs[pairnumber] = 1;
      this->numpair = this->numpair + 1;
  } /*else if (prob <= 40 && this->nummutual < MAXMUTUAL) {
      int pairnumber = sch_mutualnumber(this);
      pcb_ptr m1 = make_mutual(pid, that->totaltime,
        pri, mpc, t2, pairnumber);
      pid = pid + 1;
      
      pcb_ptr m2 = make_mutual(pid, that->totaltime,
        pri, mpc, m1->terminate, pairnumber);
      pid = pid + 1;
      
      q_enqueue(this->enq, m1);
      q_enqueue(this->enq, m2);
      mutex_ptr mtx1 = mutex_constructor(m1->mtx_lockon[0]);
      mutex_ptr mtx2 = mutex_constructor(m1->mtx_lockon[1]);
      this->m_mtx[mtx1->mutex_name] = mtx1;
      this->m_mtx[mtx2->mutex_name] = mtx2;
      this->m_pairs[pairnumber] = 1;
      this->nummutual = this->nummutual + 1;
  }*/ else if (this->numreg < MAXREG) {
    q_enqueue(this->enq, make_regular(pid, that->totaltime, pri, mpc, t2));
    pid = pid + 1;
    this->numreg = this->numreg + 1;
  }
  return pid;
}
int sch_enqueue(sch_ptr this, cpu_ptr that, int pid) {
    /*Initialize some PCBs to be run.*/
    int i = random1(1, 5);
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
pcb_ptr sch_init(sch_ptr this, cpu_ptr that, int * pid) {
    time_t seed; //this should be in main
    seed = time(NULL);
    srand((int)seed);
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
    
    printf("To %s\r\n", q_toString(to));
    printf("From: %s\r\n", q_toString(from));
    
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
    //printf("%s\r\n", pq_toString(this->rdyq));
    printf("Switching...%d : %d : %d\r\n",
      current->state, current->type, current->pairnumber);
    printf("PID: %d, PC: %d\r\n", current->pid, current->pc);
    //printf("PCB: %s\r\n", pcb_toString(current));
    int pri = pcb_get_priority(current);
    que_ptr from = pq_minpri(this->rdyq);
    que_ptr to = NULL;
    if (pri >= 0) to = this->rdyq->priorityQue[pri];
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
        //printf("Waiting: %d\r\n", this->iowait1->node_count);
        //printf("PCB: %s\r\n", pcb_toString(q_peek(this->iowait1)));
        next = q_dequeue(this->iowait1);
        //printf("PCB: %s\r\n", pcb_toString(next));
        next->state = ready;
        pq_enqueue(this->rdyq, next);
        current->state = running;
        next = current;
        break;
        
        case ioready2:
        //printf("Waiting: %d\r\n", this->iowait2->node_count);
        //printf("PCB: %s\r\n", pcb_toString(q_peek(this->iowait2)));
        next = q_dequeue(this->iowait2);
        //printf("PCB: %s\r\n", pcb_toString(next));
        next->state = ready;
        pq_enqueue(this->rdyq, next);
        current->state = running;
        next = current;
        break;
        
        //enqueue to mutex list
        case p_blocked:
        next = dispatcher( //send the mutex list to dispatcher
          this->p_mtx[pcb_get_mtx_index(current)]->waiting_pcbs,
          from, current);
        next->state = running;
        break;
        
        case m_blocked:
        next = dispatcher( //send the mutex list to dispatcher
          this->m_mtx[pcb_get_mtx_index(current)]->waiting_pcbs,
          from, current);
        next->state = running;
        break;
        
        case prodwait:
        next = dispatcher(
          this->prod_var[pcb_get_mtx_index(current)]->waiting_thread,
          from, current);
        next-> state = running;
        break;
        
        case conswait:
        next = dispatcher(
          this->cons_var[pcb_get_mtx_index(current)]->waiting_thread,
          from, current);
        next->state = running;
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
          case producer:
          if (this->p_pairs[current->pairnumber] >= 2) {
            this->numpair = this->numpair - 1;
            this->p_pairs[current->pairnumber] = 0;
          }
          break;
          case consumer:
          if (this->p_pairs[current->pairnumber] >= 2) {
            this->numpair = this->numpair - 1;
            this->p_pairs[current->pairnumber] = 0;
          }
          break;
          case mutual:
          if (this->m_pairs[current->pairnumber] >= 2) {
            this->nummutual = this->nummutual - 1;
            this->m_pairs[current->pairnumber] = 0;
          }
          break;
        }
        next = dispatcher(this->deadq, from, current);
        next->state = running;
        //pseudostack = next->pc;
        break;
    }
    if (next) {
      pcb_set_marker(next);
      printf("PID: %d, PC: %d\r\n", next->pid, next->pc);
      if (next->pc % 903 == 0) {
        next->pc = next->pc + 1;
        pseudostack = pseudostack + 1;
      }
      printf("PID: %d, PC: %d\r\n", next->pid, next->pc);
    }
    that->pc = pseudostack;
    return next;
}

int sch_pairnumber(sch_ptr this) {
  int i = 0;
  while (this->p_pairs[i] == 1) {
    i = i + 1;
  }
  return i;
}

int sch_mutualnumber(sch_ptr this) {
  int i = 0;
  while(this->m_pairs[i] == 1) {
    i = i + 1;
  }
  return i;
}

int sch_numpcbs(sch_ptr this) {
  return this->numreg + this->numbusy + this->numpair
    + this->nummutual;
}

int sch_dumptrash(sch_ptr this) {
    /*Delete all the things in deadq.*/
    printf("Dead %s\r\n", q_toString(this->deadq));
    pcb_ptr current = q_dequeue(this->deadq);
    while (this->deadq->node_count > 0) {
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

char * sch_toString(sch_ptr this) {
  	char * str = (char *) malloc(sizeof(char) * 10000);
    char * curr = (char *) malloc(sizeof(char) * 64);
    strcat(str, "Scheduler:\r\n");
    strcat(str, pq_toString(this->rdyq));
    strcat(str, "\r\nIO1:\r\n");
    strcat(str, q_toString(this->iowait1));
    strcat(str, "\r\nIO2:\r\n");
    strcat(str, q_toString(this->iowait2));
    strcat(str, "\r\nPairs1: [");
    int i;
    for (i = 0; i < MAXPAIR-1; i = i + 1) {
      sprintf(curr, "%d, ", this->p_pairs[i]);
      strcat(str, curr);
    }
    sprintf(curr, "%d]", this->p_pairs[MAXPAIR-1]);
    strcat(str, curr);
    strcat(str, "\r\nPairs2: [");
    for (i = 0; i < MAXMUTUAL-1; i = i + 1) {
      sprintf(curr, "%d, ", this->m_pairs[i]);
      strcat(str, curr);
    }
    sprintf(curr, "%d]", this->m_pairs[MAXMUTUAL-1]);
    strcat(str, curr);
    strcat(str, "\r\nValues: ");
    sprintf(curr, "NR: %d, NB: %d, NP: %d, NM: %d",
      this->numreg, this->numbusy, this->numpair, this->nummutual);
    strcat(str, curr);
    return str;
}
