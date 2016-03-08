/*
 * pcb.c
 *
 *  Created on: January 5 2016
 *      Author: Melinda Robertson, Chetayana, Jason Hall, Shewangizaw Gebremariam
 *     Version: February 4 2016
 *
 *      Handles process control block functions to safely
 *      access pcb data.
 */

#include "pcb.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAXTIME 300

int error_handle(char * error, int error_code, int critical) {
    printf("Error %d: %s\n", error_code, error);
    return error_code;
}
int min(int a, int b) {
  if (a <= b) return a;
  else return b;
}
int pcb_set_original(pcb_ptr this, int priority) {
  this->origpri = priority;
  this->priority = priority;
  this->pridown = (this->origpri * MAXTIME) + MAXTIME;
  return 0;
}
pcb_ptr pcb_constructor() {
    pcb_ptr p = (pcb_ptr) malloc(sizeof(pcb));
    if (!p) error_handle("PCB could not be allocated.", 3, 1);
    p->pid = -1;
    p->priority = MAXPRI;
    p->origpri = MAXPRI;
    p->state = dead;
    p->pc = 0;
    p->max_pc = 0;
    p->creation = 0;
    p->termination = 0;
    p->terminate = 0;
    p->termcount = 0;
    p->pridown = 0;
    p->marker = 0;
    p->oldmarker = -1;
    p->name = NULL;
    p->pairnumber = -1;
    p->mtxtime = 0;
    p->index = -1;
    int i;
    for(i = 0; i < NUMTRAPS; i = i+1) {
        p->IO_1_TRAPS[i] = -1;
        p->IO_2_TRAPS[i] = -1;
        p->mtx[i] = -1;
        p->mtx_lockon[i] = -1;
    }
    return p;
}
int pcb_initialize(pcb_ptr this, int pid, int priority,
    enum state_type state, enum process_type type,
    unsigned int max_pc, long creation, int terminate) {
        this->pid = pid;
        this->origpri = priority;
        this->state = state;
        this->type = type;
        this->max_pc = max_pc;
        this->creation = creation;
        this->terminate = terminate;
        pcb_set_original(this, priority);
        return 0;
}

int pcb_set_priority (pcb_ptr this) {
  if (this->marker < this->oldmarker) {
    if (this->pridown == 0) {
      if (this->priority >= 0)
        this->priority = this->priority - 1;
    } else
      this->pridown = this->pridown - 1;
  } else {
    this->priority = this->origpri;
    this->oldmarker = this->marker;
  }
  //this->pridown = (this->origpri * MAXTIME) + MAXTIME;
  return 0;
}
int pcb_get_priority (pcb_ptr this) {
    return this->priority;
}

int pcb_set_io1 (pcb_ptr this, int * io_1_traps) {
	int i;
	for (i = 0; i < NUMTRAPS; i++) {
		this->IO_1_TRAPS[i] = io_1_traps[i];
	}
    return 0;
}
int * pcb_get_io1 (pcb_ptr this) {
    int * traps = (int*) malloc(sizeof(int)*NUMTRAPS);
    int i = 0;
    for (; i < NUMTRAPS; i++) {
        traps[i] = this->IO_1_TRAPS[i];
    }
    return traps;
}

int pcb_set_io2 (pcb_ptr this, int * io_2_traps) {
	int i;
	for (i = 0; i < NUMTRAPS; i++) {
		this->IO_2_TRAPS[i] = io_2_traps[i];
	}
    return 0;
}
int * pcb_get_io2 (pcb_ptr this) {
    int * traps = (int*) malloc(sizeof(int)*NUMTRAPS);
    int i = 0;
    for (; i < NUMTRAPS; i++) {
        traps[i] = this->IO_2_TRAPS[i];
    }
    return traps;
}

int * pcb_get_mtx (pcb_ptr this) {
    int * traps = (int*) malloc(sizeof(int)*NUMTRAPS);
    int i = 0;
    for (; i < NUMTRAPS; i++) {
        traps[i] = this->mtx[i];
    }
    return traps;
}
int pcb_set_mtx (pcb_ptr this, int * mtx) {
	int i;
	for (i = 0; i < NUMTRAPS; i++) {
		this->mtx[i] = mtx[i];
	}
    return 0;
}

int * pcb_get_mtxlock (pcb_ptr this) {
    int * traps = (int*) malloc(sizeof(int)*NUMTRAPS);
    int i = 0;
    for (; i < NUMTRAPS; i++) {
        traps[i] = this->mtx_lockon[i];
    }
    return traps;
}
int pcb_set_mtxlock (pcb_ptr this, int * mtx) {
	int i;
	for (i = 0; i < NUMTRAPS; i++) {
		this->mtx_lockon[i] = mtx[i];
	}
  this->index = 0;
  this->mtxtime = mtx[0] - 1 + (min(
    this->IO_1_TRAPS[1], this->IO_2_TRAPS[1])
    - min(this->IO_1_TRAPS[0], this->IO_2_TRAPS[0]));
  return 0;
}

int pcb_set_marker(pcb_ptr this) {
  this->marker = this->marker + 1;
  this->priority = this->origpri;
  this->pridown = 0;
  return 0;
}

int pcb_set_name(pcb_ptr this, char * name) {
  if (this->name) free(this->name);
  this->name = (char *) malloc(sizeof(char) * strlen(name));
  strcpy(this->name, name);
  return 0;
}

int pcb_get_mtx_index(pcb_ptr this) {
  if (this->index < 0) return -1;
  else return this->mtx_lockon[this->index];
}

int pcb_mtx_inter (pcb_ptr this) {
  /* Checking if the current is in mtx lock,
   * if so count down until mtx is released.*/
  if(this->index < 0 || this->index >= NUMTRAPS)
    return 0;
  if (this->mtxtime == 0) {
    this->index = this->index + 1;
    if (this->index < 3)
      this->mtxtime = this->mtx[this->index] - 1 + (min(
        this->IO_1_TRAPS[this->index+1], this->IO_2_TRAPS[this->index+1])
        - min(this->IO_1_TRAPS[this->index], this->IO_2_TRAPS[this->index]));
    else if (this->index == 3)
      this->mtxtime = this->mtx[this->index] - 1 + (this->max_pc
        - min(this->IO_1_TRAPS[this->index], this->IO_2_TRAPS[this->index]));
    return 1;
  } else
    this->mtxtime = this->mtxtime - 1;
  return 0;
}

int pcb_reset_pc(pcb_ptr this) {
  if (this->pc >= this->max_pc) {
    this->termcount = this->termcount + 1;
    if (this->index > 0) {
      this->index = 0;
      this->mtxtime = this->mtx[0] - 1 + (min(
        this->IO_1_TRAPS[1], this->IO_2_TRAPS[1])
        - min(this->IO_1_TRAPS[0], this->IO_2_TRAPS[0]));
    }
    this->pc = 0;
  }
  if (this->termcount >= this->terminate)
    return 1;
  else return 0;
}

int pcb_destructor(pcb_ptr this) {
    free (this);
    return 0;
}
char * pcb_toString(pcb_ptr this) {
    char * str;
    int pri, st, id, t2, tc;
    long cre, t1;
    unsigned int pc, mpc;
    str = (char *) malloc(sizeof(char) * 80);
    //PRI: 1, PID: 0, STATE: ready, PC: 0x00, IO1: [0,0,0,0], IO2: [0,0,0,0]
    pri = pcb_get_priority(this);
    id = this->pid;
    st = this->state;
    pc = this->pc;
    mpc = this->max_pc;
    cre = this->creation;
    t1 = this->termination;
    t2 = this->terminate;
    tc = this->termcount;
    
    sprintf(str, "PRI: %d, PID: %d, STATE: %d, PC: %d, "
            "MPC: %d, CRE: %ld, T1: %ld, T2: %d, TC: %d, "
            "IO1: [%d,%d,%d,%d], IO2: [%d, %d, %d, %d]",
        pri, id, st, pc, mpc, cre, t1, t2, tc,
        this->IO_1_TRAPS[0], this->IO_1_TRAPS[1], this->IO_1_TRAPS[2], this->IO_1_TRAPS[3],
        this->IO_2_TRAPS[0], this->IO_2_TRAPS[1], this->IO_2_TRAPS[2], this->IO_2_TRAPS[3]
        );
    return str;
}
