
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
#include "logger.h"
#include "build.h"
#include "string.h"

typedef uint8_t State;
extern build_conf* Confp;

static parse_data PARSE_DATA = {
  .context = 0,
  .str = 0,
  .var_num = 0,
  .rule_num = 0,
};

static State do_context_none(uint8_t* p) {

  if (*p == 0x23/*#*/) {
    logger_emit("misc.log", "[none] -> [comment]\n");
    return CONTEXT_COMMENT;
  }
  if (*p == 0x3a/*:*/) {
    logger_emit("misc.log", "[none] -> [not yet]\n");
    return 0;
  }
  if (*p == 0x3b/*;*/) {
    logger_emit("misc.log", "[none] -> [not yet]\n");
    return 0;
  }
  if (*p == 0x3d/*=*/) {
    logger_emit("misc.log", "[none] -> [not yet]\n");
    return 0;
  }
  // if ((0x41 <= *p && *p <= 0x5a) || (0x61 <= *p && *p <= 0x7a)) {
  if ((0x21 <= *p && *p <= 0x7e)) {
    logger_emit("misc.log", "[none] -> [read_now]\n");
    PARSE_DATA.str = p;
    return CONTEXT_READ_NOW;
    // PARSE_DATA.context |= CONTEXT_READ_NOW;
  }
  return PARSE_DATA.context;
}

static State do_context_comment(uint8_t* p) {
  switch (*p) {
  case 0x0a/*LF\n*/:
    return CONTEXT_NONE;
  default:
    return CONTEXT_VARIABLE;
  }
}

static uint8_t is_implicit(uint8_t* s, uint8_t* q) {
  uint8_t i = 1;
  for (;s<q;s++,i++) {
    if (*s == '%') return i;
  }
  return 0;
}

static void eval_target(void* p) {

  uint8_t i;
  if (i = is_implicit(PARSE_DATA.str, p)) {
    if (i) {
      implicit_trim_len* trim =
	__malloc(sizeof(implicit_trim_len));
      trim->begin = i - 1;
      trim->end = p - (void*)PARSE_DATA.str - i;
      logger_emit_p(p - (void*)PARSE_DATA.str);
      logger_emit("misc.log","ko\n");
      list* l = __malloc(sizeof(list));
      l->p = trim;
      if (Confp->rules.implicit_pre_suffix_len_p == 0) {
	Confp->rules.implicit_pre_suffix_len_p = l;
	logger_emit("misc.log","ok\n");
      } else {
	list* pre = Confp->rules.implicit_pre_suffix_len_p;
	for (;pre->next;pre = pre->next);
	pre->next = l;
      }
    }
  }
}

static State do_context_read_now(uint8_t* p) {

  var* a_var;
  rule* a_rule;
  switch (*p) {
  case 0x2b/*+*/:
    if (*(p+1) == 0x3d/*=*/) {
      *p = 0;
      a_var = Confp->vars.first_var + Confp->vars.num;
      a_var->name = PARSE_DATA.str;
      a_var->kind = ASSIGN_ADD;
      PARSE_DATA.str = p + 2;
      logger_emit("misc.log", "[var +=]\n");
      return CONTEXT_VARIABLE;
    }
    break;

  case 0x3a/*:*/:
    if (*(p+1) == 0x3d/*=*/) {
      *p = 0;
      a_var = Confp->vars.first_var + Confp->vars.num;
      a_var->name = PARSE_DATA.str;
      a_var->kind = ASSIGN_IMMEDIATE;
      PARSE_DATA.str = p + 2;
      logger_emit("misc.log", "[var :=]\n");
      return CONTEXT_VARIABLE;
    }
    Confp->rules.num++;
    a_rule = Confp->rules.first_rule + Confp->rules.num - 1;
    *p = 0;
    // TODO :: In this stage, evaluation needs to be done.
    // This includes implicit suffix rule check.
    // So far, just implicit rule check is added.
    eval_target(p);
    a_rule->target = PARSE_DATA.str;
    PARSE_DATA.str = p + 1;
    logger_emit("misc.log", "[read_now] -> [deps]\n");
    return CONTEXT_RULE_DEPS;
  case 0x3d/*=*/:
    a_var = Confp->vars.first_var + Confp->vars.num;
    *p = 0;
    a_var->name = PARSE_DATA.str;
    a_var->kind = ASSIGN_RECURSIVE;
    PARSE_DATA.str = p + 1;
    logger_emit("misc.log", "[read_now] -> [variable]\n");
    return CONTEXT_VARIABLE;
  case 0x3f/*?*/:
    if (*(p+1) == 0x3d/*=*/) {
      a_var = Confp->vars.first_var + Confp->vars.num;
      *p = 0;
      a_var->name = PARSE_DATA.str;
      a_var->kind = ASSIGN_RECURSIVE_ALLOW_NON_DEFINED;
      PARSE_DATA.str = p + 1;
      logger_emit("misc.log", "[read_now] -> [variable]\n");
      return CONTEXT_VARIABLE;
    }
    return CONTEXT_READ_NOW;
  default:
    // PARSE_DATA.str = p;
    return CONTEXT_READ_NOW;
  }
  return 0;
}

static State do_context_sep(uint8_t* p) {
  if (*p == 0x09/*TAB*/ && *(p-1) == 0x0/*0x0a*/) {
    PARSE_DATA.str = p + 1;
    logger_emit("misc.log", "[seperator] -> [cmd]\n");
    return CONTEXT_RULE_CMD;
  }
  // if ((0x41 <= *p && *p <= 0x5a) || (0x61 <= *p && *p <= 0x7a)) {
  if ((0x21 <= *p && *p <= 0x7e)) {
    PARSE_DATA.str = p;
    logger_emit("misc.log", "[seperator] -> [read_now]\n");
    return CONTEXT_READ_NOW;
  }
  logger_emit("misc.log", "[sep] -> [sep]\n");
  return CONTEXT_RULE_SEP;
}

static State do_context_variable(uint8_t* p) {

  State r;
  switch (*p) {
  case 0x0a/*LF\n*/:
    r = CONTEXT_NONE;
    goto b1;
  case 0x23/*#*/:
    r = CONTEXT_COMMENT;
  b1: {
      var* a_var = Confp->vars.first_var + Confp->vars.num;
      int col_type = COL_OVERRIDE;
      *p = 0;
      if (a_var->kind == ASSIGN_IMMEDIATE) {
        void* r = resolve_vars(PARSE_DATA.str);
	if (r) a_var->value = r;
	else a_var->value = PARSE_DATA.str;
      } else if (a_var->kind == ASSIGN_ADD) {
	col_type = COL_ADD;
	*(char*)(PARSE_DATA.str - 1) = 0x20;
	a_var->value = PARSE_DATA.str - 1;
      } else a_var->value = PARSE_DATA.str;
      __z__std__hash_set(&Confp->vars.var_hash_table, a_var->name, a_var->value, col_type);
      PARSE_DATA.str = 0;
    }
    return r;
  default:
    return CONTEXT_VARIABLE;
  }
}

static State do_context_rule_cmd(uint8_t* p) {

  State ret;
  switch (*p) {
  case 0x0a/*LF\n*/:
    ret = CONTEXT_RULE_SEP;
    goto b1;
  case 0x23/*#*/:
    ret = CONTEXT_COMMENT;
  b1: {
      rule* a_rule = Confp->rules.first_rule + Confp->rules.num - 1;
      a_rule->num_cmd++;
      list* li = __malloc(sizeof(list));
      // if there is precedented list element, set this as next of previous one.
      list* pre;
      if (a_rule->cmd) {
	for (pre = a_rule->cmd;pre->next;pre = pre->next);
	pre->next = li;
      } else {
	a_rule->cmd = li;
      }
      *p = 0;
      li->p = PARSE_DATA.str;
      PARSE_DATA.str = 0;
      logger_emit("misc.log", "[cmd] -> [seperator]\n");
    }
    return ret;
  default:
    return CONTEXT_RULE_CMD;
  }
}

static State do_context_rule_deps(uint8_t* p) {
  if (*p == 0x0a/*LF\n*/) {

    rule* a_rule = Confp->rules.first_rule + Confp->rules.num - 1;
    char* q = PARSE_DATA.str;
    for (;;q++) if (*q != ' ') break;
    a_rule->deps = q;
    for (;;p--) if (*p != ' ') break;
    *p = 0;
    __z__std__hash_set(&Confp->rules.rule_target_hash_table,
		       a_rule->target, a_rule, COL_RAISE_ERROR/*col_type*/);
    PARSE_DATA.str = 0;
    logger_emit("misc.log", "[deps] -> [seperator]\n");
    return CONTEXT_RULE_SEP;
  }
  return PARSE_DATA.context;
}

/////////////////////////////////////////////////////////////
///// export ////////////////////////////////////////////////
/////////////////////////////////////////////////////////////

void* __z__build__parse_makefile(uint8_t* p, uint8_t* e) {

  State next = 0;
  for (;p<e;p++) {
    switch (PARSE_DATA.context) {
    case CONTEXT_NONE:
      PARSE_DATA.context = do_context_none(p);
      break;
    case CONTEXT_COMMENT:
      PARSE_DATA.context = do_context_comment(p);
      break;
    case CONTEXT_READ_NOW:
      PARSE_DATA.context = do_context_read_now(p);
      break;
    case CONTEXT_RULE_DEPS:
      PARSE_DATA.context = do_context_rule_deps(p);
      break;
    case CONTEXT_RULE_SEP:
      PARSE_DATA.context = do_context_sep(p);
      break;
    case CONTEXT_RULE_CMD:
      PARSE_DATA.context = do_context_rule_cmd(p);
      break;
    case CONTEXT_VARIABLE:
      PARSE_DATA.context = do_context_variable(p);
      break;
    default:
      __os__write(1, "e\n", 2);
      // __os__write(1, "unknown state\n", sizeof("unknown state\n"));
      break;
    }
  }
  logger_emit("misc.log", "ok!\n");
  __os__write(1, "ok!\n", 4);
  // for (;;);
  int ret = __os__open("b.txt", O_RDONLY, 0777);
  if (ret < 0) {
  logger_emit_p(ret);
    for (;;);
    return 0;
  }
  logger_emit_p(ret);
  __os__close(ret);
  for (;;);

}

