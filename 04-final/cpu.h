
#ifndef CPU
#define CPU

#define MAXTIME 300

typedef struct cpu_type {
    unsigned int pseudostack;
    unsigned int pc;
    int timer;
    int totaltime;
    int iotime1;
    int iotime2;
    int mtxtime; //set by totaltime + pcb->nexttrap-1
} cpu;
typedef cpu * cpu_ptr;

cpu_ptr cpu_constructor (void);
int time_inter (cpu_ptr this);
int io_1_inter (cpu_ptr this, int node_count);
int io_2_inter (cpu_ptr this, int node_count);
int cpu_destructor (cpu_ptr this);

#endif