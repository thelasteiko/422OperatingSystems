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
  this->state = ready;
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
  this->super = *pcb_make_busy(pid, tid, origpri); //getting value
  this->super.pri = origpri;
  this->iodevice = -1;
  int last, i, next, k;
  int max = mpc;
  last = min;
  k = max / 10;
  next = k;
  for (i = 0; i < ASIZE; i = i + 1) {
    if (next > max) next = max;
    this->io_1_traps[i] = my_rand(last, next);
    last = this->io_1_traps[i] + 1;
    next = last + k;
    this->io_2_traps[i] = my_rand(last, next);
    last = this->io_2_traps[i] + 1;
    next = last + k;
  }
  return this;
}
pcb_pc_ptr pcb_make_pc(int pid, int tid, int origpri, int mpc,
  enum process_type type, int pair) {
    //printf("Making PC or Mutual %d\r\n", pair);
  pcb_pc_ptr this = (pcb_pc_ptr) malloc(sizeof(pcb_pc));
  //start with hard coded values
  this->mtx = -1;
  this->cv = -1;
  //this->index = 0;
  this->mtxtime = 0;
  int m1[ASIZE] = {5, 17, 32, 54};
  int i, p1 = pair*2, p2 = pair*2+1;
  for (i = 0; i < ASIZE; i = i + 1) {
    //printf("(%d,%d,%d,%d)", i, pair, p1, p2);
    this->mtxpc[i] = m1[i];
    if (type == pc_pair)
      this->mtxlock[i] = pair;
    else if (type == mutual && i%2==0)
      this->mtxlock[i] = p1;
    else if (type == mutual && i%2==1)
      this->mtxlock[i] = p2;
    //printf(" : %d\r\n", this->mtxlock[i]);
  }
  this->super = *pcb_make_reg(pid, tid, origpri, 54, mpc);
  return this;
}
//0:no trap, 1: trap IO 1, 2: trap IO 2
//set iodevice here
int pcb_trap_io(pcb_reg_ptr this, int pc) {
  int i;
  for (i = 0; i < ASIZE; i = i + 1) {
    if (this->io_1_traps[i] == pc) {
      this->iodevice = 1;
      return this->iodevice;
    }
    if (this->io_2_traps[i] == pc) {
      this->iodevice = 2;
      return this->iodevice;
    }
  }
  this->iodevice = 0;
  return this->iodevice;
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
  int temp = 0;
  if (this->mtx < 0) return 0;
  if (this->mtxtime == 0) {
    temp = this->mtx;
    this->mtx = -1;
  }
  this->mtxtime = this->mtxtime - 1;
  return temp;
}

//pc >= mpc ? pc = 0, return 1
//return 0
//called by prc
int pcb_reset_pc(pcb_base_ptr this, int mpc, int pc) {
  //get pc from cpu since it doesn't register with
  //the pcb until the pcb is changed out
  if (pc >= mpc) {
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
      if (this->pri > 0)
        this->pri = this->pri - 1;
      this->pridown = (origpri*MAXTIME) + MAXTIME;
      return 1;
    } else
      this->pridown = this->pridown - 1;
  } else {
    this->pri = origpri;
    this->oldmarker = this->marker;
    this->pridown = (origpri*MAXTIME) + MAXTIME;
  }
  return 0;
}
int pcb_set_marker(pcb_base_ptr this) {
  this->marker = this->marker + 1;
  return 0;
}

pcb_base_ptr cast_base(void * this, enum process_type from) {
  if (from == busy) {
    return (pcb_base_ptr) this;
  } else if (from == regular) {
    pcb_reg_ptr temp = (pcb_reg_ptr) this;
    return &temp->super;
  } else if (from >= pc_pair) {
    pcb_pc_ptr temp = (pcb_pc_ptr) this;
    return &temp->super.super;
  }
  return NULL;
}

pcb_reg_ptr cast_reg (void * this, enum process_type from) {
  if (from == busy) {
    printf("Error: Cannot cast busy to regular.\r\n");
    return NULL;
  } else if (from == regular) {
    return (pcb_reg_ptr) this;
  } else if (from >= pc_pair) {
    pcb_pc_ptr temp = (pcb_pc_ptr) this;
    return &temp->super;
  }
  return NULL;
}

pcb_pc_ptr cast_pc (void * this, enum process_type from) {
  if (from == busy) {
    printf("Error: Cannot cast busy to pc.\r\n");
    return NULL;
  } else if (from == regular) {
    printf("Error: Cannot cast regular to pc.\r\n");
  } else if (from >= pc_pair) {
    return (pcb_pc_ptr) this;
  }
  return NULL;
}

char * pcb_base_toString(pcb_base_ptr this) {
  //printf("Got %p\r\n", this);
  char * str = (char *) malloc(sizeof(char) * 80);
  sprintf(str, "PRI: %d, PID: %d, TID: %d, ST: %d, PC: %d, " //33
    "PRID: %d, Mkr: %d, OMkr: %d", //21
    this->pri, this->pid, this->tid, this->state,
    this->pc, this->pridown, this->marker, this->oldmarker
  );
  //printf("Returning %p\r\n", str);
  return str;
}
char * pcb_reg_toString(pcb_reg_ptr this) {
  //printf("Got %p\r\n", this);
  pcb_base_ptr that = &this->super; //getting pointer
  char * str = (char *) malloc(sizeof(char) * (80+80));
  char * curr = (char *) malloc(sizeof(char) * 80);
  //printf("Sending %p\r\n", that);
  curr = pcb_base_toString(that);
  //printf("Received %p\r\n", curr);
  strcat(str, curr);
  sprintf(curr, ", IOD: %d, IO1: [%d,%d,%d,%d], IO2: [%d,%d,%d,%d]", //32 + 24 = 56
    this->iodevice,
    this->io_1_traps[0], this->io_1_traps[1], this->io_1_traps[2], this->io_1_traps[3],
    this->io_2_traps[0], this->io_2_traps[1], this->io_2_traps[2], this->io_2_traps[3]
  );
  strcat(str, curr);
  //printf("Returning %p\r\n", str);
  return str;
}
char * pcb_pc_toString(pcb_pc_ptr this) {
  pcb_reg_ptr that = &this->super;
  char * str = (char *) malloc(sizeof(char) * (80+80+90));
  char * curr = (char *) malloc(sizeof(char) * 90);
  strcat(str, pcb_reg_toString(that));
  sprintf(curr, ", NM: %d, MTX: %d, CV: %d, MTM: %d, " //28 + 10 + 6 = 44
    "MPC: [%d,%d,%d,%d], MLC: [%d,%d,%d,%d]", //22 + 24 = 46
    this->name, this->mtx, this->cv, this->mtxtime,
    this->mtxpc[0], this->mtxpc[1], this->mtxpc[2], this->mtxpc[3],
    this->mtxlock[0], this->mtxlock[1], this->mtxlock[2], this->mtxlock[3]
  );
  //printf("MLC: [%d,%d,%d,%d]",this->mtxlock[0], this->mtxlock[1],
  //  this->mtxlock[2], this->mtxlock[3]);
  strcat(str, curr);
  return str;
}