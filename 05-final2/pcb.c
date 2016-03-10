/*
 * pcb.c
 *
 *  Created on: February 9, 2016
 *      Author: Melinda Robertson, Chetayana, Jason Hall, Shewangizaw Gebremariam
 *     Version: February 9 2016
 *
 *     Processes, yay.
 */
 
 #include "pcb.h"
 #include "util.h"
 
pcb_base_ptr pcb_make_busy(int pid, int tid, int origpri) {
  pcb_base_ptr this = (pcb_base_ptr) malloc(sizeof(pcb_base));
  this->pid = pid;
  this->tid = tid;
  this->pc = 0;
  this->pri = 0;
  this->pridown = (origpri*MAXTIME) + MAXTIME;
  this->marker = -1;
  this->oldmarker = 0;
  return this;
}
pcb_reg_ptr pcb_make_reg(int pid, int tid, int origpri,
  int min, int mpc) {
  pcb_reg_ptr this = (pcb_reg_ptr) malloc(sizeof(pcb_reg));
  this->super = &pcb_make_busy(pid, tid, origpri);
  this->super.pri = origpri;
  this->iodevice = -1;
  this->io_1_traps = (int *) malloc(sizeof(int)*ASIZE);
  this->io_2_traps = (int *) malloc(sizeof(int)*ASIZE);
  create_list(min, mpc, this->io_1_traps);
  create_list(this->io_1_traps[ASIZE-1], mpc, this->io_2_traps);
  return this;
}
pcb_pc_ptr pcb_make_pc(int pid, int tid, int origpri, int mpc,
  enum process_type type, int pair) {
  pcb_pc_ptr this = (pcb_pc_ptr) malloc(sizeof(pcb_pc));
  //start with hard coded values
  this->mtx = -1;
  this->cv = -1;
  //this->index = 0;
  this->mtxtime = 0;
  this->mtxpc = {5, 17, 32, 54};
  if (type == pc_pair)
    this->mtxlock = {pair, pair, pair, pair};
  else if (type == mutual)
    this->mtxlock = {pair*2, pair*2+1, pair*2, pair*2+1};
  this->super = &pcb_make_reg(pid, tid, origpri, 54, mpc);
  return this;
}
//0:no trap, 1: trap IO 1, 2: trap IO 2
//set iodevice here
int pcb_trap_io(pcb_reg_ptr this, int pc) {
  int i;
  for (i = 0; i < ASIZE; i = i + 1) {
    if (this->io_1_traps[i] == pc)
      return 1;
    if (this->io_2_traps[i] == pc)
      return 2;
  }
  return 0;
}
//-1: no lock, >=0: locks at pc, return mtx, increase index
//set mtx here
int pcb_lock_mtx(pcb_pc_ptr this, int pc) {
  int i;
  for (i = 0; i < ASIZE; i = i + 1) {
    if (this->mtxpc[i] == pc) {
      this->mtx = this->mtxlock[i];
      this->mtxtime = pc + 5;
      return this->mtx;
    }
  }
  return -1;
}
//0: still has time, 1: reset time
//if 1, reset mtx
int pcb_free_mtx(pcb_pc_ptr this) {
  if (this->mtxtime == 0) {
    this->mtx = -1;
    return 1;
  }
  this->mtxtime = this->mtxtime - 1;
  return 0;
}
//changing shared variables???should be in isr???
int pcb_run(pcb_base_ptr this, enum process_type type);
//pc >= mpc ? pc = 0, return 1
//return 0
//called by prc
int pcb_reset_pc(pcb_base_ptr this, int mpc) {
  if (this->pc >= mpc) {
    this->pc = 0;
    return 1;
  }
  return 0;
}
//THIS MIGHT CAUSE PROBLEMS!
//between this and set marker, the pcb might set the marker,
//but the priority won't update until the monitor is called
int pcb_set_priority(pcb_base_ptr this, int origpri) {
  if (origpri == 0) return 0;
  if (this->marker < this->oldmarker) {
    if (this->pridown == 0) {
      if (this->priority > 0)
        this->priority = this->priority - 1;
      this->pridown = (origpri*MAXTIME) + MAXTIME;
    } else
      this->pridown = this->pridown - 1;
  } else {
    this->priority = origpri;
    this->oldmarker = this->marker;
    this->pridown = (origpri*MAXTIME) + MAXTIME;
  }
}
int pcb_set_marker(pcb_base_ptr this) {
  this->marker = this->marker + 1;
  return 0;
}