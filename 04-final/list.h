
#ifndef
#define LIST

typedef struct listnode_type {
  void * value;
  struct listnode_type * next;
  struct listnode_type * prev;
} listnode;
typedef listnode * ln_ptr;
typedef struct list_type {
  ln_ptr first;
  ln_ptr last;
  int node_count;
} list;
typedef list * list_ptr;

list_ptr ls_constructor(void);
void * ls_get(list_ptr this, int index);
int ls_insertAt(list_ptr this, int index, void * value);
void * ls_remove(list_ptr this, int index);
int ls_destructor(list_ptr this);

#endif