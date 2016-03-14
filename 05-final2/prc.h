/*
 * prc.h
 *
 *  Created on: February 9, 2016
 *      Author: Melinda Robertson, Chetayana, Jason Hall, Shewangizaw Gebremariam
 *     Version: February 9 2016
 *
 *     Header file for process.
 */

#ifndef PRC
#define PRC

#include "list.h"
#include "pcb.h"

typedef struct prc_type {
  int pid;
  int pair;
  enum process_type type;
  int origpri;
  int max_pc;
  long creation; //when it was created
  long termination; //when it is terminated
  int terminate; //how many cycles until process stops; 0 for infinity
  int termcount; //how many times max_pc is passed
  list_ptr threads;
} prc;
typedef prc * prc_ptr;

prc_ptr prc_constructor(void);
//this is where I make the appropriate pcbs
int prc_initialize(prc_ptr this, int pid, int tid, int pri,
  long creation, enum process_type type);
void * prc_thread(prc_ptr this, int tid);
//0: keep going, 1: terminate process
//check each -> 1 ? add to termcount
//call inside ISR handle
int prc_check_term(prc_ptr this, int tid, int pc);
int prc_destructor(prc_ptr this);
char * prc_toString(prc_ptr this);

#endif