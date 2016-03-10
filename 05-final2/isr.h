/*
 * isr.h
 *
 *  Created on: February 9, 2016
 *      Author: Melinda Robertson, Chetayana, Jason Hall, Shewangizaw Gebremariam
 *     Version: February 9 2016
 *
 *    Instruction Set!!!!
 */
 
#ifndef ISR
#define ISR

#include "sch.h"
#include "cpu.h"

int handle_trap_io(sch_ptr this, cpu_ptr that);
int handle_lock_mtx(sch_ptr this, cpu_ptr that, int lockthis);
int handle_free_mtx(sch_ptr this, cpu_ptr that, int freethis);
int handle_run_mtx(sch_ptr this, cpu_ptr that);
int handle_timer(sch_ptr this, cpu_ptr that);
//1: prc->termcount == prc->terminate
//else check thread, if pcb->pc >= prc->maxpc, prc->termcount++
//also check deadprc for pid
//if mtx or cv is >= 0 then need to remove / release
int handle_term(sch_ptr this, cpu_ptr that);
int handle_inter_io(sch_ptr this, cpu_ptr that, int device);
//int handle_inter_io2(sch_ptr this, cpu_ptr that);

#endif