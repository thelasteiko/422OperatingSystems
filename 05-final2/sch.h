/* sch.h
 *
 *  Created on: February 21 2016
 *      Author: Melinda Robertson, Chetayana, Jason Hall, Shewangizaw Gebremariam
 *     Version: March 8 2016
 *
 *      Scheduler definition; the scheduler creates PCBs, manages
 *      dispatch events and process termination.
 */
#ifndef SCH
#define SCH

#include "cpu.h"
#include "prc.h"
#include "pque.h"

//use the current's thread pid reference to locate
//the parent process
typedef struct sch_type {
  void * cpcb; //current thread
  prc_ptr cprc; //current process
  list_ptr processes;
  que_ptr enq;
  pque_ptr rdyq; //list of threads
  que_ptr io1;
  que_ptr io2;
  list_ptr deadprc; //terminated processes
  //0: regular, 1: busy, 2: pc pairs, 3: mutual
  int numbers[mutual+1];
} sch;
typedef sch * sch_ptr;

sch_ptr sch_constructor(void);
//run this only once
int sch_init(sch_ptr this, cpu_ptr that);
//make prcs and schedule the threads
int sch_enqueue(sch_ptr this, cpu_ptr that);
int sch_ready(sch_ptr this, void * that);
//when a process is chosen the 'deadprc' needs to be searched
//for the other shoe if it's type > 1
//if found then remove from deadprc and get next pcb
int scheduler(sch_ptr this, cpu_ptr that);
//what was this for?
//STEP 4 : Check for resetting pc or termination.
int sch_check_pc(sch_ptr this, cpu_ptr that);
//manage priorities
int sch_monitor(sch_ptr this);
//int sch_dumptrash(sch_ptr this);
int sch_destructor(sch_ptr this);
//char * sch_toString(sch_ptr this);

#endif