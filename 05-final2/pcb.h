/*
 * pcb.h
 *
 *  Created on: February 9, 2016
 *      Author: Melinda Robertson, Chetayana, Jason Hall, Shewangizaw Gebremariam
 *     Version: February 9 2016
 *
 *     Header file for process memory block.
 */

#ifndef PCB
#define PCB

#include "util.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct pcb_base_type {
  enum state_type state;
  //enum process_type type; //cause it's easier
  int pid; //keep a reference to parent process
  int tid; //thread id, used for pque
  int pc;
  int pri;
  int pridown;
  int marker;
  int oldmarker;
} pcb_base;
typedef pcb_base * pcb_base_ptr;

typedef struct pcb_reg_type {
  int iodevice; //which device it's waiting on
  int io_1_traps[ASIZE];
  int io_2_traps[ASIZE];
  pcb_base super;
} pcb_reg;
typedef pcb_reg * pcb_reg_ptr;

typedef struct pcb_pc_type {
  int mtxpc[ASIZE]; //when it attempts a lock
  int mtxlock[ASIZE]; //which mutex it will lock
  enum pc_type name;
  int mtx; //which mtx it has a lock on, if any
  int cv; //if it is waiting for a signal
  //int index; //which lock is it on
  int mtxtime;  //the time until it releases lock
  pcb_reg super;
} pcb_pc;
typedef pcb_pc * pcb_pc_ptr;

typedef struct pcb_m_type {
  pcb_pc super; //access base: super.super.super
  //int index2; //2nd mutex and shared
} pcb_m;
typedef pcb_m * pcb_m_ptr;

/*parse
pcb_reg_ptr reg = pcb_make_reg(0, 0);
pcb_base_ptr pcb = reg;
pcb_init(pcb, 1, 1);
*/
//Constructors for each type of pcb
pcb_base_ptr pcb_make_busy(int pid, int tid, int origpri);
pcb_reg_ptr pcb_make_reg(int pid, int tid, int origpri, int min, int mpc);
pcb_pc_ptr pcb_make_pc(int pid, int tid, int origpri, int mpc,
  enum process_type type, int pair);
//maybe just need pc
pcb_m_ptr pcb_make_m(int pid, int tid);

//int pcb_init(pcb_base_ptr this, int pri, enum process_type type);

//0:no trap, 1: trap IO 1, 2: trap IO 2
//set iodevice here
int pcb_trap_io(pcb_reg_ptr this, int pc);
//-1: no lock, >0: locks at pc, return mtx, increase index
//set mtx here
int pcb_lock_mtx(pcb_pc_ptr this, int pc);
//0: still has time, 1: reset time
//if 1, reset mtx, also need to reset if in cond var
int pcb_free_mtx(pcb_pc_ptr this);
//changing shared variables???should be in isr???
//int pcb_run(pcb_base_ptr this, enum process_type type);
//pc >= mpc ? pc = 0, return 1
//return 0
//called by prc
int pcb_reset_pc(pcb_base_ptr this, int mpc, int pc);
int pcb_set_priority(pcb_base_ptr this, int origpri);
int pcb_set_marker(pcb_base_ptr this);
pcb_base_ptr cast_base(void * this, enum process_type from);
pcb_reg_ptr cast_reg (void * this, enum process_type from);
pcb_pc_ptr cast_pc (void * this, enum process_type from);
char * pcb_base_toString(pcb_base_ptr this);
char * pcb_reg_toString(pcb_reg_ptr this);
char * pcb_pc_toString(pcb_pc_ptr this);

#endif