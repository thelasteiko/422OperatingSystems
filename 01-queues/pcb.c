/*
 * pcb.c
 *
 *  Created on: January 5 2016
 *      Author: Melinda Robertson
 *     Version: January 7 2016
 *
 *      Handles process control block functions to safely
 *      access pcb data.
 */

#include "pcb.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int error_handle(char * error, int error_code, int critical) {
    printf("Error %d: %s\n", error_code, error);
    if (critical) exit(error_code);
    return error_code;
}
pcb_ptr pcb_constructor() {
    pcb_ptr p = (pcb_ptr) malloc(sizeof(pcb));
    if (!p) error_handle("PCB could not be allocated.", 3, 0);
    p->pid = -1;
    p->state = dead;
    p->pc = NULL;
    p->priority = MAXPRI;
    int reg[NUMREGS];
    int i;
    for(i = 0; i < NUMREGS; i = i+1) {
        reg[i] = 0;
    }
    memcpy(p->reg_file, reg, NUMREGS);
    p->next_pcb = NULL;
    return p;
}
int pcb_initialize(pcb_ptr this, int pid, enum state_type state, address pc,
    int * reg_file, int priority) {
        if (!this) return error_handle("PCB is null.", 2, 1);
        this->pid = pid;
        this->state = state;
        this->pc = pc;
        pcb_set_registers(this, reg_file);
        this->priority = priority;
        return 0;
}
int pcb_set_pid                 (pcb_ptr this, int pid) {
    if (!this) return error_handle("PCB is null.", 2, 1);
    this->pid = pid;
    return 0;
}
int pcb_get_pid                 (pcb_ptr this) {
    if (!this) return error_handle("PCB is null.", -1, 0);
    return this->pid;
}
int pcb_set_state               (pcb_ptr this, enum state_type state) {
    if (!this) return error_handle("PCB is null.", 2, 0);
    this->state = state;
    return 0;
}
enum state_type pcb_get_state   (pcb_ptr this) {
    if (!this) return error_handle("PCB is null.", -1, 0);
    if (this->state < 0 || this->state > 3)
        return error_handle ("Invalid state.", -1, 0);
    return this->state;
}
int pcb_set_pc                  (pcb_ptr this, address pc) {
    if (!this) return error_handle("PCB is null.", 2, 0);
    this->pc = pc;
    return 0;
}
address pcb_get_pc        (pcb_ptr this) {
    if (!this) error_handle("PCB is null.", 2, 0);
    return this->pc;
}
int pcb_set_registers           (pcb_ptr this, int * reg_file) {
    if (!this) return error_handle("PCB is null.", 2, 1);
    if (!this->reg_file) return error_handle("Register file does not exist.", 3, 1);
    int i;
    for (i = 0; i < NUMREGS; i = i + 1)  {
        this->reg_file[i] = reg_file[i];
    }
    return 0;
}
int * pcb_get_registers         (pcb_ptr this) {
    if (!this) error_handle("PCB is null.", 2, 0);
    if (!this->reg_file) error_handle("PCB registers not initialized.", 3, 0);
    int * reg = (int *) malloc(sizeof(int) * NUMREGS);
    if (!reg) error_handle("Cannot get register values.", 4, 0);
    int i;
    for (i = 0; i < NUMREGS; i = i + 1) {
        reg[i] = this->reg_file[i];
    }
    return reg;
}
int pcb_set_reg_index           (pcb_ptr this, int index, int value) {
    if (!this) return error_handle("PCB is null.", 2, 1);
    if (!this->reg_file) return error_handle("PCB registers not initialized.", 3, 1);
    this->reg_file[index] = value;
    return 0;
}
int pcb_get_reg_index           (pcb_ptr this, int index) {
    if (!this) return error_handle("PCB is null.", -1, 1);
    if (!this->reg_file) return error_handle("PCB registers not initialized.", -1, 1);
    return this->reg_file[index];
}
int pcb_set_next                (pcb_ptr this, pcb_ptr next) {
    if (!this) return error_handle("PCB is null.", 2, 1);
    if (!next) return error_handle("Next is null.", 3, 1);
    this->next_pcb = next;
    return 0;
}
pcb_ptr pcb_get_next     (pcb_ptr this) {
    if (!this) {
        error_handle("PCB is null.", 2, 1);
        return NULL;
    }
    if (!this->next_pcb) return NULL;
    return (pcb_ptr) this->next_pcb;
}
int pcb_set_priority            (pcb_ptr this, int priority) {
    if (!this) return error_handle("PCB is null.", 2, 1);
    this->priority = priority;
    return 0;
}
int pcb_get_priority            (pcb_ptr this) {
    if (!this) return error_handle("PCB is null.", MAXPRI, 1);
    return this->priority;
}
int pcb_destructor(pcb_ptr this) {
    if (!this) return error_handle("PCB is null.", 2, 1);
    if (this->reg_file) free (this->reg_file);
    free (this);
    return 0;
}
char * pcb_toString(pcb_ptr this) {
    char * str, r;
    int pri, st, id;
    address pc;
    int * reg;
    str = (char *) malloc(sizeof(char) * 80);
    //PRI: 1, PID: 0, STATE: ready, PC: 0x00, R: [0,0,0,0], NPID: 1
    pri = pcb_get_priority(this);
    id = pcb_get_pid(this);
    st = pcb_get_state(this);
    pc = pcb_get_pc(this);
    reg = pcb_get_registers(this);
    
    sprintf(str, "PRI: %d, PID: %d, STATE: %d, PC: %p, R: [%d,%d,%d,%d]",
        pri, id, st, pc, reg[0],reg[1],reg[2],reg[3]);
    return str;
}