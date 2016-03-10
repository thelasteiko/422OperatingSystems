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
 
mutex_ptr p_mutexes[MAXPAIR];
mutex_ptr m_mutexes[MAXMUTUAL*2];
cond_var p_var[MAXPAIR];
cond_var c_var[MAXPAIR];
int p_shared[MAXPAIR];
int c_shared[MAXPAIR];
int m_shared[MAXMUTUAL*2];

int inter_time (cpu_ptr that);
int inter_io1 (cpu_ptr that, int node_count);
int inter_io2 (cpu_ptr that, int node_count);
int inter_free_mtx (sch_ptr this);
int inter_lock_mtx (sch_ptr this);
int inter_io_trap (sch_ptr this, cpu_ptr that);
//call this for each loop through the cpu;
int cpu_loop(sch_ptr this, cpu_ptr that);

//ISR -------------------------------------------------------
int handle_trap_io(sch_ptr this, cpu_ptr that, int device);
int handle_lock_mtx(sch_ptr this, cpu_ptr that, int lockthis);
int handle_free_mtx(sch_ptr this, cpu_ptr that, int freethis);
int handle_run_mtx(sch_ptr this, cpu_ptr that);
int handle_timer(sch_ptr this, cpu_ptr that);
//1: prc->termcount == prc->terminate
//else check thread, if pcb->pc >= prc->maxpc, prc->termcount++
//also check deadprc for pid
//if mtx or cv is >= 0 then need to remove / release
int handle_term(sch_ptr this, cpu_ptr that);
int handle_inter_io1(sch_ptr this, cpu_ptr that);
int handle_inter_io2(sch_ptr this, cpu_ptr that);
 
int handle_trap_io(sch_ptr this, cpu_ptr that) {
  pcb_reg_ptr current = (pcb_reg_ptr) this->cpcb;
  current->state = current->iowait;
  scheduler(this, that);
  return 0;
}
int handle_lock_mtx(sch_ptr this, cpu_ptr that) {
  enum process_type t = this->cprc->type; //check type
  mutex_ptr m = NULL;
  pcb_pc_ptr node = NULL;
  if (t == pc_pair) {
    node = (pcb_pc_ptr) this->cpcb;
    //should already has mtx registered
    if (node->mtx < 0) {
      printf("Error: Mutex P does not exist.");
      return 0;
    }
    m = p_mutexes[node->mtx];
  }
  if (t == mutual) {
    node = (pcb_pc_ptr) this->cpcb;
    if (node->mtx < 0) {
      printf("Error: Mutex M does not exist.");
      return 0;
    }
    m = m_mutexes[node->mtx];
  }
  if (!m) m = mutex_constructor();
  if (mutex_lock(m, node))
    return 0;
  else {
    //state should be set my mutex or be running if an error occured
    //node->state = blocked;
    scheduler(this, that);
  }
  return 0;
}

int handle_free_mtx(sch_ptr this, cpu_ptr that, int freethis) {
  enum process_type t = this->cprc->type; //check type
  mutex_ptr m = NULL;
  //mtx has been reset, hopefully
  pcb_pc_ptr node = NULL;
  if (t == pc_pair) {
    node = (pcb_pc_ptr) this->cpcb;
    //should already have mtx registered
    if (freethis < 0) {
      printf("Error: Mutex P does not exist.");
      return 0;
    }
    m = p_mutexes[freethis];
  }
  if (t == mutual) {
    node = (pcb_pc_ptr) this->cpcb;
    if (freethis < 0) {
      printf("Error: Mutex M does not exist.");
      return 0;
    }
    m = m_mutexes[freethis];
  }
  if (!m) {
    printf("Error: Mutex %d not created.", freethis);
    return 0;
  }
  node = mutex_unlock(m, node);
  if (node) {
    sch_ready(this, &node->super.super);
  }
  return 0;
}
int handle_run_mtx(sch_ptr this, cpu_ptr that) {
  //pcb should have a lock already
  enum process_type t = this->cprc->type; //check type
  mutex_ptr m = NULL;
  pcb_pc_ptr node = NULL;
  if (t == pc_pair) {
    node = (pcb_pc_ptr) this->cpcb;
    //should already has mtx registered
    if (node->mtx < 0) {
      printf("Error: Mutex P is not locked.");
      return 0;
    }
    m = p_mutexes[node->mtx];
    if (!m) {
      printf("Error: Mutex %d not created.", node->mtx);
      return 0;
    }
    if (!m->using_pcb == node) {
      printf("Error: Incorrect mutex %d requested.", node->mtx);
      return 0;
    }
    int n = strncmp("Producer", node->name, 8);
    cond_ptr c = NULL;
    if (n) { //consume
      if (p_shared[node->mtx] > c_shared[node->mtx]) {
        printf("%s reads in value of %d.\r\n",
          node->name, p_shared[node->mtx]);
        c_shared[node->mtx] = c_shared[node->mtx] + 1;
        c = p_var[node->mtx];
        if (c && c->waiting_thread->node_count > 0)
          sch_ready(this, cond_signal(c));
      } else {
        printf("%s unable to read value.\r\n", node->name);
        c = c_var[node->mtx];
        if (!c) c = cond_constructor();
        cond_wait(c, m);
      }
    } else { //produce...hopefully
      if (p_shared[node->mtx] <= c_shared[node->mtx]) {
        p_shared[node->mtx] = p_shared[node->mtx] + 1;
        printf("%s changes value from %d to %d.\r\n",
          node->name, c_shared[node->mtx], p_shared[node->mtx]);
        c = c_var[node_mtx];
        if (c && c->waiting_thread->node_count > 0)
          sch_ready(this, cond_signal(c));
      } else {
        printf("%s unable to change value.\r\n", node->name);
        c = p_var[node->mtx];
        if (!c) c = cond_constructor();
        cond_wait(c, m);
      }
    }
  }
  if (t == mutual) {
    node = (pcb_pc_ptr) this->cpcb;
    if (node->mtx < 0) {
      printf("Error: Mutex M does not exist.");
      return 0;
    }
    m = m_mutexes[node->mtx];
    if (!m) {
      printf("Error: Mutex %d not created.", node->mtx);
      return 0;
    }
    if (!m->using_pcb == node) {
      printf("Error: Incorrect mutex %d requested.", node->mtx);
      return 0;
    }
    int oldval = m_shared[node->mtx];
    m_shared[node->mtx] = m_shared[node->mtx] + 1;
    printf("%s changes value from %d to %d.\r\n",
      node->name, oldval, m_shared[node->mtx]);
  }
  return 0;
}

int handle_timer(sch_ptr this, cpu_ptr that) {
  this->cpcb->state = interrupted;
  scheduler(this, that);
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
    handle_trap_io(this, that);
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