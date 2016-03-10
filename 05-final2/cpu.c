/*
 * cpu.c
 *
 *  Created on: February 9, 2016
 *      Author: Melinda Robertson, Chetayana, Jason Hall, Shewangizaw Gebremariam
 *     Version: February 9 2016
 *
 *     The CPU folks.
 */
 
#include "cpu.h"
#include "sch.h"
#include "pcb.h"
//#include "que.h"
//#include "pque.h"
#include "util.h"
#include "isr.h"
#include "prc.h"
 
 //Should I put the mutexes and shared variables here?
 
cpu_ptr cpu_constructor() {
  cpu_ptr that = (cpu_ptr) malloc(sizeof(cpu));
  that->pc = 0;
  that->pid = 0;
  that->timer = MAXTIME;
  that->totaltime = 0;
  that->iotime1 = random(MAXTIME * 3, MAXTIME * 4);
  that->iotime2 = random(MAXTIME * 3, MAXTIME * 4);
  return that;
}

int inter_time (cpu_ptr that) {
  /*Determines if there is a timer interrupt.*/
  if (this->timer == 0) {
    this->timer = MAXTIME;
    return 1;
  }
  else this->timer = this->timer - 1;
  return 0;
}

int inter_io1 (cpu_ptr that) {
  /*Track if there is a interrupt for
   * IO device 1.*/
  if (this->iotime1 == 0) {
    this->iotime1 = random1(MAXTIME * 2, MAXTIME * 3);
    return 1;
  }
  else if (node_count > 0)
    this->iotime1 = this->iotime1 - 1;
  return 0;
}

int inter_io2 (cpu_ptr that) {
  /*Track if there is a interrupt for
   * IO device 2.*/
  if (this->iotime2 == 0) {
    this->iotime2 = random1(MAXTIME * 2, MAXTIME * 3);
    return 1;
  }
  else if (node_count > 0)
    this->iotime2 = this->iotime2 - 1;
  return 0;
}

int inter_free_mtx(sch_ptr this) {
  //need the current process and it's type
  pcb_base_ptr cpcb = this->cpcb;
  prc_ptr cprc = this->cprc;
  if (cprc->type >= pc_pair) {
    pcb_pc_ptr current = cpcb;
    return (pcb_free_mtx(current));
  }
  return -1;
}

int inter_io_trap (sch_ptr this, cpu_ptr that) {
  pcb_base_ptr cpcb = this->cpcb;
  prc_ptr cprc = this->cprc;
  if (cprc->type >= regular) {
    pcb_reg_ptr current = cpcb;
    return (pcb_trap_io(current, that->pc));
  }
  return 0;
}

int inter_lock_mtx(sch_ptr this, cpu_ptr that) {
  pcb_base_ptr cpcb = this->cpcb;
  prc_ptr cprc = this->cprc;
  if (cprc->type >= pc_pair) {
    pcb_pc_ptr current = cpcb;
    return (pcb_lock_mtx(current));
  }
  return -1;
}

int cpu_loop (sch_ptr this, cpu_ptr that) {
  //STEP 1 : Create new processes.
  //1.a: How many processes are there? : < max, create new
  //      Have sch check in enqueue
  sch_enqueue(this, that);
  //STEP 2 : Check for starvation.
  if (that->totaltime % MAXTIME == 0)
    sch_monitor(this);
  //STEP 3 : Check for timer interrupt.
  if (inter_time(that))
    handle_timer(this, that);
  //STEP 4 : Check for resetting pc or termination.
  if (sch_check_pc(this, that))
    handle_term(this, that);
  //STEP 5 : Check for IO interrupt.
  //5.a : io 1
  if (inter_io1(that, this->io1->node_count))
    handle_inter_io1(this, that);
  //5.b : io 2
  if (inter_io2(that, this->io2->node_count))
    handle_inter_io2(this, that);
  //STEP 6 : Check for freeing mutex.
  //6.a : Check if timeout is reached.
  int freethis = inter_free_mtx(this);
  if (freethis >= 0)
    //6.c : if so, release the mutex and move next waiting thread to ready q
    handle_free_mtx(this, that, freethis);
  else
    //6.b : if not, run the pcb to attempt to change the shared variable
    handle_run_mtx(this, that);
  //STEP 6.5 : must increase pc before going to trap checks
  that->pc = that->pc+1;
  //STEP 7 : Check for IO trap. IO will return device or 0.
  int device = inter_io_trap(this, that);
  if (device)
    handle_inter_io(this, that, device);
  //STEP 8 : Check for mutex lock.
  //where do I keep track of mutexes?
  int lockthis = inter_lock_mtx(this, that);
  if (lockthis >= 0)
    handle_lock_mtx(this, that, lockthis);
}

int cpu_destructor (cpu_ptr that) {
  free(that);
  return 0;
}