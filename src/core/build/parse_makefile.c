
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

typedef uint8_t State;
extern build_conf* Confp;

static parse_data PARSE_DATA = {
  .context = 0,
  .str = 0,
  .var_num = 0,
  .rule_num = 0,
};

static State do_context_none(uint8_t* p) {

  if (*p == 0x24) {
    PARSE_DATA.str = p;
    return CONTEXT_READ_NOW;
  }
  if ((0x41 <= *p && *p <= 0x5a) || (0x61 <= *p && *p <= 0x7a)) {
    __os__write(1, "y\n", 2);
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
      return CONTEXT_VARIABLE;
    }
    Confp->rules.num++;
    a_rule = Confp->rules.first_rule + Confp->rules.num - 1;
    *p = 0;
    a_rule->target = PARSE_DATA.str;
    // retrieve(&a_rule->target, p, PARSE_DATA.str, 0);
    PARSE_DATA.str = p + 1;
    return CONTEXT_RULE_DEPS;
  case 0x3d/*=*/:
    a_var = Confp->vars.first_var + Confp->vars.num;
    *p = 0;
    a_var->name = PARSE_DATA.str;
    // retrieve(&a_var->name, p, PARSE_DATA.str, 0);
    a_var->kind = ASSIGN_RECURSIVE;
    PARSE_DATA.str = p + 1;
    return CONTEXT_VARIABLE;
  case 0x3f/*?*/:
    if (*(p+1) == 0x3d/*=*/) {
      a_var = Confp->vars.first_var + Confp->vars.num;
      *p = 0;
      a_var->name = PARSE_DATA.str;
      // retrieve(&a_var->name, p, PARSE_DATA.str, 0);
      a_var->kind = ASSIGN_RECURSIVE_ALLOW_NON_DEFINED;
      PARSE_DATA.str = p + 1;
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
  if (*p == 0x09/*TAB*/ && *(p-1) == 0x0a) {
    __os__write(1, "s\n", 2);
    PARSE_DATA.str = p + 1;
    /* PARSE_DATA.context |= CONTEXT_RULE_CMD; */
    /* PARSE_DATA.context &= ~CONTEXT_RULE_SEP; */
    return CONTEXT_RULE_CMD;
  }
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
      __z__std__hash_set(a_var->name, a_var->value, col_type);
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
      __os__write(1, "c\n", 2);
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
      //retrieve(&a_rule->cmd, p, PARSE_DATA.str, 0);
      PARSE_DATA.str = 0;
    }
    return ret;
  default:
    return CONTEXT_RULE_CMD;
  }
}

static State do_context_rule_deps(uint8_t* p) {
  if (*p == 0x0a/*LF\n*/) {
    __os__write(1, "d\n", 2);
    rule* a_rule = Confp->rules.first_rule + Confp->rules.num - 1;
    a_rule->deps = PARSE_DATA.str;
    *p = 0;
    // retrieve(&a_rule->deps, p, PARSE_DATA.str, 0);
    PARSE_DATA.str = 0;

    /* PARSE_DATA.context |= CONTEXT_RULE_SEP;
    /* PARSE_DATA.context &= ~CONTEXT_RULE_DEPS; */
    /* PARSE_DATA.context &= ~CONTEXT_READ_NOW; */
    return CONTEXT_RULE_SEP;
  }
  return PARSE_DATA.context;
}

/////////////////////////////////////////////////////////////
///// export ////////////////////////////////////////////////
/////////////////////////////////////////////////////////////

void tmp1() {
  __os__write(1, "tmp\n", 4);
}

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
      break;
    }
  }

  __z__std__put_task(&tmp1);

}

