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
typedef cpu * cpu_ptr;
//The CPU should check the following each loop
//don't think i need to have the headers...
cpu_ptr cpu_constructor (void);
int cpu_destructor (cpu_ptr that);
int inter_time (cpu_ptr that);
int inter_io1 (cpu_ptr that, int node_count);
int inter_io2 (cpu_ptr that, int node_count);


#endif