/*
 * mainloop.c
 *
 *  Created on: January 21 2016
 *      Author: Melinda Robertson
 *     Version: January 21 2016
 *
 *      A loop to simulate a CPU.
 */
 
#include "pcb.h"
#include "cpu.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*Types of interrupts the scheduler can recieve.*/
enum interrupt_type {
  timer
};

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

int dispatcher() {
    
}

int scheduler(enum interrupt_type inter) {
    switch(inter) {
        case timer:
        //stuff
        break;
    }
}

 
int main(void) {
     //make a loop, runs 30 times
     int run = 30;
     //initialize queues
        //enqueueing
    que_ptr enqrdy = que_constructor();
    que_ptr readyq = que_constructor();
    que_ptr 
        //ready
        //IO
        //dead
    //types of interrupts: IO, timer, 
     while(run) {
         //create a process; PCB
         //add to enqueueing queue
         //move a process from the enqueueing queue to the ready queue
         //get a process from the ready queue
         //pseudo-run the process; ie add 3000-4000 to the PC
         //if done move to dead
         //else move to end of ready queue
         
         run = run - 1;
     }
 }