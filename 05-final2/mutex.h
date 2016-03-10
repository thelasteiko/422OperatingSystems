/*
 * mutex.h
 *
 *
 *  Created on: Feburary 20 2016
 *      Author: Melinda Robertson, Chetayana, Jason Hall, Shewangizaw Gebremariam
 *
 */
 
 #ifndef MUTEX
#define MUTEX

#include "pcb.h"
#include "que.h"

typedef struct mutex_type {
	int mutex_state;
	int mutex_name;
	pcb_pc_ptr using_pcb;
	que_ptr waiting_pcbs;
} mutex;
typedef mutex * mutex_ptr;

mutex_ptr mutex_constructor(int name);
int mutex_lock (mutex_ptr this, pcb_pc_ptr thispcb);
int mutex_trylock (mutex_ptr this);
pcb_ptr mutex_unlock (mutex_ptr this, pcb_ptr thispcb);

#endif