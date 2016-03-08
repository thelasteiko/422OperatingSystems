/*
 * pcb.h
 *
 *  Created on: January 5, 2016
 *      Author: Melinda Robertson, Chetayana, Jason Hall, Shewangizaw Gebremariam
 *     Version: February 4 2016
 *
 *     Header file for process memory block.
 */

#ifndef PCB
#define PCB

#define NUMTRAPS 4
#define MAXPRI 3

/*Possible states a process can be in. Default is dead.*/
enum state_type {
  //0      1          2           3    4       5
  ready, running, interrupted, wait1, wait2, ioready1,
  //6         7       8
  ioready2, blocked, dead
};
enum process_type {
  //0       1     2            3      4
  regular, busy, producer, consumer, mutual
};
/*
 * Process control block data type. Holds everything
 * needed to run the process.
 */
typedef struct pcb_type {
    //These are necessary whatever the type.
    int pid;
    int priority;
    enum state_type state;
    enum process_type type;
    unsigned int pc;
    unsigned int max_pc;
    long creation; //when it was created
    long termination; //when it is terminated
    int terminate; //how many cycles until process stops; 0 for infinity
    int termcount; //how many time max_pc is passed
    int origpri; //the original priority
    int pridown; //time until demotion
    int marker; //process has run
    int oldmarker; //last time process ran
    //These depend on the type.
    int IO_1_TRAPS[NUMTRAPS];
    int IO_2_TRAPS[NUMTRAPS];
    int mtx[NUMTRAPS];
    int mtx_lockon[NUMTRAPS]; //the 'name' of the mutexes to lock
    int mtxtime; //if the thread has a lock on something
    int index;
    char * name; //used to tell the pairs for the prducer/consumer
    int pairnumber; // index of shared variable
} pcb;
typedef pcb * pcb_ptr;
/*Creates a process control block.*/
pcb_ptr pcb_constructor();
/*Initializes the values of a pcb.*/
int pcb_initialize(pcb_ptr this, int pid, int priority,
    enum state_type state, enum process_type type,
    unsigned int max_pc, long creation, int terminate);
/*Set the priority of the current process.*/
int pcb_set_priority (pcb_ptr this);
/*Returns the priority of the current process.*/
int pcb_get_priority (pcb_ptr this);

/*Set the register values for the process.*/
int pcb_set_io1 (pcb_ptr this, int * io_1_traps);
/*Get the register values for this process.*/
int * pcb_get_io1 (pcb_ptr this);

int pcb_set_io2 (pcb_ptr this, int * io_2_traps);
int * pcb_get_io2 (pcb_ptr this);

int * pcb_get_mtx(pcb_ptr this);
int pcb_set_mtx(pcb_ptr this, int * mtx);

int * pcb_get_mtxlock(pcb_ptr this);
int pcb_set_mtxlock(pcb_ptr this, int * mtx);

int pcb_set_marker(pcb_ptr this);
int pcb_set_name(pcb_ptr this, char * name);
int pcb_get_mtx_index(pcb_ptr this);
int pcb_mtx_inter(pcb_ptr this);
int pcb_reset_pc(pcb_ptr this);
/*Deallocates the memory dedicated to the current process.*/
int pcb_destructor(pcb_ptr this);
/*Returns a pointer to a String representation of the process.*/
char * pcb_toString(pcb_ptr this);
#endif
