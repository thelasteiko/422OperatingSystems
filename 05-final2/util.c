/*
 * util.c
 *
 *  Created on: January 5, 2016
 *      Author: Melinda Robertson, Chetayana, Jason Hall, Shewangizaw Gebremariam
 *     Version: February 4 2016
 *
 *     Utility functions.
 */


#include <stdlib.h>
#include "util.h"

int my_rand(int min, int max) {
  return (rand() % (max-min)) + min;
}
int create_list(int min, int max, int * list) {
  int last, i, next, k;
  last = min;
  k = max / 10;
  next = k;
  for (i = 0; i < ASIZE; i = i + 1) {
    if (next > max) next = max;
    list[i] = my_rand(last, next);
    last = list[i] + 1;
    next = last + k;
  }
  return 0;
}

int get_free_pair(enum process_type type) {
  int i, k;
  if (type == pc_pair) {
    i = 0;
    k = MAXPAIR;
  } else if (type == mutual) {
    i = MAXPAIR;
    k = MAXMUTUAL + MAXPAIR;
  } else return -1;
  for (; i < k; i = i + 1) {
    if (pairs[i] == 0) {
      pairs[i] = 1;
      return i;
    }
  }
  return -1;
}