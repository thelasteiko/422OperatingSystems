/*
 * pcb.c
 *
 *  Created on: January 5 2016
 *      Author: Melinda Robertson
 *     Version: February 4 2016
 *
 *      Handles process control block functions to safely
 *      access pcb data.
 */

#include "pcb.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

int error_handle(char * error, int error_code, int critical) {
    printf("Error %d: %s\n", error_code, error);
    if (critical) exit(error_code);
    return error_code;
}
pcb_ptr pcb_constructor() {
    pcb_ptr p = (pcb_ptr) malloc(sizeof(pcb));
    if (!p) error_handle("PCB could not be allocated.", 3, 1);
    p->pid = -1;
    p->priority = MAXPRI;
    p->state = dead;
    p->pc = 0;
    p->max_pc = 0;
    time(&p->creation);
    time(&p->termination);
    p->terminate = 0;
    p->termcount = 0;
    int reg[NUMTRAPS];
    int i;
    for(i = 0; i < NUMTRAPS; i = i+1) {
        reg[i] = 0;
    }
    memcpy(p->IO_1_TRAPS, reg, NUMTRAPS);
    memcpy(p->IO_2_TRAPS, reg, NUMTRAPS);
    return p;
}
int pcb_initialize(pcb_ptr this, int pid, int priority,
    enum state_type state, address pc, address max_pc,
    time_t creation, int terminate,
    int * IO_1_TRAPS, int * IO_2_TRAPS) {
        this->pid = pid;
        this->priority = priority;
        this->state = state;
        this->pc = pc;
        this->max_pc = max_pc;
        this->creation = creation;
        this->terminate = terminate;
        pcb_set_io1(this, IO_1_TRAPS);
        pcb_set_io2(this, IO_2_TRAPS);
        return 0;
}
int pcb_set_pid (pcb_ptr this, int pid) {
    this->pid = pid;
    return 0;
}
int pcb_get_pid (pcb_ptr this) {
    return this->pid;
}
int pcb_set_priority (pcb_ptr this, int priority) {
    if (priority > MAXPRI)
        return error_handle("Priority cannot be over the max priority.", 2, 0);
    this->priority = priority;
    return 0;
}
int pcb_get_priority (pcb_ptr this) {
    return this->priority;
}
int pcb_set_state (pcb_ptr this, enum state_type state) {
    this->state = state;
    return 0;
}
enum state_type pcb_get_state (pcb_ptr this) {
    if (this->state < 0 || this->state > 4)
        return error_handle ("Invalid state.", -1, 0);
    return this->state;
}
int pcb_set_pc (pcb_ptr this, address pc) {
    this->pc = pc;
    return 0;
}
address pcb_get_pc (pcb_ptr this) {
    return this->pc;
}
int pcb_set_max_pc (pcb_ptr this, address max_pc) {
    this->max_pc = max_pc;
    return 0;
}
address pcb_get_max_pc (pcb_ptr this) {
    return this->max_pc;
}
int pcb_set_creation (pcb_ptr this, time_t creation) {
    this->creation = creation;
    return 0;
}
time_t pcb_get_creation (pcb_ptr this) {
    return this->creation;
}
int pcb_set_termination (pcb_ptr this, time_t termination) {
    this->termination = termination;
    return 0;
}
time_t pcb_get_termination (pcb_ptr this) {
    return this->termination;
}
int pcb_set_terminate (pdb_ptr this, int terminate) {
    this->terminate = terminate;
    return 0;
}
int pcb_get_terminate (pcb_ptr this) {
    return this->terminate;
}
int pcb_set_termcount (pcb_ptr this, int termcount) {
    this->termcount = termcount;
    return 0;
}
int pcb_get_termcount (pcb_ptr this) {
    return this->termcount;
}
int pcb_set_io1 (pcb_ptr this, int * io_1_traps) {
    memcpy(this->IO_1_TRAPS, io_1_traps, NUMTRAPS);
    return 0;
}
int * pcb_get_io1 (pcb_ptr this) {
    int * traps = (int *) malloc(sizeof(int) * NUMTRAPS);
    if (!reg) error_handle("Cannot get register values.", 4, 0);
    memcpy(traps, this->IO_1_TRAPS, NUMTRAPS);
    return traps;
}

int pcb_set_io2 (pcb_ptr this, int * io_2_traps) {
    memcpy(this->IO_2_TRAPS, io_2_traps, NUMTRAPS);
    return 0;
}
int * pcb_get_io2 (pcb_ptr this) {
    int * traps = (int *) malloc(sizeof(int) * NUMTRAPS);
    if (!reg) error_handle("Cannot get register values.", 4, 0);
    memcpy(traps, this->IO_2_TRAPS, NUMTRAPS);
    return traps;
}

int pcb_destructor(pcb_ptr this) {
    if (this->IO_1_TRAPS) free (this->IO_1_TRAPS);
    if (this->IO_2_TRAPS) free (this->IO_2_TRAPS);
    free (this);
    return 0;
}
char * pcb_toString(pcb_ptr this) {
    char * str, r;
    int pri, st, id, cre, t1, t2, tc;
    address pc, mpc;
    int * io1;
    int * io2;
    str = (char *) malloc(sizeof(char) * 80);
    //PRI: 1, PID: 0, STATE: ready, PC: 0x00, IO1: [0,0,0,0], IO2: [0,0,0,0]
    pri = pcb_get_priority(this);
    id = pcb_get_pid(this);
    st = pcb_get_state(this);
    pc = pcb_get_pc(this);
    mpc = pcb_get_max_pc(this);
    cre = pcb_get_creation(this);
    t1 = pcb_get_termination(this);
    t2 = pcb_get_terminate(this);
    tc = pcb_get_termcount(this);
    io1 = pcb_get_io1(this);
    io2 = pcb_get_io2(this);
    
    sprintf(str, "PRI: %d, PID: %d, STATE: %d, PC: %p, "
            "MPC: %p, CRE: %d, T1: %d, T2: %d, TC: %d, "
            "IO1: [%d,%d,%d,%d], IO2: [%d, %d, %d, %d]",
        pri, id, st, pc, mpc, cre, t1, t2, tc,
        io1[0],io1[1],io1[2],io1[3],io2[0],io2[1],io2[2],io2[3]);
    return str;
}