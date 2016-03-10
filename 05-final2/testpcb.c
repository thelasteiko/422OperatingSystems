
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "pcb.h"

int main (void) {
  int pid, tid;
  pid = 0;
  tid = 0;
  pcb_base_ptr p1 = pcb_make_busy(pid, tid, 0);
  pid = pid + 1;
  tid = tid + 1;
  pcb_reg_ptr p2 = pcb_make_reg(pid, tid, 2, 0, 1000);
  printf("%s\r\n", pcb_base_toString(p1));
  printf("%s\r\n", pcb_reg_toString(p2));
  return 0;
}