#include "cpu.h"
#include "sch.h"

cpu_ptr cpu_constructor () {
    cpu_ptr mycpu = (cpu_ptr) malloc(sizeof(cpu));
    mycpu->pseudostack = 0;
    mycpu->pc = 0;
}

