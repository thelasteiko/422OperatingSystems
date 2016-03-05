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
      this->using_pcb->state = unblocked;
		} else {
			this->using_pcb = NULL;
      this->mutex_state = 0;
		}
		//if there is noting in the waiting que then using_pcb is null if there is something
		//then using_pcb ='s the next item in the waiting que.
	}
	return this->using_pcb;
}

cond_ptr cond_constructor() {
	cond_ptr con = (cond_ptr) malloc (sizeof(cond));
	con->associated_mutex = que_constructor();
	con->waiting_threads = que_constructor();
	con->condition = 0;
	return con;
}

int cond_wait(cond_ptr this, mutex_ptr this2) {
  //mutex_unlock(this2, this2->using_pcb);
	this->associated_mutex = this2;
	this->waiting_threads = this2->using_pcb;
	return 1;
}

int cond_signal(cond_ptr this) {
	if (this->condition == 1) {
		int i;
		for( i = 0; i < this->waiting_threads->node_count; i++) {
			q_dequeue(this->waiting_threads);
			q_dequeue(this->associated_mutex);
			//put pcbs back to work
		}

	}
	int result = 1;
	return result;
}


int main1111 () {

	pcb_ptr p1 = pcb_constructor();
	pcb_ptr p2 = pcb_constructor();
	pcb_ptr p3 = pcb_constructor();
	int t1[] = {1,1,1,1};
	int t2[] = {2,2,2,2};
	pcb_initialize(p1, 1, 1,
		    running, 1, 100,
		    1, 1,
		    t1, t2);
	pcb_initialize(p2, 2, 1,
			    running, 1, 100,
			    1, 1,
			    t1, t2);
	pcb_initialize(p3, 3, 1,
			    running, 1, 100,
			    1, 1,
			    t1, t2);

	mutex_ptr mut = mutex_constructor(10);

	printf("%d\n", mutex_trylock(mut));
	mutex_lock(mut, p1);
	printf("%d\n", mutex_trylock(mut));
	mutex_lock(mut, p2);
	mutex_lock(mut, p3);



	printf("Lock Name: %d  Lock State: %d\n", mut->mutex_name, mut->mutex_state);
	printf("Holding PCB: %s\n", pcb_toString(mut->using_pcb));
	printf("Waiting PCB's: %s\n", q_toString(mut->waiting_pcbs));
	printf("Done que: %s\n\n", q_toString(mut->done_que));



	mutex_unlock(mut, p1);


	printf("Lock Name: %d  Lock State: %d\n", mut->mutex_name, mut->mutex_state);
	printf("Holding PCB: %s\n", pcb_toString(mut->using_pcb));
	printf("%s\n", q_toString(mut->waiting_pcbs));
	printf("Done que: %s\n\n", q_toString(mut->done_que));

	mutex_unlock(mut, p2);


	printf("Lock Name: %d  Lock State: %d\n", mut->mutex_name, mut->mutex_state);
	printf("Holding PCB: %s\n", pcb_toString(mut->using_pcb));
	printf("%s\n", q_toString(mut->waiting_pcbs));
	printf("Done que: %s\n\n", q_toString(mut->done_que));

	mutex_unlock(mut, p3);

	printf("Lock Name: %d  Lock State: %d\n", mut->mutex_name, mut->mutex_state);
	//printf("Holding PCB: %s\n", pcb_toString(mut->using_pcb));
	printf("%s\n", q_toString(mut->waiting_pcbs));
	printf("Done que: %s\n\n", q_toString(mut->done_que));

	return 0;

}
