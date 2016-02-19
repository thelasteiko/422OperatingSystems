/* cpu.c
 *
 *  Created on: January 21 2016
 *      Author: Melinda Robertson, Chetayana, Jason Hall, Shewangizaw Gebremariam
 *     Version: February 4 2016
 *
 *      Has a loop to simulate a CPU.
 */

#include "pcb.h"
#include "que.h"
#include "cpu.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

pcb_ptr io_trap_handle(sch_ptr this, cpu_ptr that, pcb_ptr current, int device) {
    /*ISR for traps.*/
    if (device == 1) {
        if (that->iotime1 == 0)
            that->iotime1 = random1(MAXTIME * 3, MAXTIME * 4);
        current->state = wait1;
    }
    else if (device == 2) {
        if (that->iotime2 == 0)
            that->iotime2 = random1(MAXTIME * 3, MAXTIME * 4);
        current->state = wait2;
    }
    return scheduler(this, that, current);
}

pcb_ptr time_inter_handle(sch_ptr this, cpu_ptr that, pcb_ptr current) {
    /*ISR for time interrupt.*/
    current->state = interrupted;
    pcb_ptr next = scheduler(this, that, current);
    return next;
}

pcb_ptr term_inter_handle (sch_ptr this, cpu_ptr that, pcb_ptr current) {
    /*ISR for termination of a process.*/
    current->state = dead;
    time(&current->termination);
    return scheduler(this, that, current);
}

pcb_ptr io_inter_handle (sch_ptr this, cpu_ptr that, pcb_ptr current, enum state_type device) {
    /*ISR for IO interruption.*/
    current->state = device;
    return scheduler(sch_ptr this, cpu_ptr that, current);
}

int cpu_loop (sch_ptr this, cpu_ptr that) {
    printf("Starting...\r\n");
    int run = 10000;
    unsigned int pid = 0;
    long rawTime;
    struct timespec timeinfo;
    clock_gettime(CLOCK_REALTIME, &timeinfo);
    rawTime = timeinfo.tv_nsec;
    pid = sch_init_pcb(pid);
     printf("Process created: PID %d at %ld\r\n", pid, rawTime);
     sch_ready();
     pcb_ptr current = make_pcb(pid);
     pid = pid+1;
     pseudostack = current->pc;
     pc = pseudostack;
     
     while (run) {
        clock_gettime(CLOCK_REALTIME, &timeinfo);
        rawTime = timeinfo.tv_nsec;
         if(pid < 30) {
            pid = sch_init_pcb(pid);
            sch_ready();
         }
         pc = pc + 1;
         if(time_inter()) {
             printf("Timer interrupt: PID %d at %ld\r\n", current->pid, rawTime);
             current = time_inter_handle(current);
         }
         if (current->pc >= current->max_pc) {
             current->termcount = current->termcount + 1;
             current->pc = 0;
         }
         if (current->termcount >= current->terminate) {
             printf("Process terminated: PID %d at %ld\r\n", current->pid, rawTime);
             pcb_set_termination(current, rawTime);
             current = term_inter_handle(current);
         }
         if (io_1_inter()) {
             printf("IO 1 Complete: PID %d at %ld\r\n", q_peek(iowait1)->pid, rawTime);
             current = io_inter_handle(current, ioready1);
             //printf("%s\n", q_toString(rdyq));
         }
         if (io_2_inter()) {
             printf("IO 2 Complete: PID %d at %ld\r\n", q_peek(iowait2)->pid, rawTime);
             current = io_inter_handle(current, ioready2);
             //printf("%s\n", q_toString(rdyq));
         }
         int i;
         for (i = 0; i < NUMTRAPS; i = i + 1) {
            if (current->IO_1_TRAPS[i] == pc) {
                printf("IO 1 Trap: PID %d at %ld\r\n", current->pid, rawTime);
                current = io_trap_handle(current, 1);
                break;
            }
            if (current->IO_2_TRAPS[i] == pc) {
                printf("IO 2 Trap: PID %d at %ld\r\n", current->pid, rawTime);
                current = io_trap_handle(current, 2);
                break;
            }
         }
         //run = run - 1;
         if (deadq->node_count == 10) break;
     }
     
     printf("Ready: %s\r\n", q_toString(rdyq));
     printf("IO1: %s\r\n", q_toString(iowait1));
     printf("IO2: %s\r\n", q_toString(iowait2));
     printf("Dead: %s\r\n", q_toString(deadq));
     return 0;
}

int main (void) {
    
    iotime1 = random1(MAXTIME * 3, MAXTIME * 4);
    iotime2 = random1(MAXTIME * 3, MAXTIME * 4);
    enq = que_constructor();
    rdyq = que_constructor();
    iowait1 = que_constructor();
    iowait2 = que_constructor();
    deadq = que_constructor();
    return cpu_loop();
}



