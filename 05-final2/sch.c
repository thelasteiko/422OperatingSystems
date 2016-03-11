/* sch.c
 *
 *  Created on: March 10 2016
 *      Author: Melinda Robertson, Chetayana, Jason Hall, Shewangizaw Gebremariam
 *     Version: March 10 2016
 *
 *      The scheduler.
 */

#include "pcb.h"
#include "prc.h"
#include "que.h"
#include "pque.h"
#include "list.h"
#include "cpu.h"
#include "sch.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <time.h>

int tag = 0; //every % 3 == 0, print
int pseudostack = 0;

sch_ptr sch_constructor(void) {
  sch_ptr this = (sch_ptr) malloc(sizeof(sch));
  this->cpcb = NULL;
  this->cprc = NULL;
  this->processes = ls_constructor();
  this->enq = que_constructor();
  this->io1 = que_constructor();
  this->io2 = que_constructor();
  this->rdyq = pq_constructor(MAXPRI+1);
  this->deadprc = ls_constructor();
  return this;
}

prc_ptr make_process(sch_ptr this, cpu_ptr that) {
  int pri = 3;
  int prob = my_rand(1, 100);
  if (prob <= 20) pri = 0;
  else if (prob <= 85) pri = 1;
  else if (prob <= 95) pri = 2;
  prob = my_rand(1, 100);
  enum process_type type = busy;
  if (pri == 0 && this->numbers[busy] < MAXBUSY) {
    this->numbers[busy] = this->numbers[busy] + 1;
  } else if (prob <= 20 && this->numbers[pc_pair] < MAXPAIR) {
    type = pc_pair;
    this->numbers[pc_pair] = this->numbers[pc_pair] + 1;
  } else if (prob <= 40 && this->numbers[mutual] < MAXMUTUAL) {
    type = mutual;
    this->numbers[mutual] = this->numbers[mutual] + 1;
  }
  prc_ptr newprc = prc_constructor();
  that->tid = prc_initialize(newprc, that->pid, that->tid,
    pri, that->totaltime, type);
  that->pid = that->pid + 1;
  return newprc;
}

prc_ptr get_prc(sch_ptr this, int pid) {
  int i;
  prc_ptr node = NULL;
  for (i = 0; i < this->processes->node_count; i = i + 1) {
    node = ls_get(this->processes, i);
    if (node->pid == pid)
      return node;
  }
  return node;
}
//so i want to make a set of pcbs and enqueue them each turn
int sch_enqueue(sch_ptr this, cpu_ptr that) {
  int i;
  if (that->pid < MAXPID) i = my_rand(1, 5);
  else i = 0;
  prc_ptr newprc = NULL;
  printf("%d\r\n", i);
  while (i) {
    newprc = make_process(this, that);
    printf("Made Process\r\n%s\r\n", prc_toString(newprc));
    //add to prc list and add pcbs to enq
    int n;
    for (n = 0; n < newprc->threads->node_count; n = n + 1) {
      q_enqueue(this->enq, ls_get(newprc->threads, n));
    }
    ls_insertAt(this->processes, 0, newprc);
    i = i - 1;
  }
	while(this->enq->node_count > 0) {
    pcb_base_ptr node = (pcb_base_ptr) q_dequeue(this->enq);
    pq_enqueue(this->rdyq, node, node->pri);
  }
  return 0;
}

int sch_init(sch_ptr this, cpu_ptr that) {
  //create some processes and enqueue them
  sch_enqueue(this, that);
  //set the first process to run
  if (this->rdyq->node_count > 0) {
    this->cpcb = pq_dequeue(this->rdyq);
    pcb_base_ptr node = (pcb_base_ptr) this->cpcb;
    //find the corresponding prc
    this->cprc = get_prc(this, node->pid);
  } else {
    printf("Error: No PCBs ready.");
    return 1;
  }
  return 0;
}

int sch_ready(sch_ptr this, void * that) {
  pcb_base_ptr next = (pcb_base_ptr) that;
  next->state = ready;
  q_enqueue(this->enq, that);
  return 0;
}

pcb_base_ptr idle_process () {
    /*An idle process to keep the CPU busy.*/
    pcb_base_ptr idle = pcb_make_busy(-1,-1,MAXPRI);
    return idle;
}

int dispatch_ready(sch_ptr this) {
  /*Moving to and from ready queue.*/
  pcb_base_ptr current = (pcb_base_ptr) this->cpcb;
  current->pc = pseudostack;
  if (tag == 3)
    printf("\r\nSwitching from %s\r\n", pcb_base_toString(current));
  if (current->pid == -1)
    free(current);
  else pq_enqueue(this->rdyq, current, current->pri);
  void * next = NULL;
  if (this->rdyq->node_count > 0)
    next = pq_dequeue(this->rdyq);
  else
    next = (void *) idle_process();
  pcb_base_ptr temp = (pcb_base_ptr) next;
  pseudostack = temp->pc;
  if(tag == 3) {
    printf("Switched to %s\r\n\r\n", pcb_base_toString(temp));
    tag = 0;
  } else {
    tag = tag + 1;
  }
  this->cpcb = next;
  return 0;
}

void * dispatch_io_wait(sch_ptr this) {
  pcb_reg_ptr current = (pcb_reg_ptr) this->cpcb;
  current->super.pc = pseudostack;
  if (tag == 3)
    printf("\r\nSwitching from %s\r\n", pcb_reg_toString(current));
  if (current->super.pid == -1)
    free(current);
  else {
    if (current->iodevice == 1)
      q_enqueue(this->io1, current);
    else
      q_enqueue(this->io2, current);
  }
  void * next = NULL;
  next = pq_dequeue(this->rdyq);
  if (!next)
    next = (void *) idle_process();
  pcb_base_ptr temp = (pcb_base_ptr) next;
  pseudostack = temp->pc;
  if(tag == 3) {
    printf("Switched to %s\r\n\r\n", pcb_base_toString(temp));
    tag = 0;
  } else {
    tag = tag + 1;
  }
  this->cpcb = next;
  return 0;
}

void * dispatch_io_ready(sch_ptr this, int device) {
  pcb_reg_ptr current = (pcb_reg_ptr) this->cpcb;
  current->super.pc = pseudostack;
  if (tag == 3)
    printf("\r\nSwitching from %s\r\n", pcb_reg_toString(current));
  void * next = NULL;
  if (device == 1)
    next = q_dequeue(this->io1);
  else
    next = q_dequeue(this->io2);
  if (next) {
    pcb_base_ptr temp = (pcb_base_ptr) next;
    temp->state = ready;
    pq_enqueue(this->rdyq, temp, temp->pri);
    if(tag == 3) {
      printf("Enqueued %s\r\n\r\n", pcb_base_toString(temp));
      tag = 0;
    } else {
      tag = tag + 1;
    }
  }
  pseudostack = current->super.pc;
  return 0;
}

void * dispatch_blocked(sch_ptr this) {
  pcb_pc_ptr current = (pcb_pc_ptr) this->cpcb;
  current->super.super.pc = pseudostack;
  if (tag == 3)
    printf("\r\nSwitching from %s\r\n", pcb_pc_toString(current));
  void * next = NULL;
  next = pq_dequeue(this->rdyq);
  if (!next)
    next = (void *) idle_process();
  pcb_base_ptr temp = (pcb_base_ptr) next;
  pseudostack = temp->pc;
  if(tag == 3) {
    printf("Switched to %s\r\n\r\n", pcb_base_toString(temp));
    tag = 0;
  } else {
    tag = tag + 1;
  }
  this->cpcb = next;
  return 0;
}

void * dispatch_dead(sch_ptr this) {
  pcb_base_ptr current = (pcb_base_ptr) this->cpcb;
  current->pc = pseudostack;
  if (tag == 3)
    printf("\r\nSwitching from %s\r\n", pcb_base_toString(current));
  if(this->cprc) {
    this->numbers[this->cprc->type] =
      this->numbers[this->cprc->type] - 1;
    ls_insertAt(this->deadprc, 0, this->cprc);
  }
  if (current->pid == -1)
    free(current);
  void * next = NULL;
  next = pq_dequeue(this->rdyq);
  if (!next)
    next = (void *) idle_process();
  pcb_base_ptr temp = (pcb_base_ptr) next;
  pseudostack = temp->pc;
  if(tag == 3) {
    printf("Switched to %s\r\n\r\n", pcb_base_toString(temp));
    tag = 0;
  } else {
    tag = tag + 1;
  }
  this->cpcb = next;
  return 0;
}

int scheduler(sch_ptr this, cpu_ptr that) {
  pseudostack = that->pc;
  pcb_base_ptr current = (pcb_base_ptr) this->cpcb;
  enum state_type inter = current->state;
  switch(inter) {
    case ready: //enqueue
    dispatch_ready(this);
    break;
    case running:
    return 1;
    break;
    case interrupted:
    current->state = ready;
    dispatch_ready(this);
    break;
    case iowait:
    dispatch_io_wait(this);
    break;
    case ioready1:
    dispatch_io_ready(this, 1);
    break;
    case ioready2:
    dispatch_io_ready(this, 2);
    break;
    case blocked:
    dispatch_blocked(this);
    break;
    case dead:
    dispatch_dead(this);
    break;
  }
  current = (pcb_base_ptr) this->cpcb;
  current->state = running;
  pcb_set_marker(current);
  if (current->pid != -1)
    this->cprc = get_prc(this, current->pid);
  else this->cprc = NULL;
  that->pc = pseudostack;
  return 0;
}

int sch_monitor(sch_ptr this) {
  /*Go through list of processes to update.*/
  list_ptr ls = this->processes;
  int i, j, n;
  for (i = 0; i < ls->node_count; i = i + 1) {
    prc_ptr temp = ls_get(ls, i);
    for (j = 0; j < temp->threads->node_count; j = j + 1) {
      pcb_set_priority(ls_get(temp->threads, j), temp->origpri);
    }
  }
  for (i = 0; i < MAXPRI+1; i = i + 1) {
    que_ptr temp = this->rdyq->priorityQue[i];
    n = temp->node_count;
    while (n > 0) {
      pcb_base_ptr temp2 = (pcb_base_ptr) q_dequeue(temp);
      n = n - 1;
      //requeue the PCB to the proper priority level
      pq_enqueue(this->rdyq, temp2, temp2->pri);
    }
  }
  return 0;
}

int sch_destructor(sch_ptr this) {
  if (this->cpcb) free(this->cpcb);
  if (this->cprc) free(this->cprc);
  if (this->enq) q_destructor(this->enq);
  if (this->rdyq) pque_destructor(this->rdyq);
  if (this->io1) q_destructor(this->io1);
  if (this->io2) q_destructor(this->io2);
  if (this->deadprc) ls_destructor(this->deadprc);
  free(this);
  return 0;
}
