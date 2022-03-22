
#include "os.h"
#include <fcntl.h>
#include <stdlib.h>

void h1() {
  
  // for (;;);
  char b[4] = {};
  __os__open("ex.out", O_CREAT, 0777);
  __os__read(0, b, 4);
  __os__write(1, b, 4);
  
  malloc(0x10);
  
  /* char aa[4] = {}; */
  /* char* a = aa; */
  /* int i = 0; */
  /* for (;i < 4;i++,a++) { */
  /*   *a = 1; */
  /* } */
  // for (;;);
}

