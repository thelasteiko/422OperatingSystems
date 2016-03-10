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
 
pcb_base_ptr pcb_make_busy(int pid, int tid) {
  pcb_base_ptr this = (pcb_base_ptr) malloc(sizeof(pcb_base));
  this->pid = pid;
  this->tid = tid;
  this->pc = 0;
  this->pri = 0;
  this->pridown = MAXTIME;
  this->marker = -1;
  this->oldmarker = 0;
  return this;
}
pcb_reg_ptr pcb_make_reg(int pid, int tid, int min, int mpc) {
  pcb_reg_ptr this = (pcb_reg_ptr) malloc(sizeof(pcb_reg));
  this->super = &pcb_make_busy(pid, tid);
  this->super.pri = random(1, 3);
  this->iodevice = -1;
  this->io_1_traps = (int *) malloc(sizeof(int)*ASIZE);
  this->io_2_traps = (int *) malloc(sizeof(int)*ASIZE);
  create_list(min, mpc, this->io_1_traps);
  create_list(this->io_1_traps[ASIZE-1], mpc, this->io_2_traps);
  return this;
}
pcb_pc_ptr pcb_make_pc(int pid, int tid, int mpc,
  enum process_type type, int pair) {
  pcb_pc_ptr this = (pcb_pc_ptr) malloc(sizeof(pcb_pc));
  //start with hard coded values
  this->mtx = -1;
  this->cv = -1;
  this->index = 0;
  this->mtxtime = 0;
  this->mtxpc = {5, 17, 32, 54};
  if (type == pc_pair)
    this->mtxlock = {pair, pair, pair, pair};
  else if (type == mutual)
    this->mtxlock = {pair*2, pair*2+1, pair*2, pair*2+1};
  this->super = &pcb_make_reg(pid, tid, 54, mpc);
  return this;
}

int pcb_init(pcb_base_ptr this, int pri, enum process_type type);
//0:no trap, 1: trap IO 1, 2: trap IO 2
//set iodevice here
int pcb_trap_io(pcb_reg_ptr this, int pc);
//0: no lock, >0: locks at pc, return mtx, increase index
//set mtx here
int pcb_lock_mtx(pcb_pc_ptr this, int pc);
//0: still has time, 1: reset time
//if 1, reset mtx
int pcb_free_mtx(pcb_pc_ptr this);
//changing shared variables???should be in isr???
int pcb_run(pcb_base_ptr this, enum process_type type);
//pc >= mpc ? pc = 0, return 1
//return 0
//called by prc
int pcb_reset_pc(pcb_base_ptr this, int mpc);
int pcb_set_priority(pcb_base_ptr this, int origpri);
int pcb_set_marker(pcb_base_ptr this);