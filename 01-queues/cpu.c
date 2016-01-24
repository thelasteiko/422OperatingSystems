/*
 * mainloop.c
 *
 *  Created on: January 21 2016
 *      Author: Melinda Robertson
 *     Version: January 23 2016
 *
 *      A loop to simulate a CPU.
 */
 
#include "pcb.h"
#include "cpu.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

unsigned int pseudostack;

pcb_ptr make_pcb(int pid) {
    /*Create a randomized pcb with the pid.*/
    pcb_ptr node = pcb_constructor();
    int r = (rand() % 15);
    enum state_type rstate = ready;
    int reg[NUMREGS];
    int i;
    for(i = 0; i < NUMREGS; i++) {
        reg[i] = rand();
    }
    int pc = rand();
    pcb_initialize(node, pid, rstate, pc, reg, r);
    return node;
}

 
int cpu_loop(cpu_ptr this, sch_ptr scheduler) {
     //make a loop, runs ?? times
     int run = 30;
     int pc = 0;
        //enqueueing
        //ready
        //dead
     while(run) {
         //create 0-5 processes; add to enqueueing queue
         sch_create_pcb(scheduler);
         //move a process from the enqueueing queue to the ready queue
         sch_ready(scheduler);
         //get a process from the ready queue
         pcb_ptr current = sch_get_running(scheduler);
         //pseudo-run the process; ie add 3000-4000 to the PC
         pc = current->pc;
         pc = pc + 
         //if done move to dead
         //else move to end of ready queue
         
         run = run - 1;
     }
 }