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
  this->rdyq = pque_constructor();
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
  } else if (prob <= 20 && this->numbers[pc_pairs] < MAXPAIR) {
    type = pc_pairs;
    this->numbers[pc_pairs] = this->numbers[pc_pairs] + 1;
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
  int i = my_rand(1, 5);
  prc_ptr newprc = NULL;
  while (i) {
    newprc = make_process(this, that);
    //add to prc list and add pcbs to enq
    int n;
    for (n = 0; n < newprc->threads->node_count; n = n + 1) {
      q_enqueue(enq, ls_get(newprc->threads, n));
    }
    ls_insertAt(this->processes, newprc, 0);
    i = i + 1;
  }
	while(this->enq->node_count > 0) {
    void * node = q_dequeue(this->enq);
    printf("Process has been enqueued --> PCB Contents: %s\r\n", pcb_toString(node));
    pq_enqueue(this->rdyq, node);
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
  else pq_enqueue(this->rdyq, current);
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
    tag = tag + 1
  }
  this->cpcb = next;
  if (temp->pid != -1)
    this->cprc = get_prc(this, temp);
  else this->cprc = NULL;
  return 0;
}

void * dispatch_io_wait(sch_ptr this) {
  pcb_reg_ptr current = (pcb_reg_ptr) this->cpcb;
  current->pc = pseudostack;
  if (tag == 3)
    printf("\r\nSwitching from %s\r\n", pcb_base_toString(current));
  if (current->pid == -1)
    free(current);
  else {
    if (current->iodevice == 1)
      q_enqueue(this->io1, current);
    else
      q_enqueue(this->io2, current);
  }
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
    tag = tag + 1
  }
  this->cpcb = next;
  if (temp->pid != -1)
    this->cprc = get_prc(this, temp);
  else this->cprc = NULL;
  return 0;
}

void * dispatch(que_ptr to, que_ptr from, pcb_base_ptr current) {
  /*Move current to and return from. Doesn't update process.*/
  current->pc = pseudostack;
  if (tag == 3)
    printf("\r\nSwitching from %s\r\n", pcb_base_toString(current));
  if (current->pid == -1)
    free(current);
  else q_enqueue(to, current);
  void * next = NULL;
  if (from->node_count > 0)
    next = q_dequeue(from);
  else
    next = (void *) idle_process();
  pcb_base_ptr temp = (pcb_base_ptr) next;
  pseudostack = temp->pc;
  if(tag == 3) {
    printf("Switched to %s\r\n\r\n", pcb_base_toString(temp));
    tag = 0;
  } else {
    tag = tag + 1
  }
  return next;
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
  }
  current = (pcb_base_ptr) this->cpcb;
  pcb_set_marker(current);
  that->pc = pseudostack;
  return 0;
}
