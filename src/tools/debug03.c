
#include <stdio.h>

#include "logger.h"

void start(/*int argc,*/ char** argv) {
  mem_init();

  /* size_t* t = *argv; */
  int argc = *argv++;
  if (argc == 3) {
    for (;;)
      ;
  }

  // argv++;
  /* argv++; */
  /* argv++; */
  char* argv1 = *argv++;
  __write(1, argv1, 8);

  /* char* s = "mem"; */
  /* logger_emit("misc.log", s);   */

  __malloc(2);
  __malloc(1);
  __malloc(1);
  __malloc(1);
  __malloc(1);

  /* for (;*argv != 0;argv++); */
  /* argv1 = *argv++; */
  /* __write(1, argv1, 3); */

  // for(;;);
}
