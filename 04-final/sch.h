/*
 * Move the scheduler to a different class
 * for clarity. The dispatcher, scheduler and
 * queues should be here.
 */

#ifndef SCH
#define SCH

#include "pcb.h"
#include "que.h"
#include "cpu.h"
#include "pque.h"

#define MAXIOP 50

typedef struct sch_type {
    que_ptr enq;
    pque_ptr rdyq;
    que_ptr iowait1;
    que_ptr iowait2;
    que_ptr deadq;
} sch;
typedef sch * sch_ptr;

int random1(int min, int max);
sch_ptr sch_constructor (void);
pcb_ptr make_pcb(int pid, long rawTime);
pcb_ptr sch_init(sch_ptr this, cpu_ptr that, int * pid);
int sch_enqueue(sch_ptr this, cpu_ptr that, int pid);
int sch_ready(sch_ptr this);
pcb_ptr idle_process (void);
pcb_ptr dispatcher(que_ptr to, que_ptr from, pcb_ptr current);
pcb_ptr scheduler (sch_ptr this, cpu_ptr that, pcb_ptr current);
int sch_updatepri(sch_ptr this);
int sch_dumptrash(sch_ptr this);
int sch_destructor(sch_ptr this);

#endif
