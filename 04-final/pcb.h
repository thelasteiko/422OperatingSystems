/*
 * pcb.h
 *
 *  Created on: January 5, 2016
 *      Author: Melinda Robertson, Chetayana, Jason Hall, Shewangizaw Gebremariam
 *     Version: February 4 2016
 *
 *     Header file for process memory block.
 */
 
 #include <time.h>

#ifndef PCB
#define PCB

#define NUMTRAPS 4
#define MAXPRI 3
/*Possible states a process can be in. Default is dead.*/
enum state_type {
  ready, running, interrupted, wait1, wait2, ioready1, ioready2, blocked, dead
};
/*
 * Process control block data type. Holds everything
 * needed to run the process.
 */
typedef struct pcb_type {
    int pid;
    int priority;
    enum state_type state;
    unsigned int pc;
    unsigned int max_pc;
    long creation; //when it was created
    long termination; //when it is terminated
    int terminate; //how many cycles until process stops; 0 for infinity
    int termcount; //how many time max_pc is passed
    int IO_1_TRAPS[NUMTRAPS];
    int IO_2_TRAPS[NUMTRAPS];
    int origpri; //the original priority
    int pridown; //time until demotion
    int pritimeout; //the max time it can wait
} pcb;
typedef pcb * pcb_ptr;
/*Creates a process control block.*/
pcb_ptr pcb_constructor();
/*Initializes the values of a pcb.*/
int pcb_initialize(pcb_ptr this, int pid, int priority,
    enum state_type state, unsigned int pc, unsigned int max_pc,
    long creation, int terminate,
    int * IO_1_TRAPS, int * IO_2_TRAPS);
/*Set the process id.*/
int pcb_set_pid (pcb_ptr this, int pid);
/*Get the process id.*/
int pcb_get_pid (pcb_ptr this);
/*Set the priority of the current process.*/
int pcb_set_priority (pcb_ptr this, int priority);
/*Returns the priority of the current process.*/
int pcb_get_priority (pcb_ptr this);
/*Set the state of the current process.*/
int pcb_set_state (pcb_ptr this, enum state_type state);
/*Get the state of the process to run.*/
enum state_type pcb_get_state  (pcb_ptr this);
/*
 * Set the location of the next line of code to run
 * in the current process. Given as a void *
 */
int pcb_set_pc (pcb_ptr this, unsigned int pc);
/*Get the location of the next line of code for this process.*/
unsigned int pcb_get_pc (pcb_ptr this);
/*MORE FIELDS!!!!!!*/
int pcb_set_max_pc (pcb_ptr this, unsigned int max_pc);
unsigned int pcb_get_max_pc (pcb_ptr this);
int pcb_set_creation (pcb_ptr this, long creation);
long pcb_get_creation (pcb_ptr this);
int pcb_set_termination (pcb_ptr this, long termination);
long pcb_get_termination (pcb_ptr this);
int pcb_set_terminate (pcb_ptr this, int terminate);
int pcb_get_terminate (pcb_ptr this);
int pcb_set_termcount (pcb_ptr this, int termcount);
int pcb_get_termcount (pcb_ptr this);
/*Set the register values for the process.*/
int pcb_set_io1 (pcb_ptr this, int * io_1_traps);
/*Get the register values for this process.*/
int * pcb_get_io1 (pcb_ptr this);
int pcb_set_io2 (pcb_ptr this, int * io_2_traps);
int * pcb_get_io2 (pcb_ptr this);
/*Deallocates the memory dedicated to the current process.*/
int pcb_destructor(pcb_ptr this);
/*Returns a pointer to a String representation of the process.*/
char * pcb_toString(pcb_ptr this);
#endif
