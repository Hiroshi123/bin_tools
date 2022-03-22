
#include <stdio.h>
#include <stdint.h>
#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>
#include "alloc.h"

#define CONTEXT_RULE_CMD 0x1
#define CONTEXT_RULE_TARGET 0x2
#define CONTEXT_VARIABLE 0x4
#define CONTEXT_READ_NOW 0x8

typedef struct {
  uint8_t* name;
  uint8_t* value;  
} var;

typedef struct {
  uint8_t* target;
  uint8_t* deps;
  uint8_t* cmd;
} rule;

static size_t* VARS_P;
static size_t* RULES_P;
static size_t* CMD_P;

static void* f1(char* fname, int* size) {

  size_t fd = __os__open(fname, O_RDONLY, 0777);
  if (fd == -ENOENT) {
    __write(1, "error\n", 6);
    return 0;
  }
  struct stat sb;
  __os__stat(fname, &sb);
  *size = sb.st_size;
  logger_emit_p(sb.st_size);
  logger_emit_p(sb.st_mtime);
  void* buf = __malloc(sb.st_size);
  __os__read(fd, buf, sb.st_size);
  logger_emit_p(buf);
  return buf;
}

static void* _strtok(uint8_t* p) {

  uint8_t has = 0;
  for (;;p++) {
    if (*p==0x20) {
      *p = 0x0;
      return p + 1;
    }
    if (*p==0x0) {
      return has;
    }
    has = 1;
  }
  return 0;
}

static uint8_t check_var(uint8_t** _p1, uint8_t** _t1, uint64_t* vars) {

  uint8_t* p1 = *_p1;
  uint8_t* t1 = *_t1;
  if (*(uint16_t*)p1 == 0x2824/*$(*/) {
    void* p2 = p1+2;
    for (;;p1++) {
      if (*p1== 0x29/*)*/) {
	*p1 = 0;
	break;
      }
    }
    uint8_t index = lookup_vars(p2, vars);
    if (index) {
      // when a referred variable is used, this expression is expanded properly.
      *t1 = 0x24;t1++;
      *t1 = 0x28;t1++;
      *t1 = index;t1++;
      *t1 = 0x29;t1++;
      p1++;
      *_t1 = t1;
      *_p1 = p1;
      return 1;
    } else {
      printf("not found variables\n");
      return -1;
    }
  }
  return 0;
}

static void* retrieve(uint64_t** dest, uint8_t* p, uint8_t* p1, uint8_t var_check) {
  uint8_t* t;
  uint8_t* t1;
  t = t1 = __malloc(1 + p - p1);
  uint8_t in_context = 0;
  for (;p1<p;p1++) {
    if (!in_context && (*p1 == 0x20)) continue;
    in_context = 1;
    if (var_check) check_var(&p1, &t1, VARS_P);
    *t1 = *p1;
    t1++;
  }
  *t1 = 0;
  for (p1--,t1--;*p1==0x20;p1--,t1--) *t1 = 0;  
  return t;
}

static void parse(uint8_t* p, uint8_t* e) {

  uint64_t* vars = VARS_P;
  uint8_t* p1 = 0;
  var* a_var = vars + 1;
  uint64_t* rules = RULES_P;
  uint8_t context = 0;
  // RULES_P = rules;
  // number of rules should be recorded on top of all records.
  uint64_t* r = rules;
  rule* a_rule = rules+1;
  
  for (;p<e;p++) {
    if (*p == 0x20) continue;
    if (*p == 0x0a) {
      if (p1 && p1 <= p) {
	if (context & CONTEXT_RULE_CMD) {	
	  __write(1, "aa\n", 3);
	  a_rule->cmd = retrieve(&a_rule->cmd, p, p1, 1);
	  /* __write(1, a_rule->target, strlen(a_rule->target)); */
	  /* __write(1, "\n", 1); */
	  __write(1, a_rule->cmd, strlen(a_rule->cmd));
	  __write(1, "\n", 1);
	  a_rule++;
	  *rules+=1;
	}
	if (context & CONTEXT_RULE_TARGET) {
	  __write(1, "ab\n", 3);
	  a_rule->deps = retrieve(&a_rule->deps, p, p1, 1);
	  context &= ~CONTEXT_RULE_TARGET;
	  context |= CONTEXT_RULE_CMD;
	  context &= ~CONTEXT_READ_NOW;	  
	}
	if (context & CONTEXT_VARIABLE) {
	  a_var->value = retrieve(&a_var->value, p, p1, 1);
	  p1 = 0;
	  context &= ~CONTEXT_VARIABLE;
	  context &= ~CONTEXT_READ_NOW;
	  // printf("var : %d\n",*vars);
	  // printf("\tname:%s\n",a_var->name);
	  // printf("\tvalue:%s\n",a_var->value);
	  *vars+=1;
	  a_var++;
	  continue;
	}
      }
      p1 = 0;
      continue;
    }
    if (*(uint16_t*)p == 0x3d3a) {      
      a_var->name = retrieve(&a_var->name, p, p1, 0);
      p1 = p + 2;
      context |= CONTEXT_VARIABLE;
      context &= ~CONTEXT_READ_NOW;
      continue;
    }
    if (*(uint8_t*)p == 0x3a) {
      a_rule->target = retrieve(&a_rule->target, p, p1, 1);      
      context |= CONTEXT_RULE_TARGET;
      context &= ~CONTEXT_READ_NOW;
      p1 = p + 1;
      continue;
      /* _strtok(p1); */
      /* __write(1, p1, strlen(p1)); */
      /* __write(1, "e\n", 2); */
    }
    if (!p1) {
      p1 = p;
      context |= CONTEXT_READ_NOW;
    }
  }
  /* a_rule->target = retrieve(&a_rule->target, p, p1, 1); */
  /* context |= CONTEXT_RULE_TARGET; */
  /* context &= ~CONTEXT_READ_NOW; */
}

static uint8_t isFile(void* p) {
  int ret = __os__open(p, O_RDONLY, 0777);
  if (ret == -ENOENT) {
    return 0;
  }
  return ret;
}

static uint32_t get_mtime(char* fname) {
  struct stat sb;
  __os__stat(fname, &sb);
  //  = sb.st_size;
  logger_emit_p(sb.st_mtime);
  return sb.st_mtime;
}

static void resolve_deps(rule* r) {
  uint8_t* p1 = r->deps;
  uint8_t* p;
  rule* v;  
  for (;;) {
    p = _strtok(p1);
    if (isFile(p1)) {
      get_mtime(p1);
      __os__write(1, p1, strlen(p1));
      __os__write(1, "\n", 1);
    }
    if (!p) break;
    if (p == 1) {
      break;
    }
  }
}

static uint8_t check_modification(rule* r) {

  uint8_t* p1 = r->deps;
  uint64_t a = 0;
  uint64_t b = 0;
  if (isFile(p1)) {
    a = get_mtime(p1);
  }
  if (isFile(r->target)) {
    b = get_mtime(r->target);
  }
  if (a > b) {
    return 1;
  }
  return 0;
}

static void do_exec(char* cmd) {

  __os__write(1, cmd, strlen(cmd));
  char* p = cmd;
  size_t args[10] = {};
  void* q = 0;
  int i=0;
  for (;;i++) {
    q = _strtok(p);    
    if (q == 1) {
      args[i] = p;
      __os__write(1, p, strlen(p));
      __os__write(1, "\n", 1);    
      break;
    }
    if (q == 0) {
      break;
    }
    args[i] = p;
    __os__write(1, p, strlen(p));
    __os__write(1, "\n", 1);
    p = q;
  }  
  if (__os__fork() == 0) {
    char* envp = 0;
    uint64_t ret = __os__execve(args[0], &args[0], &envp);
    if (ret == -1) {
      __os__write(1, "i\n", 2);
    }
  }  
  __os__write(1, "g\n", 2);
  
}

static void* assign_var(uint8_t* p,   uint8_t* q) {
  var* v = VARS_P + 1;
  // retrieve index value
  v += (*(p+2) - 1);
  uint8_t* s;
  for ( s = v->value ; *q = *s;q++,s++);
  return q;
}

static uint8_t check_assign_var(uint8_t** _p, uint8_t** _q) {
  uint8_t* p = *_p;
  uint8_t* q = *_q;
  if (*(uint16_t*)p == 0x2824/*$*/) {
    q = assign_var(p, q);
    p+=4;
  } else return 0;
  *_p = p;
  *_q = q;
  return 1;
}


static void* bind_cmd(rule* r) {

  uint8_t* p = r->cmd;
  uint8_t* q = CMD_P;
  uint8_t* s;  
  for (;*p!=0;) {

    // variable
    check_assign_var(&p, &q);
    // 1st deps($<)
    if (*(uint16_t*)p == 0x3c24) {
      p+=2;
      // stpcpy
      // for (s = resolve_var_suffix(r->deps) ; *q = *s;q++,s++);
      continue;
    }
    // target($@)
    if (*(uint16_t*)p == 0x4024) {
      __os__write(1, "h\n", 2);
      p+=2;
      s = r->target;// resolve_var_suffix(r->target);
      for ( ; *q = *s;q++,s++);
      continue;
    }
    if (*p) {
      *q = *p;
      p++,q++;
    } else {
      break;
    }
  }
  *q = 0;
  return CMD_P;
}

static void search_rule() {

  uint64_t len = *RULES_P;
  uint8_t* cmd;
  uint8_t* suffix;
  uint8_t i=0;
  uint8_t* c = 0;
  rule* t = RULES_P + 1;
  for (;i<len;i++,t++) {
    resolve_deps(t);
    //if (check_modification(t)) {
      cmd = bind_cmd(t);
      do_exec(cmd);
      __os__write(1, "e\n", 2);
      //}
    __os__write(1, "r\n", 2);
  }
}

static int strlen(char* p) {
  int i;
  for (i=0;*p;p++,i++);
  return i;
}

void start(char** argv) {

  VARS_P = __malloc(50 * sizeof(void*));
  RULES_P = __malloc(50 * sizeof(void*));
  CMD_P = __malloc(30 * sizeof(void*));
  
  char* s = "jij";
  logger_emit("misc.log", s);

  // __os__write(1, &argv[1], strlen(argv[0]));
  // __os__open(argv[1]);

  int argc = *argv++;
  if (argc == 1) {
    char* str = "usage\n"\
      "almost equivalent GNU make\n"\
      "1st : specify a build file\n"\
      "2nd : specify a target name\n\n";
    __os__write(1, str, strlen(str));
    return;
  }
  argv++;
  size_t* argv1 = *argv++;  
  // argv1 = argv;
  char* fname = argv1;
  
  int size = 0;
  uint8_t* buf = f1(fname, &size);
  if (buf == 0 || size == 0) {
    __os__write(1, "error\n", 6);
  }
  logger_emit_p(buf);
  
  parse(buf, buf + size);
  search_rule(/*argv[2]*/);
  
  /* argv1 = *argv++; */
  /* __write(1, argv1, 8); */
    
}

