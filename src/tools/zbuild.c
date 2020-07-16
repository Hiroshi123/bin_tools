
#include <stdio.h>
#include <stdint.h>
#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <sys/mman.h>
#include <sys/stat.h>

#include "alloc.h"
#include "os.h"
#include "build.h"
#include "thread.h"

static void* read_build_file(char* fname, int* size) {

  size_t fd = __os__open(fname, O_RDONLY, 0777);
  if (fd == -ENOENT) {
    __os__write(1, "error\n", 6);
    return 0;
  }
  struct stat sb;
  __os__stat(fname, &sb);
  *size = sb.st_size;
  void* buf = __malloc(sb.st_size);
  __os__read(fd, buf, sb.st_size);
  return buf;
}

static void th1(void* arg,uint64_t* arg2,uint64_t* arg3,uint64_t* arg4) {
  __os__write(1, "a!!\n", 4);
  for(;;);
}

static void th2() {
  __os__write(1, "b!!\n", 4);
}

#define USUAGE "usage\n"\
  "minimum GNU make\n"				\
  "-f : specify a build file\n"			\
  "1st : specify a target name\n\n"

void start(char** argv) {

  int argc = *argv++;
  if (argc == 1) {
    char* str = USUAGE;
    __os__write(1, str, sizeof(USUAGE));
    return;
  }
  
  // this init should be replaced something equivalent to attribute constructor
  // on my loader on a shared library.
  __z__build__init();
  
  argv++;
  char* fname = *argv++;
  size_t* target;
  if (argc == 2) {
    // target should be first rule.
    target = 0;
  } else {
    target = *argv;
  }
  int size = 0;
  uint8_t* buf = read_build_file(fname, &size);
  if (buf == 0 || size == 0) {
    __os__write(1, "error\n", 6);
  }
  
  __z__build__parse_makefile(buf, buf + size);

  
  // resolve();
  // __z__build__resolve_target();

  __z__build__traverse(target);
  
  // search_rule(target);
  // __z__std__init_thread_pool(1);
  
  
  for (;;);
  
  // edge ..  
}


