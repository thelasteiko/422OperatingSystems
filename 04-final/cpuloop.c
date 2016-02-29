/* cpu.c
 *
 *  Created on: January 21 2016
 *      Author: Melinda Robertson, Chetayana, Jason Hall, Shewangizaw Gebremariam
 *     Version: February 4 2016
 *
 *      Has a loop to simulate a CPU. The main loop is in
 *      cpu_loop. The other methods are to handle ISR
 *      commands. The ISR commands will call the scheduler
 *      as necessary.
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
    /*Here's where things run.*/
    printf("Starting...\r\n");
    int run = 1000; //This is how long it runs
    unsigned int pid = random1(0, 200);
    //This is how many PCBs will be made.
    unsigned int maxpid = pid + 10;
    pcb_ptr current = sch_init(this, that, &pid);
    printf("Process created: PID %d at %d\r\n", pid, that->totaltime);
    while (run) {
        /*printf("Time: %d\r\nT1IO: %d\r\nT2IO: %d\r\n",
            that->totaltime, that->iotime1, that->iotime2);
        */
        //To keep track of termination and creation times.
        that->totaltime = that->totaltime + 1;
        //Update the priorities to prevent starvation.
        sch_updatepri(this);
        //Add PCBs if there are fewer than the max.
        if(pid < maxpid) {
           pid = sch_enqueue(this, that, pid);
           sch_ready(this);
        }
        //printf("%s\r\n", pq_toString(this->rdyq));
        //Increment the PCB.
        that->pc = that->pc + 1;
        //Check for timer interrupt.
        if(time_inter(that)) {
            printf("Timer interrupt: PID %d at %d\r\n", current->pid, that->totaltime);
            current = time_inter_handle(this, that, current);
        }
        //If the current PCB pc value is at the max
        //reset the PCB pc and increase the termination count.
        if (current->pc >= current->max_pc) {
            current->termcount = current->termcount + 1;
            current->pc = 0;
        }
        //When the termination count is at max the PCB
        //is ready to be terminated.
        if (current->termcount >= current->terminate) {
            printf("Process terminated: PID %d at %d\r\n", current->pid, that->totaltime);
            pcb_set_termination(current, that->totaltime);
            current = term_inter_handle(this, that, current);
        }
        //Check for IO completion.
        if (io_1_inter(that, this->iowait1->node_count)) {
            printf("IO 1 Complete: PID %d at %d\r\n", q_peek(this->iowait1)->pid, that->totaltime);
            current = io_inter_handle(this, that, current, ioready1);
        }
        if (io_2_inter(that, this->iowait2->node_count)) {
            printf("IO 2 Complete: PID %d at %d\r\n", q_peek(this->iowait2)->pid, that->totaltime);
            current = io_inter_handle(this, that, current, ioready2);
        }
        //Check for IO Traps.
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
        //When the number of terminated PCBs reach a
        //limit, delete the dead queue.
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



