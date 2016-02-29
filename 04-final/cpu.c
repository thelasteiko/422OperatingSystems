#include "cpu.h"
#include "sch.h"
#include <stdlib.h>

cpu_ptr cpu_constructor () {
    cpu_ptr mycpu = (cpu_ptr) malloc(sizeof(cpu));
    mycpu->pseudostack = 0;
    mycpu->pc = 0;
    mycpu->timer = MAXTIME;
    mycpu->totaltime = 0;
    mycpu->iotime1 = random1(MAXTIME * 3, MAXTIME * 4);
    mycpu->iotime2 = random1(MAXTIME * 3, MAXTIME * 4);
    return mycpu;
}

int time_inter (cpu_ptr this) {
    /*Determines if there is a timer interrupt.*/
    if (this->timer == 0) {
        this->timer = MAXTIME;
        return 1;
    }
    else this->timer = this->timer - 1;
    return 0;
}

int io_1_inter (cpu_ptr this, int node_count) {
    /*Track if there is a interrupt for
     * IO device 1.*/
    if (this->iotime1 == 0) {
        this->iotime1 = random1(MAXTIME * 2, MAXTIME * 3);
        return 1;
    }
    else if (node_count > 0)
        this->iotime1 = this->iotime1 - 1;
    return 0;
}

int io_2_inter (cpu_ptr this, int node_count) {
    /*Track if there is a interrupt for
     * IO device 2.*/
    if (this->iotime2 == 0) {
        this->iotime2 = random1(MAXTIME * 2, MAXTIME * 3);
        return 1;
    }
    else if (node_count > 0)
        this->iotime2 = this->iotime2 - 1;
    return 0;
}

int cpu_destructor(cpu_ptr this) {
    free(this);
    return 0;
}

