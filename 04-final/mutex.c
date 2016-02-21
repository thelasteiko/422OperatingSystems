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
#include "cpu.h"
#include "sch.h"
#include <stdlib.h>
#include <stdio.h>
#include <time.h>

mutex_ptr mutex_constructor() {
	mutex_ptr mut = (mutex_ptr) malloc (sizeof(mutex));
	mut->mutex_state = 0;
	//The thing we're using to simulate threads
	return mut;
}

int mutex_lock (mutex_ptr this) {
	int result = 0;
	return result;
}

int mutex_trylock (mutex_ptr this) {
	return this->mutex_state;
}

int mutex_unlock (mutex_ptr this) {
	int result = 0;
	return result;
}

cond_ptr cond_constructor() {
	cond_ptr con = (cond_ptr) malloc (sizeof(cond));
	con->associated_mutex = que_constructor();
	con->waiting_threads = que_constructor();
	return cond;
}

int cond_wait(cond_ptr this, mutex_ptr this2) {
	int result = 0;
	return result;
}

int cond_signal(cond_ptr this) {
	int result = 0;
	return result;
}
