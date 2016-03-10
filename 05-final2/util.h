/*
 * util.h
 *
 *  Created on: January 5, 2016
 *      Author: Melinda Robertson, Chetayana, Jason Hall, Shewangizaw Gebremariam
 *     Version: February 4 2016
 *
 *     Utility functions.
 */
 
#ifndef UTIL
#define UTIL
 
#define MAXTIME 300
#define ASIZE 4
#define MAXPRI 3
#define MAXREG 50
#define MAXBUSY 25
#define MAXPAIR 10
#define MAXMUTUAL 10
/*Possible states a process can be in. Default is dead.*/
//prolly need to change these
enum state_type {
  //0      1          2           3    4       5
  ready, running, interrupted, wait1, wait2, ioready1,
  //6         7           8         9         10        11
  ioready2, p_blocked, m_blocked, prodwait, conswait, dead
};
enum process_type {
  //0       1     2         3      4
  regular, busy, pc_pair, mutual
};

int random(int min, int max);
int create_list(int min, int max, int * list);

#endif