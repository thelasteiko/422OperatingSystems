

#include "list.h"
//#include "prc.h"
#include "pcb.h"
#include <stdio.h>

ln_ptr ln_constructor(void) {
  ln_ptr this = (ln_ptr) malloc(sizeof(listnode));
  this->prev = NULL;
  this->next = NULL;
  return this;
}
int setValue(ln_ptr this, void * value) {
  this->value = value;
  return 0;
}
ln_ptr getNodeAt(list_ptr this, int index) {
  /*Finds and returns the node at the indicated index.*/
  if (index == 0) return this->first;
  if (index == this->node_count-1) return this->last;
  int i = 0;
  ln_ptr node = this->first;
  while(node && i < index) {
    node = node->next;
    i = i + 1;
  }
  return node->next;
}
int ln_destructor(ln_ptr this) {
  free(this);
  return 0;
}
list_ptr ls_constructor(void) {
  list_ptr this = (list_ptr) malloc(sizeof(list));
  this->first = NULL;
  this->last = this->first;
  this->node_count = 0;
  return this;
}
void * ls_get(list_ptr this, int index) {
  /* Gets the value at an index without removing it.*/
  if (index >= this->node_count)
    return NULL;
  ln_ptr node = getNodeAt(this, index);
  return node->value;
}

int ls_insertAt(list_ptr this, int index, void * value) {
  /*Adds a new value to the list.*/
  if (index > this->node_count) return 1;
  ln_ptr node = getNodeAt(this, index);
  if (!node) {
    this->first = ln_constructor();
    this->last = this->first;
    setValue(this->first, value);
  } else {
  //insert before the node
  ln_ptr newnode = ln_constructor();
  setValue(newnode, value);
  newnode->prev = node->prev;
  newnode->next = node;
  if (!node->prev)
    this->first = newnode;
  else
    node->prev->next = newnode;
    node->prev = newnode;
  }
  this->node_count = this->node_count + 1;
  return 0;
}
void * removeNode(list_ptr this, ln_ptr node) {
  /*Removes a particular node.*/
  if (!node->prev) {
    this->first = node->next;
  } else {
    node->prev->next = node->next;
  }
  if (!node->next) {
    this->last = node->prev;
  } else {
    node->next->prev = node->prev;
  }
  void * value = node->value;
  ln_destructor(node);
  this->node_count = this->node_count - 1;
  return value;
}
void * ls_remove(list_ptr this, int index) {
  /*Gets a value and removes it from the list.*/
  if (index >= this->node_count || index < 0)
    return NULL;
  ln_ptr node = getNodeAt(this, index);
  return removeNode(this, node);
}
void * ls_removeVal(list_ptr this, void * value) {
  return ls_remove(this, ls_contains(this, value));
}
int ls_contains(list_ptr this, void * value) {
  /*Returns the index of the value.*/
  ln_ptr temp = this->first;
  int i = -1;
  while (temp) {
    i = i + 1;
    if (temp->value == value)
      return i;
    temp = temp->next;
  }
  return i;
}
int ls_destructor(list_ptr this) {
  /*Destroys all nodes and the list.*/
  while(this->last != this->first) {
    ls_remove(this, 0);
  }
  ln_destructor(this->first);
  free(this);
  return 0;
}
/*
prc_ptr ls_containsPID(list_ptr this, int pid) {
  ln_ptr temp = this->first;
  prc_ptr temp2 = NULL;
  while (temp) {
    temp2 = (prc_ptr) temp->value;
    if (temp2->pid == pid)
      return temp2;
    temp = temp->next;
  }
  return temp2;
}
*/
pcb_base_ptr ls_containsTID(list_ptr this, int tid) {
  ln_ptr temp = this->first;
  pcb_base_ptr temp2 = NULL;
  while (temp) {
    temp2 = (pcb_base_ptr) temp->value;
    if (temp2->tid == tid)
      return temp2;
    temp = temp->next;
  }
  return temp2;
}

