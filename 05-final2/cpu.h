/*
We have:
  PCB
  Queue
  Pri Queue
  Original Scheduler
*/

#ifndef CPU
#define CPU

#include "util.h"
#include "sch.h"

typedef struct cpu_type {
  int pc;
  int pid; //next pid that can be made
  int tid; //next tid that can be made
  int timer;
  long totaltime;
  int iotime1;
  int iotime2;
  //int mtxtime; still needs to be done by pcb
} cpu;
typedef cput * cpu_ptr;
//The CPU should check the following each loop
cpu_ptr cpu_constructor (void);
int inter_time (cpu_ptr that);
int inter_io1 (cpu_ptr that, int node_count);
int inter_io2 (cpu_ptr that, int node_count);
int inter_free_mtx (sch_ptr this);
int inter_lock_mtx (sch_ptr this);
int inter_io_trap (sch_ptr this, cpu_ptr that);
//int inter_mtx (cpu_ptr this, int next_pc);
//call this for each loop through the cpu;
int cpu_loop(sch_ptr this, cpu_ptr that);
int cpu_destructor (cpu_ptr that);

#endif