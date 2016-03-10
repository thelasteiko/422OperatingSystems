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

typedef struct process_type {
  int pid;
  enum process_type type;
  int origpri;
  int max_pc;
  long creation; //when it was created
  long termination; //when it is terminated
  int terminate; //how many cycles until process stops; 0 for infinity
  int termcount; //how many times max_pc is passed
  list_ptr threads;
} process;
typedef process * prc_ptr;

prc_ptr prc_constructor(void);
//this is where I make the appropriate pcbs
int prc_initialize(int pid, int max_pc, long creation,
  int terminate, enum process_type type);
enum process_type prc_type(prc_ptr this);
//ok so I need to return based on type
pcb_base_ptr prc_thread(prc_ptr this, int tid);
//int prc_addthread(prc_ptr this, void * that);
//0: keep going, 1: terminate process
//check each -> 1 ? add to termcount, type > 1 : index = 0
//call inside ISR handle
int prc_check_term(prc_ptr this, int tid);

#endif