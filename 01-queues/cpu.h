/*
* cpu.h
*
*  Created on: January 23, 2016
*      Author: Melinda Robertson
*     Version: January 23, 2016
*
*     Header file for the CPU.
*/

#include "pcb.h"

#ifndef CPU
#define CPU



cpu_ptr cpu_construcor(void);
int cpu_set_pc(cpu_ptr this, int pc);
int cpu_get_pc(cpu_ptr this);
int cpu_set_reg(cpu_ptr this, int reg_file);
int * cpu_get_reg(cpu_ptr this);
int cpu_loop(cpu_ptr this);
int cpu_destructor(cpu_ptr this);

#endif