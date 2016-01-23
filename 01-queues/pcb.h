/*
 * pcb.h
 *
 *  Created on: January 5, 2016
 *      Author: Melinda Robertson
 *     Version: January 8, 2016
 *
 *     Header file for process memory block.
 */

#ifndef PCB
#define PCB

#define NUMREGS 4
#define MAXPRI 15
#define address int
/*Possible states a process can be in. Default is dead.*/
enum state_type {
  ready, running, interrupted, waiting, dead
};
/*
 * Process control block data type. Holds everything
 * needed to run the process.
 */
typedef struct pcb_type {
    int pid;
    enum state_type state;
    address pc;
    int reg_file[NUMREGS];
    int priority;
} pcb;
typedef pcb * pcb_ptr;
/*Creates a process control block.*/
pcb_ptr pcb_constructor();
/*Initializes the values of a pcb.*/
int pcb_initialize(pcb_ptr this, int pid, enum state_type state, address pc,
    int * reg_file, int priority);
/*Set the process id.*/
int pcb_set_pid                 (pcb_ptr this, int pid);
/*Get the process id.*/
int pcb_get_pid                 (pcb_ptr this);
/*Set the state of the current process.*/
int pcb_set_state               (pcb_ptr this, enum state_type state);
/*Get the state of the process to run.*/
enum state_type pcb_get_state   (pcb_ptr this);
/*
 * Set the location of the next line of code to run
 * in the current process. Given as a void *
 */
int pcb_set_pc                  (pcb_ptr this, address pc);
/*Get the location of the next line of code for this process.*/
address pcb_get_pc              (pcb_ptr this);
/*Set the register values for the process.*/
int pcb_set_registers           (pcb_ptr this, int * reg_file);
/*Get the register values for this process.*/
int * pcb_get_registers         (pcb_ptr this);
/*Set the value of a particular register for a process.*/
int pcb_set_reg_index           (pcb_ptr this, int index, int value);
/*Get the value of a particular register for a process.*/
int pcb_get_reg_index           (pcb_ptr this, int index);
/*Set the next process.*/
//int pcb_set_next                (pcb_ptr this, pcb_ptr next);
/*Return the pointer to the next process.*/
//pcb_ptr pcb_get_next     (pcb_ptr this);
/*Set the priority of the current process.*/
int pcb_set_priority            (pcb_ptr this, int priority);
/*Returns the priority of the current process.*/
int pcb_get_priority            (pcb_ptr this);
/*Deallocates the memory dedicated to the current process.*/
int pcb_destructor(pcb_ptr this);
/*Returns a pointer to a String representation of the process.*/
char * pcb_toString(pcb_ptr this);
#endif