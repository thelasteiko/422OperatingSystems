#include "list.h"

ln_ptr ln_constructor(void) {
  ln_ptr this = (ln_ptr) malloc(sizeof(listnode_type));
  this->prev = NULL;
  this->next = NULL;
  return this;
}
int setValue(ln_ptr this, void * value) {
  this->value = value;
  return 0;
}
int ln_destructor(ln_ptr this) {
  free(this);
  return 0;
}
list_ptr ls_constructor(void) {
  list_ptr this = (list_ptr) malloc(sizeof(list_type));
  this->first = NULL;
  this->last = this->first;
  this->node_count = 0;
  return this;
}
void * ls_get(list_ptr this, int index) {
  /* Gets the value at an index without removing it.*/
  if (index >= this->node_count)
    return NULL;
  if (index == this->node_count-1)
    return this->last->value;
  if (index == 0)
    return this->first->value;
  ln_ptr node = this->first;
  int i = 0;
  while (node && i < index) {
    node = node->next;
    i = i + 1;
  }
  return node->next->value;
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
int ls_insertAt(list_ptr this, int index, void * value) {
  /*Adds a new value to the list.*/
  if (index > this->node_count) return 1;
  if (index == this->node_count) {
    ln_ptr newnode = ln_constructor();
    newnode->prev = this->last;
    this->last = newnode;
  } else {
    ln_ptr node = getNodeAt(this, index);
    //insert before the node
    ln_ptr newnode = ln_constructor();
    ln_setValue(newnode, value);
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
    list->first = node->next;
  } else {
    node->prev->next = node->next;
  }
  if (!node->next) {
    list->last = node->prev;
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
  if (index >= this->node_count)
    return NULL;
  ln_ptr node = getNodeAt(this, index);
  return removeNode(this, node);
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

