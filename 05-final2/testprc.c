
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "prc.h"

int main (void) {
  printf("Start.\r\n");
  prc_ptr reg = prc_constructor();
  prc_ptr pc = prc_constructor();
  printf("Constructed prcs.\r\n");
  int tid = 0;
  tid = prc_initialize(reg, 0, tid, 2, 0, regular, -1);
  tid = prc_initialize(pc, 1, tid, 2, 1, pc_pair, 0);
  printf("Created prcs.\r\n");
  printf("Reg\r\n%s\r\n", prc_toString(reg));
  printf("PC\r\n%s\r\n", prc_toString(pc));
  printf("End.");
  return 0;
}