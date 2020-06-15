
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

extern build_conf* Confp;

static uint8_t lookup_vars(uint8_t* query) {

  // TMPVAR.num;
  // uint64_t num = *p;
  var* v = Confp->vars.first_var;
  uint64_t i = 0;
  for (;i< Confp->vars.num;i++,v++) {
    if (!strcmp(query, v->name)) {
      return i+1;
      // return v;
    }
  }
  return 0;
}

static uint8_t check_var(uint8_t** _p1, uint8_t** _t1) {

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
    uint8_t index = lookup_vars(p2);
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
      // printf("not found variables\n");
      return -1;
    }
  }
  return 0;
}

void* retrieve(uint64_t** dest, uint8_t* p, uint8_t* p1, uint8_t var_check) {

  uint8_t* t;
  uint8_t* t1;
  t = t1 = __malloc(1 + p - p1);
  // *dest = t1;
  uint8_t in_context = 0;
  for (;p1<p;p1++) {
    if (!in_context && (*p1 == 0x20)) continue;
    in_context = 1;
    if (var_check) check_var(&p1, &t1);
    *t1 = *p1;
    t1++;
  }
  *t1 = 0;
  for (p1--,t1--;*p1==0x20;p1--,t1--) *t1 = 0;
  return t;
}

static int get_length_var(uint8_t* p) {
  var* v = Confp->vars.first_var;
  // retrieve index value
  v += (*(p+2) - 1);
  uint8_t* s;
  int i=0;
  for ( s = v->value ;*s;s++,i++);
  return i;
}

static int get_need_size(char* p, rule* r) {
  int i = 0;
  for (;*p;p++,i++) {
    if (*(uint16_t*)p == 0x2824/*$*/) {
      i+= get_length_var(p);
      i-=4;
    }
    // $? (every file of which the mtime is prior to targets)
    if (*(uint16_t*)p == 0x3f24/*$<*/) {
      i += strlen(r->deps);
      i-=2;
    }
    // $^ (all deps files)
    if (*(uint16_t*)p == 0x5e24/*$^*/) {
      i += strlen(r->deps);
      i-=2;
    }
    // $+ (deps file according to its position)
    if (*(uint16_t*)p == 0x2b24/*$+*/) {
      i += strlen(r->deps);
      i-=2;
    }
    // $* (target file except suffix)
    if (*(uint16_t*)p == 0x2a24/* $* */) {
      i += strlen(r->deps);
      i-=2;
    }
    // first deps
    if (*(uint16_t*)p == 0x3c24/*$<*/) {
      int j = 0;
      uint8_t* t = r->deps;
      for (;*t != 0 && *t != 0x20;t++,j++);
      i += j;
      i-=2;
    }
    // target
    if (*(uint16_t*)p == 0x4024/*$@*/) {
      i += strlen(r->target);
      i-=2;
    }
  }
  return i;
}

static void* assign_var(uint8_t* p, uint8_t* q) {
  var* v = Confp->vars.first_var;
  // retrieve index value
  v += (*(p+2) - 1);
  uint8_t* s;
  for ( s = v->value ; *q = *s;q++,s++);
  return q;
}

static uint8_t _check_assign_var(uint8_t** _p, uint8_t** _q) {
  uint8_t* p = *_p;
  uint8_t* q = *_q;
  void* i;
  if (*(uint16_t*)p == 0x2824/*$*/) {
    __os__write(1, "u\n", 2);
    i = q;
    q = assign_var(p, q);
    p+=4;
  } else return 0;
  return 1;
}

static void fill_vars(char* p, char* q, rule* r) {

  char* s;
  for (;*p;p++,q++) {
    if (*(uint16_t*)p == 0x2824/*$*/) {
      q = assign_var(p, q);
      p+=4;
    }
    // first deps
    if (*(uint16_t*)p == 0x3c24/*$<*/) {
      p+=2;
      for (s = r->deps; (*s != 0x20) && (*q = *s);q++,s++);
      // for (s = r->deps; *q = *s;q++,s++);
    }
    // $^ (all deps files)
    if (*(uint16_t*)p == 0x5e24/*$^*/) {
      p+=2;
      for (s = r->deps;*q = *s;q++,s++);
    }
    // target
    if (*(uint16_t*)p == 0x4024/*$@*/) {
      p+=2;
      for (s = r->target; *q = *s;q++,s++);
    }
    *q = *p;
  }
}

static void* resolve_vars(rule* t) {

  char* p = t->target;
  int i = get_need_size(p, t);
  void* q = __malloc(i+1);
  fill_vars(p, q, t);
  t->target = q;

  __os__write(1, "target\n", 7);
  __os__write(1, q, strlen(q));
  __os__write(1, "\n", 1);

  p = t->deps;
  i = get_need_size(p, t);
  q = __malloc(i+1);
  fill_vars(p, q, t);
  t->deps = q;

  __os__write(1, "deps\n", 5);
  __os__write(1, q, strlen(q));
  __os__write(1, "\n", 1);

  p = t->cmd->p;
  list* li;
  for (li = t->cmd;li;li = li->next) {
    // if there is something which needs to be resolved.
    if (need_resolve(li->p)) {
      i = get_need_size(li->p, t);
      q = __malloc(i+1);
      fill_vars(li->p, q, t);
      li->p = q;
    }
  }
  return;
}

////////////////////////////////////////////////////////////////////
/////// export /////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////

void resolve() {
  uint64_t len = Confp->rules.num;
  rule* t = Confp->rules.first_rule + 1;
  uint8_t i = 0;
  for (;i<len;i++,t++) {
    resolve_vars(t);
  }
}

uint8_t check_assign_var(uint8_t** _p, uint8_t** _q) {
  uint8_t* p = *_p;
  uint8_t* q = *_q;
  void* i;
  if (*(uint16_t*)p == 0x2824/*$*/) {
    __os__write(1, "u\n", 2);
    i = q;
    q = assign_var(p, q);
    p+=4;
  } else return 0;
  *_p = p;
  *_q = q;
  __os__write(1, i, strlen(i));
  __os__write(1, "\n", 1);
  return 1;
}


