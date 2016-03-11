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
#define MAXBUSY 2
#define MAXPAIR 10
#define MAXMUTUAL 10
#define MAXPID 90

int pairs[MAXPAIR+MAXMUTUAL];
/*Possible states a process can be in. Default is dead.*/
//prolly need to change these
enum state_type {
  //0      1          2           3      4       5
  ready, running, interrupted, iowait, ioready1,
  //6         7         8       9         10        11
  ioready2, blocked,  dead
};
enum process_type {
  //0       1     2         3      4
  busy, regular, pc_pair, mutual
};

enum pc_type {
  producer, consumer, mutual1, mutual2
};

int my_rand(int min, int max);
int create_list(int min, int max, int * list);
int get_free_pair(enum process_type type);

#endif