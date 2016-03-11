/*
 * condvar.c
 *
 *
 *  Created on: Feburary 20 2016
 *      Author: Melinda Robertson, Chetayana, Jason Hall, Shewangizaw Gebremariam
 *
 */
#include "mutex.h"
#include "condvar.h"
#include "pcb.h"
#include "que.h"
#include <stdlib.h>

cond_ptr cond_constructor() {
	cond_ptr con = (cond_ptr) malloc (sizeof(cond));
	con->associated_mutex = NULL;
	con->waiting_thread = que_constructor();
	con->condition = 0;
	return con;
}

int cond_wait(cond_ptr this, mutex_ptr this2) {
	this->associated_mutex = this2;
  q_enqueue(this->waiting_thread, this2->using_pcb);
  mutex_unlock(this2, this2->using_pcb);
  this->condition = 1;
	return 1;
}

pcb_pc_ptr cond_signal(cond_ptr this) {
  pcb_pc_ptr next = NULL;
	if (this->condition == 1) {
    next = (pcb_pc_ptr) q_dequeue(this->waiting_thread);
    next->super.super.state = ready;
    mutex_lock(this->associated_mutex, next);
    this->condition = 0;
	}
	return next;
}