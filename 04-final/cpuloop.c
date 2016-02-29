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
#include "sch.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

pcb_ptr io_trap_handle(sch_ptr this, cpu_ptr that,
    pcb_ptr current, enum state_type device) {
    /*ISR for traps.*/
    current->state = device;
    if (device == wait1) {
        if (that->iotime1 == 0)
            that->iotime1 = random1(MAXTIME * 3, MAXTIME * 4);
    }
    else if (device == wait2) {
        if (that->iotime2 == 0)
            that->iotime2 = random1(MAXTIME * 3, MAXTIME * 4);
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
    return scheduler(this, that, current);
}

int cpu_loop (sch_ptr this, cpu_ptr that) {
    printf("Starting...\r\n");
    int run = 1000;
    unsigned int pid = random1(0, 200);
    unsigned int maxpid = pid + 10;
    pcb_ptr current = sch_init(this, that, &pid);
    printf("Process created: PID %d at %d\r\n", pid, that->totaltime);
    while (run) {
//        printf("Time: %d\r\nT1IO: %d\r\nT2IO: %d\r\n",
//            that->totaltime, that->iotime1, that->iotime2);
        that->totaltime = that->totaltime + 1;
        //sch_updatepri(this);
        if(pid < maxpid) {
           pid = sch_enqueue(this, that, pid);
           sch_ready(this);
        }
        printf("%s\n", pq_toString(this->rdyq));
        that->pc = that->pc + 1;
        if(time_inter(that)) {
            printf("Timer interrupt: PID %d at %d\r\n", current->pid, that->totaltime);
            current = time_inter_handle(this, that, current);
        }
        if (current->pc >= current->max_pc) {
            current->termcount = current->termcount + 1;
            current->pc = 0;
        }
        if (current->termcount >= current->terminate) {
            printf("Process terminated: PID %d at %d\r\n", current->pid, that->totaltime);
            pcb_set_termination(current, that->totaltime);
            current = term_inter_handle(this, that, current);
        }
        if (io_1_inter(that, this->iowait1->node_count)) {
            printf("IO 1 Complete: PID %d at %d\r\n", q_peek(this->iowait1)->pid, that->totaltime);
            current = io_inter_handle(this, that, current, ioready1);
        }
        if (io_2_inter(that, this->iowait2->node_count)) {
            printf("IO 2 Complete: PID %d at %d\r\n", q_peek(this->iowait2)->pid, that->totaltime);
            current = io_inter_handle(this, that, current, ioready2);
        }
        int i;
        for (i = 0; i < NUMTRAPS; i = i + 1) {
            if (current->IO_1_TRAPS[i] == that->pc) {
                printf("IO 1 Trap: PID %d at %d\r\n", current->pid, that->totaltime);
                current = io_trap_handle(this, that, current, wait1);
                break;
            }
            if (current->IO_2_TRAPS[i] == that->pc) {
                printf("IO 2 Trap: PID %d at %d\r\n", current->pid, that->totaltime);
                current = io_trap_handle(this, that, current, wait2);
                break;
            }
        }
        if(this->deadq->node_count > 25) {
            printf("Dumping trash at %d\r\n", that->totaltime);
            printf("Dead %s", q_toString(this->deadq));
            sch_dumptrash(this);
        }
        run = run - 1;
    }

    return 0;
}

int main (void) {
    cpu_ptr that = cpu_constructor();
    sch_ptr this = sch_constructor();
    return cpu_loop(this, that);
}



