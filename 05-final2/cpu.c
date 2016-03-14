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
#include "util.h"
#include "prc.h"
#include "mutex.h"
#include "condvar.h"
#include <time.h>
 
mutex_ptr p_mutexes[MAXPAIR];
mutex_ptr m_mutexes[MAXMUTUAL*2];
cond_ptr p_var[MAXPAIR];
cond_ptr c_var[MAXPAIR];
int p_shared[MAXPAIR];
int c_shared[MAXPAIR];
int m_shared[MAXMUTUAL*2];

int handle_sch(sch_ptr this, cpu_ptr that) {
  scheduler(this, that);
  pcb_base_ptr current = (pcb_base_ptr) this->cpcb;
  //if (current->pid >= 0 && !this->cprc)
  while(!this->cprc) {
    printf("Error: No process; attempting to reschedule.\r\n");
    printf("Destroying thread %d\r\n", current->tid);
    current->state = dead;
    scheduler(this, that);
  }
  return 0;
}
 
int handle_trap_io(sch_ptr this, cpu_ptr that) {
  pcb_reg_ptr current = cast_reg(this->cpcb, this->cprc->type);
  printf("IO Trap %d by %d at %ld.\r\n", current->iodevice,
    current->super.tid, that->totaltime);
  current->super.state = iowait;
  handle_sch(this, that);
  return 0;
}
int handle_lock_mtx(sch_ptr this, cpu_ptr that) {
  enum process_type t = this->cprc->type; //check type
  mutex_ptr m = NULL;
  pcb_pc_ptr node = NULL;
  if (t >= pc_pair){
  if (t == pc_pair) {
    node = cast_pc(this->cpcb, t);
    //should already have mtx registered
    printf("Mutex lock by %d at %d for %d.\r\n",
      node->super.super.tid,that->pc,node->mtx);
    if (node->mtx < 0) {
      printf("Error: Mutex P does not exist: %d.\r\n", node->mtx);
      return 0;
    }
    m = p_mutexes[node->mtx];
    if (m == NULL){
      printf("Creating Mutex.\r\n");
      m = mutex_constructor(node->mtx);
      p_mutexes[node->mtx] = m;
    }
  }
  if (t == mutual) {
    node = cast_pc(this->cpcb, t);
    printf("Mutex lock by %d at %d for %d.\r\n",
      node->super.super.tid,that->pc,node->mtx);
    if (node->mtx < 0) {
      printf("Error: Mutex M does not exist: %d.\r\n", node->mtx);
      return 0;
    }
    m = m_mutexes[node->mtx];
    if (m == NULL){
      printf("Creating Mutex.\r\n");
      m = mutex_constructor(node->mtx);
      m_mutexes[node->mtx] = m;
    }
  }
  if(m == NULL) {
    printf("Error: Mutex was not created while locking.\r\n");
    return 1;
  }
  if (mutex_lock(m, node)) {
    printf("Mutext %d locked by %d\r\n", m->mutex_name,
      node->super.super.tid);
    return 0;
  }
  else {
    //state should be set my mutex or be running if an error occured
    //node->state = blocked;
    printf("Mutex lock was blocked, status %d. Switching %d...",
      node->super.super.state, node->super.super.tid);
    handle_sch(this, that);
    /*pcb_base_ptr current = cast_base(this->cpcb, this->cprc->type);
    printf("%d\r\n", current->tid);*/
  }
  }
  return 0;
}

int handle_free_mtx(sch_ptr this, cpu_ptr that, int freethis) {
  enum process_type t = this->cprc->type; //check type
  mutex_ptr m = NULL;
  //int freethis = -1;
  //mtx has been reset so I can't use...
  pcb_pc_ptr node = NULL;
  if(t >= pc_pair){
  if (t == pc_pair) {
    node = cast_pc(this->cpcb, t);
    //freethis = node->mtx;
    //should already have mtx registered
    if (freethis < 0) {
      //printf("Error: Mutex P does not exist.\r\n");
      //printf("PCB %s\r\n", pcb_pc_toString(node));
      return 0;
    }
    m = p_mutexes[freethis];
  }
  if (t == mutual) {
    node = cast_pc(this->cpcb, t);
    //freethis = node->mtx;
    if (freethis < 0) {
      //printf("Error: Mutex M does not exist.\r\n");
      //printf("PCB %s\r\n", pcb_pc_toString(node));
      return 0;
    }
    m = m_mutexes[freethis];
  }
  if (!m) {
    printf("Error: Mutex %d not created.\r\n", freethis);
    return 0;
  }
  if (node)
    printf("%d freeing mutex %d at %ld.\r\n",
      node->super.super.tid, freethis, that->totaltime);
  node = mutex_unlock(m, node);
  if (node) {
    sch_ready(this, (void *) node);
  }
  }
  return 0;
}
int handle_run_mtx(sch_ptr this, cpu_ptr that) {
  //pcb should have a lock already
  enum process_type t = this->cprc->type; //check type
  mutex_ptr m = NULL;
  pcb_pc_ptr node = NULL;
  if (t == pc_pair) {
    node = cast_pc(this->cpcb, t);
    //should already has mtx registered
    if (node->mtx < 0) {
      //printf("Error: Mutex P is not locked: %d.\r\n", node->mtx);
      return 0;
    }
    m = p_mutexes[node->mtx];
    if (!m) {
      printf("Error: Mutex %d not created.\r\n", node->mtx);
      return 0;
    }
    if (m->using_pcb != node) {
      printf("Mutex %d is locked by another PCB %d : %p.\r\n", node->mtx,
        node->super.super.tid, m->using_pcb);
      //printf("Error: Incorrect mutex %d requested.\r\n", node->mtx);
      return 0;
    }
    cond_ptr c = NULL;
    if (node->name == consumer) { //consume
      if (p_shared[node->mtx] > c_shared[node->mtx]) {
        printf("%d reads in value of %d.\r\n",
          node->name, p_shared[node->mtx]);
        c_shared[node->mtx] = c_shared[node->mtx] + 1;
        c = p_var[node->mtx];
        if (c && c->waiting_thread->node_count > 0)
          //sch_ready(this, cond_signal(c));
          cond_signal(c);
      } else {
        printf("%d unable to read value.\r\n", node->name);
        c = c_var[node->mtx];
        if (!c) c = cond_constructor();
        c_var[node->mtx] = c;
        cond_wait(c, m);
        //printf("St: %d\r\n", node->super.super.state);
        handle_sch(this, that);
      }
    } else if (node->name == producer){ //produce...hopefully
      if (p_shared[node->mtx] <= c_shared[node->mtx]) {
        p_shared[node->mtx] = p_shared[node->mtx] + 1;
        printf("%d changes value from %d to %d.\r\n",
          node->name, c_shared[node->mtx], p_shared[node->mtx]);
        c = c_var[node->mtx];
        if (c && c->waiting_thread->node_count > 0)
          //sch_ready(this, cond_signal(c));
          cond_signal(c);
      } else {
        printf("%d unable to change value.\r\n", node->name);
        c = p_var[node->mtx];
        if (!c) c = cond_constructor();
        p_var[node->mtx] = c;
        cond_wait(c, m);
        //printf("St: %d\r\n", node->super.super.state);
        handle_sch(this, that);
      }
    }
  }
  if (t == mutual) {
    node = cast_pc(this->cpcb, t);
    if (node->mtx < 0) {
      //printf("Error: Mutex M is not locked: %d.\r\n", node->mtx);
      return 0;
    }
    m = m_mutexes[node->mtx];
    if (!m) {
      printf("Error: Mutex %d not created.\r\n", node->mtx);
      return 0;
    }
    if (m->using_pcb != node) {
      printf("Mutex %d is locked by another PCB.\r\n", node->mtx);
      //printf("Error: Incorrect mutex %d requested.\r\n", node->mtx);
      return 0;
    }
    int oldval = m_shared[node->mtx];
    m_shared[node->mtx] = m_shared[node->mtx] + 1;
    printf("%d changes value from %d to %d.\r\n",
      node->name, oldval, m_shared[node->mtx]);
  }
  return 0;
}

int handle_timer(sch_ptr this, cpu_ptr that) {
  pcb_base_ptr current = cast_base(this->cpcb, this->cprc->type);
  printf("Timer interrupt on PID %d at %ld\r\n",
    current->tid, that->totaltime);
  current->state = interrupted;
  handle_sch(this, that);
  printf("Resuming...\r\n");
  return 0;
}

int handle_term(sch_ptr this, cpu_ptr that) {
  pcb_base_ptr current = cast_base(this->cpcb, this->cprc->type);
  current->state = dead;
  printf("Terminating %d at %ld.\r\n", current->tid, that->totaltime);
  return handle_sch(this, that);
}

int handle_inter_io1(sch_ptr this, cpu_ptr that) {
  pcb_base_ptr current = cast_base(this->cpcb, this->cprc->type);
  current->state = ioready1;
  printf("IO 1 interrupt on %d at %ld\r\n",
    current->pid, that->totaltime);
  return handle_sch(this, that);
}

int handle_inter_io2(sch_ptr this, cpu_ptr that) {
  pcb_base_ptr current = cast_base(this->cpcb, this->cprc->type);
  current->state = ioready2;
  printf("IO 1 interrupt on %d at %ld\r\n",
    current->pid, that->totaltime);
  return handle_sch(this, that);
}
 
cpu_ptr cpu_constructor() {
  cpu_ptr that = (cpu_ptr) malloc(sizeof(cpu));
  that->pc = 0;
  that->pid = 0;
  that->timer = MAXTIME;
  that->totaltime = 0;
  that->iotime1 = my_rand(MAXTIME * 3, MAXTIME * 4);
  that->iotime2 = my_rand(MAXTIME * 3, MAXTIME * 4);
  return that;
}

int inter_time (cpu_ptr that) {
  /*Determines if there is a timer interrupt.*/
  if (that->timer == 0) {
    that->timer = MAXTIME;
    return 1;
  }
  else that->timer = that->timer - 1;
  return 0;
}

int inter_io1 (cpu_ptr that, int node_count) {
  /*Track if there is a interrupt for
   * IO device 1.*/
   //printf("Checking IO 1 interrupt: %d:%d\r\n",
   // that->iotime1, node_count);
  if (that->iotime1 == 0) {
    that->iotime1 = my_rand(MAXTIME * 2, MAXTIME * 3);
    return 1;
  }
  else if (node_count > 0)
    that->iotime1 = that->iotime1 - 1;
  return 0;
}

int inter_io2 (cpu_ptr that, int node_count) {
  /*Track if there is a interrupt for
   * IO device 2.*/
   //printf("Checking IO 2 interrupt: %d:%d\r\n",
   // that->iotime2, node_count);
  if (that->iotime2 == 0) {
    that->iotime2 = my_rand(MAXTIME * 2, MAXTIME * 3);
    return 1;
  }
  else if (node_count > 0)
    that->iotime2 = that->iotime2 - 1;
  return 0;
}

int inter_free_mtx(sch_ptr this) {
  //need the current process and it's type
  prc_ptr cprc = this->cprc;
  if (cprc->type >= pc_pair) {
    pcb_pc_ptr current = cast_pc(this->cpcb, cprc->type);
    return (pcb_free_mtx(current));
  }
  return 0;
}

int inter_io_trap (sch_ptr this, cpu_ptr that) {
  prc_ptr cprc = this->cprc;
  int ret = 0;
  if (cprc->type >= regular) {
    pcb_reg_ptr current = cast_reg(this->cpcb, cprc->type);
    int ret = pcb_trap_io(current, that->pc);
    if(ret > 0)
      /*printf("IO %d Trap on PID %d at %ld\r\n",
        current->iodevice, current->super.tid, that->totaltime);*/
    return ret;
  }
  return ret;
}

int inter_lock_mtx(sch_ptr this, cpu_ptr that) {
  prc_ptr cprc = this->cprc;
  if (cprc->type >= pc_pair) {
    pcb_pc_ptr current = cast_pc(this->cpcb, cprc->type);
    return (pcb_lock_mtx(current, that->pc));
  }
  return -1;
}

int inter_term(sch_ptr this, cpu_ptr that) {
  pcb_base_ptr current = cast_base(this->cpcb, this->cprc->type);
  return prc_check_term(this->cprc, current->tid, that->pc);
}

int cpu_loop (sch_ptr this, cpu_ptr that) {
  //printf("Starting loop...\r\n");
  /*pcb_base_ptr current = (pcb_base_ptr) this->cpcb;
  printf("Loop %ld with PID %d : TID %d on PC %d.\r\n",
    that->totaltime, this->cprc->pid, current->tid, that->pc);*/
  //STEP 1 : Create new processes.
  //1.a: How many processes are there? : < max, create new
  //      Have sch check in enqueue
  sch_enqueue(this, that);
  //printf("PCBs enqueued.\r\n");
  //STEP 2 : Check for starvation.
  if (that->totaltime % MAXTIME*3 == 0) {
    printf("%s\r\n", sch_toString(this));
    sch_monitor(this);
  }
  //printf("Checking timer interrupt.\r\n");
  //STEP 3 : Check for timer interrupt.
  if (inter_time(that)) {
    handle_timer(this, that);
  }
  //printf("Process at %ld\r\n%s\r\n", that->totaltime,
  //  prc_toString(this->cprc));
  //printf("Checking termination interrupt.\r\n");
  //STEP 4 : Check for resetting pc or termination.
  if (inter_term(this,that)) {
    handle_term(this, that);
  }
  //printf("Checking IO interrupt.\r\n");
  //STEP 5 : Check for IO interrupt.
  //5.a : io 1
  if (inter_io1(that, this->io1->node_count)) {
    handle_inter_io1(this, that);
  }
  //5.b : io 2
  if (inter_io2(that, this->io2->node_count)) {
    handle_inter_io2(this, that);
  }
  //printf("Checking to free mutex.\r\n");
  //STEP 6 : Check for freeing mutex.
  //6.a : Check if timeout is reached.
  int freethis = inter_free_mtx(this);
  if (freethis >= 0) {
    //6.c : if so, release the mutex and move next waiting thread to ready q
    handle_free_mtx(this, that, freethis);
  } else {
    //6.b : if not, run the pcb to attempt to change the shared variable
    handle_run_mtx(this, that);
  }
  //STEP 6.5 : must increase pc before going to trap checks
  that->pc = that->pc+1;
  //printf("Checking for IO trap.\r\n");
  //STEP 7 : Check for IO trap. IO will return device or 0.
  int device = inter_io_trap(this, that);
  if (device) {
    handle_trap_io(this, that);
  }
  //printf("Checking for mutex lock.\r\n");
  //STEP 8 : Check for mutex lock.
  //where do I keep track of mutexes?
  int lockthis = inter_lock_mtx(this, that);
  if (lockthis >= 0) {
    handle_lock_mtx(this, that);
  }
  //printf("End of loop.\r\n");
  return 0;
}

int cpu_destructor (cpu_ptr that) {
  free(that);
  return 0;
}

int main (void) {
  printf("Starting CPU simulator...\r\n");
  sch_ptr this = sch_constructor();
  cpu_ptr that = cpu_constructor();
  time_t seed;
  seed = time(NULL);
  srand((int)seed);
  printf("Initializing scheduler.\r\n");
  if(sch_init(this, that)) {
    printf("Error: Could not initialize scheduler.");
    return 1;
  }
  printf("Starting PCB:\r\n%s\r\n", prc_toString(this->cprc));
  int run = 100000;
  while (run) {
    cpu_loop(this, that);
    that->totaltime = that->totaltime + 1;
    run = run - 1;
  }
  printf("%s\r\n", sch_toString(this));
  printf("End of CPU simulator.");
  return 0;
}