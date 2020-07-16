
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

void __z__build__init() {

  int ret = __os__open("b.txt", O_RDONLY, 0777);
  if (ret < 0) {
  logger_emit_p(ret);
    for (;;);
    return 0;
  }
  logger_emit_p(ret);
  __os__close(ret);
  for (;;);
  return;
  
  Conf.vars.first_var = __malloc(50 * sizeof(void*));
  Conf.rules.first_rule = __malloc(50 * sizeof(void*));
  
  char deps_graph_fname[] = "deps.dot";
  Conf.dot_file_handle = __os__open(deps_graph_fname, O_CREAT | O_WRONLY | O_TRUNC, 0777);
  Confp = &Conf;
  Conf.vars.var_hash_table.nbucket = 100;
  int hash_size = Conf.vars.var_hash_table.nbucket * sizeof(void*);
  Conf.vars.var_hash_table.bucket = __malloc(hash_size);
  Conf.vars.var_hash_table.hash_f = &__z__std__sysv_hash;
  
  Conf.rules.rule_target_hash_table.nbucket = 100;
  hash_size = Conf.rules.rule_target_hash_table.nbucket * sizeof(void*);
  Conf.rules.rule_target_hash_table.bucket = __malloc(hash_size);
  Conf.rules.rule_target_hash_table.hash_f = &__z__std__with_escape_hash;
  Conf.rules.implicit_pre_suffix_len_p = 0;
  // __z__sysv_hash;
  // __z__std__init_hash_table(&Conf.vars.var_hash_table);

  // __z__std__init_thread_pool(1);

  
}

