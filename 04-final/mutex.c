/*
 * mutex.c
 *
 *
 *  Created on: Feburary 20 2016
 *      Author: Melinda Robertson, Chetayana, Jason Hall, Shewangizaw Gebremariam
 *
 */


#include "mutex.h"
#include "pcb.h"
#include "que.h"
#include <stdlib.h>
#include <stdio.h>
#include <time.h>

mutex_ptr mutex_constructor(int name) {
	mutex_ptr mut = (mutex_ptr) malloc (sizeof(mutex));
	mut->waiting_pcbs = que_constructor();
  mut->using_pcb = NULL;
	//mut->done_que = que_constructor();
	mut->mutex_state = 0;
	mut->mutex_name = name;
	return mut;
}

int mutex_lock (mutex_ptr this, pcb_ptr thispcb) {
	int result = 0;
  //If the using pcb is the one requesting lock,
  //just continue
  if (this->using_pcb == thispcb) {
    this->mutex_state = 1;
    return 0;
  }
	if(this->mutex_state == 0) {
		result = 1;
		this->using_pcb = thispcb;
		this->mutex_state = 1;
	} else {
		thispcb->state = blocked;
		q_enqueue(this->waiting_pcbs, thispcb);
	}
	return result;
}

int mutex_trylock (mutex_ptr this) {
	return this->mutex_state;
}

//remove cuurent holding pcb from using_pcb and add it to the done_que.
//
pcb_ptr mutex_unlock (mutex_ptr this, pcb_ptr thispcb) {
	if (this->using_pcb == thispcb && this->mutex_state == 1) {
		//q_enqueue(this->done_que, thispcb);
		if (this->waiting_pcbs->node_count > 0) {
			this->using_pcb = q_dequeue(this->waiting_pcbs);
      this->using_pcb->state = ready;
		} else {
			this->using_pcb = NULL;
      this->mutex_state = 0;
		}
		//if there is noting in the waiting que then using_pcb is null if there is something
		//then using_pcb ='s the next item in the waiting que.
	} else return NULL;
	return this->using_pcb;
}

cond_ptr cond_constructor() {
	cond_ptr con = (cond_ptr) malloc (sizeof(cond));
	con->associated_mutex = NULL;
	con->waiting_thread = NULL;
	con->condition = 0;
	return con;
}

int cond_wait(cond_ptr this, mutex_ptr this2) {
	this->associated_mutex = this2;
	this->waiting_thread = this2->using_pcb;
  mutex_unlock(this2, this2->using_pcb);
  this->condition = 1;
	return 1;
}

pcb_ptr cond_signal(cond_ptr this) {
  pcb_ptr next = NULL;
	if (this->condition == 1) {
		this->waiting_thread->state = ready;
    //mutex should be unlocked by caller...
    next = this->waiting_thread;
    this->condition = 0;
    this->waiting_thread = NULL;
	}
	return next;
}
