
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

#include "pcb.h"
#include "que.h"
#include "cpu.h"
#include "pque.h"
#include "mutex.h"

#define MAXREG 50
#define MAXBUSY 25
#define MAXPAIR 10
#define MAXMUTUAL 10

typedef struct sch_type {
    que_ptr enq;
    pque_ptr rdyq;
    que_ptr iowait1;
    que_ptr iowait2;
    que_ptr deadq;
    mutex_ptr mutexes[MAXPAIR+(MAXMUTUAL*2)]; //list of mutex that are in use
    cond_ptr prod_var[MAXPAIR]; //list of condition variables for waiting producers
    cond_ptr cons_var[MAXPAIR]; //list of condiction variables for waiting consumers
  int numreg;
  int numbusy;
  int numpair;
  int nummutual;
} sch;
typedef sch * sch_ptr;

int random1(int min, int max);
sch_ptr sch_constructor (void);
//pcb_ptr make_pcb(int pid, long rawTime);
pcb_ptr sch_init(sch_ptr this, cpu_ptr that, unsigned int * pid);
int sch_enqueue(sch_ptr this, cpu_ptr that, unsigned int pid);
int sch_ready(sch_ptr this);
pcb_ptr idle_process (void);
pcb_ptr dispatcher(que_ptr to, que_ptr from, pcb_ptr current);
pcb_ptr scheduler (sch_ptr this, cpu_ptr that, pcb_ptr current);
int sch_dumptrash(sch_ptr this);
int sch_destructor(sch_ptr this);

#endif
