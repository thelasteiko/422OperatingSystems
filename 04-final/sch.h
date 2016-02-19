/*
 * Move the scheduler to a different class
 * for clarity. The dispatcher, scheduler and
 * queues should be here.
 */

#ifindef SCH
#define SCH

#include "pcb.h"
#include "que.h"
#include "cpu.h"

typedef struct sch_type {
    que_ptr enq;
    que_ptr rdyq;
    que_ptr iowait1;
    que_ptr iowait2;
    que_ptr deadq;
    int timer;
    int iotimer1;
    int iotimer2;
} sch;
typedef sch * sch_ptr;

int random1(int min, int max);
sch_ptr sch_constructor (void);
int time_inter (sch_ptr this);
int io_1_inter (sch_ptr this);
int io_2_inter (sch_ptr this);
pcb_ptr make_pcb(int pid);
int sch_init_pcb(sch_ptr this, int pid);
int sch_ready(sch_ptr this);
pcb_ptr idle_process (void);
pcb_ptr dispatcher(que_ptr to, que_ptr from, pcb_ptr current);
pcb_ptr scheduler (sch_ptr this, cpu_ptr that, pcb_ptr current);

#endif
