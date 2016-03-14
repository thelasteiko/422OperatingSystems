/*
 * prc.c
 *
 *  Created on: February 9, 2016
 *      Author: Melinda Robertson, Chetayana, Jason Hall, Shewangizaw Gebremariam
 *     Version: February 9 2016
 *
 *     Process.
 */
 
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "prc.h"
#include "pcb.h"
#include "list.h"

prc_ptr prc_constructor(void) {
  prc_ptr this = (prc_ptr) malloc(sizeof(prc));
  this->termcount = 0;
  this->termination = -1;
  this->threads = ls_constructor();
  return this;
}
//this is where I make the appropriate pcbs
int prc_initialize(prc_ptr this, int pid, int tid, int pri,
  long creation, enum process_type type) {
    int pair = -1;
    int p;
    this->pair = pair;
    this->pid = pid;
    this->origpri = pri;
    this->max_pc = my_rand(MAXTIME * 3, MAXTIME * 4);
    this->creation = creation;
    this->type = type;
    this->terminate = my_rand(2, 15);
    pcb_pc_ptr temp = NULL;
    switch (type) {
      case busy:
      ls_insertAt(this->threads, 0,
        pcb_make_busy(pid, tid, pri));
      tid = tid + 1;
      break;
      case regular:
      ls_insertAt(this->threads, 0,
        pcb_make_reg(pid, tid, pri, 0, this->max_pc));
      tid = tid + 1;
      break;
      case pc_pair:
      pair = get_free_pair(pc_pair);
      if (pair >= MAXPAIR)
        printf("Error: PC pair allocated M pair space.\r\n");
      if (pair < 0) {
        printf("Error: Cannot create pair.\r\n");
        return prc_initialize(this,pid,tid,pri,creation,busy);
      }
      p = pair % MAXPAIR;
      temp = pcb_make_pc(pid, tid, pri, this->max_pc, pc_pair,p);
      temp->name = producer;
      ls_insertAt(this->threads, 0, temp);
      tid = tid + 1;
      temp = pcb_make_pc(pid, tid, pri, this->max_pc, pc_pair,p);
      temp->name = consumer;
      ls_insertAt(this->threads, 0, temp);
      tid = tid + 1;
      break;
      case mutual:
      pair = get_free_pair(mutual);
      if (pair < MAXPAIR)
        printf("Error: M pair allocated PC pair space.\r\n");
      if (pair < 0) {
        printf("Error: Cannot create pair.\r\n");
        return prc_initialize(this,pid,tid,pri,creation,busy);
      }
      p = pair % MAXPAIR;
      temp = pcb_make_pc(pid, tid, pri, this->max_pc, mutual,p);
      temp->name = mutual1;
      ls_insertAt(this->threads, 0, temp);
      tid = tid + 1;
      temp = pcb_make_pc(pid, tid, pri, this->max_pc, mutual,p);
      temp->name = mutual2;
      ls_insertAt(this->threads, 0, temp);
      tid = tid + 1;
      break;
    }
    return tid;
}

void * prc_thread(prc_ptr this, int tid) {
  pcb_base_ptr node = NULL;
  int i;
  for (i = 0; i < this->threads->node_count; i = i + 1) {
    node = (pcb_base_ptr) ls_get(this->threads, i);
    if (node && node->tid == tid)
      return (void *) node;
  }
  return (void *) node;
}

int prc_check_term(prc_ptr this, int tid, int pc) {
  int i = 0;
  pcb_base_ptr temp = (pcb_base_ptr) ls_get(this->threads, i);
  while (temp) {
    i = i + 1;
    if (pcb_reset_pc(temp, this->max_pc, pc)) {
      this->termcount = this->termcount + 1;
    }
    temp = (pcb_base_ptr) ls_get(this->threads, i);
  }
  if(this->termcount >= this->terminate)
    return 1;
  else return 0;
}

int prc_destructor(prc_ptr this) {
  ls_destructor(this->threads);
  free(this);
  return 0;
}

char * prc_toString(prc_ptr this) {
  char * str = NULL;
  char * curr = NULL;
  char * c1 = NULL;
  char * c2 = NULL;
  int i = 0;
  pcb_base_ptr temp1;
  pcb_reg_ptr temp2;
  pcb_pc_ptr temp3;
  //printf("Making string.\r\n");
  switch(this->type) {
    case regular:
    temp2 = (pcb_reg_ptr) ls_get(this->threads, i);
    //printf("Got %p\r\n", temp2);
    curr = (char *) malloc(sizeof(char)*300);
    //curr = pcb_reg_toString(temp2);
    sprintf(curr, "\r\n%s", pcb_reg_toString(temp2));
    //printf("Received %p\r\n", curr);
    break;
    case busy:
    temp1 = (pcb_base_ptr) ls_get(this->threads, i);
    curr = (char *) malloc(sizeof(char)*100);
    sprintf(curr, "\r\n%s", pcb_base_toString(temp1));
    break;
    case pc_pair:
    temp3 = (pcb_pc_ptr) ls_get(this->threads, i);
    curr = (char *) malloc(sizeof(char)*500);
    c1 = pcb_pc_toString(temp3);
    i = i + 1;
    temp3 = (pcb_pc_ptr) ls_get(this->threads, i);
    c2 = pcb_pc_toString(temp3);
    sprintf(curr, "\r\n%s\r\n%s", c1, c2);
    break;
    case mutual:
    temp3 = (pcb_pc_ptr) ls_get(this->threads, i);
    curr = (char *) malloc(sizeof(char)*500);
    c1 = pcb_pc_toString(temp3);
    i = i + 1;
    temp3 = (pcb_pc_ptr) ls_get(this->threads, i);
    c2 = pcb_pc_toString(temp3);
    sprintf(curr, "\r\n%s\r\n%s", c1, c2);
    break;
  }
  //printf("%s", curr);
  str = (char *) malloc(sizeof(char)*(strlen(curr) + 840));
  sprintf(str, "OP: %d, PID: %d, TY: %d, MPC: %d, CRE: %ld, " //33 + 17 = 50
    "TERM: %ld, CYC: %d, TCNT: %d", //21 + 13 = 34
    this->origpri, this->pid, this->type, this->max_pc, this->creation,
    this->termination, this->terminate, this->termcount
  );
  strcat(str, curr);
  return str;
}