
#include <stdio.h>
#include <stdint.h>
#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>
#include <sys/wait.h>

#include "alloc.h"
#include "os.h"
#include "build.h"
#include "string.h"

// If this zero initialization is not existed, it would be COM.
build_conf* Confp = 0;
static build_conf Conf;

int a = 1;

// static rules TMPRULE;
// _bind* BUF_P;
// size_t* CMD_P;
// void* DotFileHandle;

void f1() {}

void init() {

  f1();
  Conf.vars.first_var = __malloc(50 * sizeof(void*));
  Conf.rules.first_rule = __malloc(50 * sizeof(void*));
  
  char deps_graph_fname[] = "deps.dot";
  Conf.dot_file_handle = __os__open(deps_graph_fname, O_CREAT | O_WRONLY | O_TRUNC, 0777);
  Confp = &Conf;
  
  /* char str[] = "digraph graph_name {\n"; */
  /* __os__write(DotFileHandle, str, strlen(str));   */
}

