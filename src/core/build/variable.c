
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

static var* __lookup_vars(uint8_t* query) {

  // TMPVAR.num;
  // uint64_t num = *p;
  var* v = Confp->vars.first_var;
  uint64_t i = 0;
  for (;i< Confp->vars.num;i++,v++) {
    if (!strcmp(query, v->name)) {
      return v;
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
    var* v = __lookup_vars(p2);
    __os__write(1, "check\n", 6);
    __os__write(1, v->value, strlen(v->value));
  }
  return 0;
}

static int get_length_var(uint8_t* p) {
  var* v = Confp->vars.first_var;
  *(p+3) = 0;
  int index = lookup_vars(p+2);
  if (index) v += (index - 1);
  else {
    __os__write(1, "error\n", 6);
  }
  uint8_t* s;
  int i=0;
  __os__write(1, v->value, strlen(v->value));
  // logger_emit_p(i);
  uint8_t* q = __malloc(strlen(v->value)+1);
  for ( s = v->value ;*s;s++,q++) {
    *q = *s;
  }
  return q;
}

void* resolve_vars(char* p) {
  int i = 0;
  char* t;
  char* v;
  int need_resolve = 0;
  int copy_now = 0;;
  char* p1 = p;
  char* q = 0;
  
 b1:
  for (;*p;p++,i++) {
    if (p == 0x24) {
      p++;
      switch (*p) {
      case 0x28:
	need_resolve = 1;
	for (t=p;*p != ')';p++);
	*p = 0;
	v = __z__std__hash_find(&Confp->vars.var_hash_table, p);
	if (v == 0) {
	  __os__write(1, "error\n", 6);
	  return 0;
	}
	for (;*q = *v;q++,v++);
	break;
      default:
	break;
      }
    }
    if (copy_now) {
      *q = *p;
      q++;
    }
  }
  if (need_resolve) {
    need_resolve = 0;
    copy_now = 1;
    q = __malloc(100);
    p = p1;
    goto b1;
  }
  return q;
}

static int get_need_size(char* p, rule* r) {
  int i = 0;
  int j;
  uint8_t* t;
  for (;*p;p++,i++) {
    switch (*p) {
    case 0x24:
      {
	p++;
	switch (*p) {
	case 0x28:// /*$(*/
	  i+= get_length_var(p) - 4;
	  break;
	case 0x2a:// $* (target file except suffix)
	  i += strlen(r->deps);
	  break;
	case 0x2b:// $+ (deps file according to its position)
	  i += strlen(r->deps);
	  break;
	case 0x3c:// $< first deps
	  j = 0;
	  t = r->deps;
	  for (;*t != 0 && *t != 0x20;t++,j++);
	  i += j;
	  i-=2;
	case 0x3f:// $? (every file of which the mtime is prior to targets)
	  i += strlen(r->deps);
	  break;
	case 0x40:/*$@*/
	  break;
	case 0x5e:// $^ (all deps files)
	  i += strlen(r->deps) - 2;
	default:
	  break;
	}
      }
    case 0x0a:
      return 0;
    default:
      break;
    }
  }
}

static void* __assign_var(uint8_t* p, uint8_t* q) {
  var* v = Confp->vars.first_var;
  // retrieve index value
  v += (*(p+2) - 1);
  uint8_t* s;
  for ( s = v->value ; *q = *s;q++,s++);
  return q;
}

static void* assign_var(uint8_t* p, uint8_t* q) {
  var* v = Confp->vars.first_var;
  __os__write(1, "v\n", 2);
  int index = lookup_vars(p+2);
  // retrieve index value
  v += (index - 1);
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
    switch (*p) {
    case 0x0a:
      break;
    case 0x24: {
      p++;
      switch (*p) {
      case 0x28:
	q = assign_var(p-1, q);
	for (;*p != ')';p++);
	continue;
      case 0x3c:
	for (s = r->deps; (*s != 0x20) && (*q = *s);q++,s++);
	continue;
      case 0x5e:
	for (s = r->deps;*q = *s;q++,s++);
	continue;
      case 0x40:
	for (s = r->target; *q = *s;q++,s++);
	continue;
      default:
	break;
      }
    }
    default:
      *q = *p;
      break;
    }
  }
}

static uint8_t need_resolve(char* p) {

  uint8_t i = 0;
  for (;*p;p++,i++) {
    switch (*p) {
    case 0x24:
      {
	p++;
	switch (*p) {
	case 0x28:
	case 0x3c:
	case 0x40:
	case 0x5e:
	  return i;
	default:
	  break;
	}
      }
    case 0x0a:
      return 0;
    default:
      break;
    }
  }
  return -1;
}

static char* __need_resolve(char* p) {

  uint8_t i = 0;
  char* q = 0;
  char* p1 = p;
  for (;*p;p++,i++) {
    switch (*p) {
    case 0x24:
      {
	p++;
	switch (*p) {
	case 0x28: {
	  for (q = p;*p != ')' && *p != 0 ;p++);
	  *p = 0;
	  return q;
	}
	case 0x3c:
	case 0x40:
	case 0x5e:
	  return p;
	default:
	  break;
	}
      }
    case 0x0a:
      return p1;
    default:
      break;
    }
  }
  return p1;
}

static void* resolve_all(rule* t) {

  int i;
  void* q;
  char* p = t->target;
  if (need_resolve(p)) {
    i = get_need_size(p, t);
    q = __malloc(i+1);
    fill_vars(p, q, t);
    t->target = q;
  }

  p = t->deps;
  if (need_resolve(p)) {
    i = get_need_size(p, t);
    q = __malloc(i+1);
    fill_vars(p, q, t);
    t->deps = q;
  }

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

void* retrieve(uint64_t** dest, uint8_t* p, uint8_t* p1, uint8_t var_check) {

  uint8_t* t;
  uint8_t* t1;
  // name check
  if (var_check == 2) {
    t = t1 = __malloc(1 + p - p1);
    for (;p1<p;p1++) {
      *t1 = *p1;
    }
    var* v = __lookup_vars(t1);
    if (v) {
     v->value =
      __os__write(1, "j\n", 2);
    }
    return t;
  }
  // value check
  if (var_check == 1) {
    if (need_resolve(p1)) {
      int i = get_need_size(p1, 0);
      t = t1 = __malloc(i+1);
      fill_vars(p1, t, 0);
      return t;
    }
  }
  t = t1 = __malloc(1 + p - p1);
  uint8_t in_context = 0;
  for (;p1<p;p1++) {
    if (!in_context && (*p1 == 0x20)) continue;
    in_context = 1;
    if (var_check) {
      // need_resolve(p1);
    } // check_var(&p1, &t1);
    *t1 = *p1;
    t1++;
  }
  *t1 = 0;
  for (p1--,t1--;*p1==0x20;p1--,t1--) *t1 = 0;
  return t;
}

void resolve() {
  uint64_t len = Confp->rules.num;
  rule* t = Confp->rules.first_rule;
  uint8_t i = 0;
  for (;i<len;i++,t++) {
    __os__write(1, "!\n", 2);    
    resolve_all(t);
  }
}

char* __z__build__resolve(char* s, rule* r) {

  char* m = 0;
  char* m1 = 0;
  char* s1 = s;
  int size = 0;
  int num = 0;
  char* ss;
  char* s2;
  uint8_t do_copy = 0;
  int i = 0;
  
 b1:

  for (;;) {
    s2 = __need_resolve(s);
    i = s2 - s;
    if (i == 0) break;
    switch (*s2) {
    case '(':
      ss = __z__std__hash_find(&Confp->vars.var_hash_table, s2 + 1);
      break;
    case '<':
      ss = r->deps;
      /* __os__write(1, r->deps, strlen(r->deps)); */
      /* __os__write(1, "hhhh\n", 5); */
      /* for (;;); */
      // do something
      break;
    case '^':
      break;
    default:
      break;
    }
    if (ss) {
      if (do_copy) {
	// put str before $()
	for (;;s++,m++,i--) {
	  if (i == 2) break;
	  if (*s) *m=*s;
	  else break;
	}
	s++;
	// put str inside $()
	for (;*m=*ss;ss++,m++);
	// break;
	// continue;
      } else {
	size += i + strlen(ss);
      }
    } else {
      __os__write(1, "error\n", 6);
    }
    s += i + 1;
  }
  if (do_copy) {
    for (;*m = *s;s++,m++);
  }
  if (size) {
    for (;*s;size++,s++);
    m = m1 = __malloc(size);
    do_copy = 1;
    size = 0;
    s = s1;
    goto b1;
  }
  return m1 ? m1 : s1;
}

void __z__build__resolve_target() {

  uint64_t len = Confp->rules.num;
  rule* t = Confp->rules.first_rule;
  uint8_t i = 0;
  for (;i<len;i++,t++) {
    __os__write(1, t->target, strlen(t->target));
    t->target = __z__build__resolve(t->target, 0);
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


