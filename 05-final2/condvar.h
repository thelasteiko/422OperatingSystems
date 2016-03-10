/*
 * condvar.h
 *
 *
 *  Created on: Feburary 20 2016
 *      Author: Melinda Robertson, Chetayana, Jason Hall, Shewangizaw Gebremariam
 *
 */
 
#ifndef CVAR
#define CVAR

#include "mutex.h"
 
typedef struct cond_type {
	que_ptr waiting_thread;
	mutex_ptr associated_mutex;
	int condition;
} cond;
typedef cond * cond_ptr;

cond_ptr cond_constructor();
//have this enqueue
int cond_wait(cond_ptr this, mutex_ptr that);
//remove from condition variable
//unlock condition if last waiting
int cond_release(cond_ptr this, pcb_base_ptr that);
pcb_pc_ptr cond_signal(cond_ptr this);
 
#endif