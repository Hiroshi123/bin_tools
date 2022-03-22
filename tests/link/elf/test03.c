
#include <stdlib.h>

extern int f1();

int _start() {

  // printf("a\n");
  f1();
  exit(0);
  for (;;);
}

