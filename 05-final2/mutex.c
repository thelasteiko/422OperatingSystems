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

mutex_ptr mutex_constructor(int name) {
  printf("Creating Mutex.\r\n");
	mutex_ptr mut = (mutex_ptr) malloc (sizeof(mutex));
	mut->waiting_pcbs = que_constructor();
  mut->using_pcb = NULL;
	mut->mutex_state = 0;
	mut->mutex_name = name;
  printf("Mutex %d created:\r\n%s\r\n", mut->mutex_name, mutex_toString(mut));
	return mut;
}

int mutex_lock (mutex_ptr this, pcb_pc_ptr thispcb) {
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
		thispcb->super.super.state = blocked;
		q_enqueue(this->waiting_pcbs, thispcb);
	}
	return result;
}

int mutex_trylock (mutex_ptr this) {
	return this->mutex_state;
}

//remove cuurent holding pcb from using_pcb and add it to the done_que.
//The mutex queue is only a halfway point between running and ready q.
//It should have the PC already saved in it.
pcb_pc_ptr mutex_unlock (mutex_ptr this, pcb_pc_ptr thispcb) {
	if (this->using_pcb == thispcb && this->mutex_state == 1) {
		//q_enqueue(this->done_que, thispcb);
		if (this->waiting_pcbs->node_count > 0) {
			this->using_pcb = q_dequeue(this->waiting_pcbs);
      this->using_pcb->super.super.state = ready;
		} else {
			this->using_pcb = NULL;
      this->mutex_state = 0;
		}
		//if there is noting in the waiting que then using_pcb is null if there is something
		//then using_pcb ='s the next item in the waiting que.
	} else return NULL;
	return this->using_pcb;
}

char * mutex_toString(mutex_ptr this) {
  char * str = (char *) malloc(sizeof(char)*1000);
  sprintf(str, "Mutex %d, ST: %d\r\n",
    this->mutex_name, this->mutex_state);
  /*strcat(str, pcb_pc_toString(this->using_pcb));
  strcat(str, "\r\n");
  strcat(str, q_toString(this->waiting_pcbs));*/
  return str;
}

