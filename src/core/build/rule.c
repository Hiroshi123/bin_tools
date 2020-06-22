

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

#define write_to_dot_file(X) __os__write(Confp->dot_file_handle, X, strlen(X));

extern build_conf* Confp;

static _bind* BUF_P;
static size_t* CMD_P;

static uint8_t is_file(void* p) {

  int ret = __os__open(p, O_RDONLY, 0777);
  if (ret == -ENOENT) {
    return 0;
  }
  // __os__close(p);
  return ret;
}

static void write_node(void* p1, void* p2) {

  write_to_dot_file(p1);
  write_to_dot_file(" [label = \"target : ");
  write_to_dot_file(p1);
  write_to_dot_file("\n");
  write_to_dot_file("command : ");
  write_to_dot_file(p2);
  write_to_dot_file("\n");
  write_to_dot_file("\"]\n");

}

static void write_edge(void* p1, void* p2) {

  // construct node -> node;
  write_to_dot_file(p1);
  write_to_dot_file(" -> ");
  write_to_dot_file(p2);
  write_to_dot_file(";\n");
}


static void* has_suffix(uint8_t* p, uint8_t** last) {
  // if the rule contains "%"
  uint8_t count = 0;
  uint8_t i = 0;

  for (;*p!=0x20 && *p!=0x00;p++,i++) {
    // if (!begin) // printf("!%c", *p);
    if (*p == 0x25) {
      count = i;
      if (!last) return p + 1;
    }
  }
  if (count) *last = count;
  if (count) // printf("count:%d\n",count);
  return (count) ? p : 0;
}

static void* bind_suffix(uint8_t* suffix, uint8_t* p, uint8_t c) {

  uint8_t* p1;
  uint8_t* p2;
  uint8_t* p3;
  p1 = p2 = p;
  uint8_t* s1 = suffix;
  // go to last
  for (;*p1!=0x0;p1++);
  for (;*suffix!=0x25/*%*/;p1--,suffix--) {
    if (*p1 != *suffix) {
      suffix = 0;
      break;
    }
  }
  if (suffix) {
    p3 = BUF_P->bind_str;
    for (p2+=c;p2<=p1;p2++,p3++) *p3 = *p2;
    *p3 = 0x0;
    return 1;
  }
  return 0;
}

static void clear_buf() {

  uint8_t* p = BUF_P->bind_full_str;
  for (;*p!=0x0;p++) {
    *p = 0;
  }
}

static void unbind_suffix() {

  clear_buf();
  uint8_t* p = BUF_P->bind_str;
  for (;*p!=0x0;p++) {
    *p = 0;
  }
}

static void* resolve_var_suffix(uint8_t* j) {

  uint8_t* s = j;
  uint8_t* p = BUF_P->bind_str;
  if (!j) return p;
  uint8_t* k = BUF_P->bind_full_str;
  for (;*j!=0x20 && *j!=0x0;k++,j++) {
    check_assign_var(&j, &k);
    if (*j == 0x2a/* * */) {
      do_wildcard(s, &j, &k);
    }
    if (*j==0x25 /* % */) {
      for (;*k = *p;k++,p++);
      k--;
    } else {
      *k = *j;
    }
  }
  *k = 0x0;
  return BUF_P->bind_full_str;
}

static uint32_t get_mtime(char* fname) {
  struct stat sb;
  __os__stat(fname, &sb);
  //  = sb.st_size;
  // logger_emit_p(sb.st_mtime);
  return sb.st_mtime;
}

static uint8_t modified(rule* r, uint8_t* p1) {

  __os__write(1, "abcd\n", 5);
  __os__write(1, r->target, strlen(r->target));
  __os__write(1, "\n", 1);
  __os__write(1, p1, strlen(p1));
  __os__write(1, "\n", 1);

  uint64_t a = 0;
  uint64_t b = 0;
  if (is_file(p1)) {
    a = get_mtime(p1);
  }
  if (is_file(r->target)) {
    b = get_mtime(r->target);
  }
  if (a > b) {
    return 1;
  }
  return 0;
}

// check dependencies.
// this must be resolved by files or rules.
static uint8_t resolve_deps(rule* r) {
  uint8_t* p1 = r->deps;
  uint8_t* p;
  rule* v;
  uint8_t needs_exec = 0;
  __os__write(1, "a!\n", 3);
  for (;;) {
    p = _strtok(p1);
    // printf("look for %s\n",p1);
    if (!p) break;
    if (has_suffix(p1, 0)) {
      __os__write(1, "a\n", 2);
      p1 = resolve_var_suffix(p1);
    } else {
      __os__write(1, p1, strlen(p1));
      __os__write(1, "b\n", 2);
    }
    write_edge(r->target, p1);
    if (is_file(p1)) {
      needs_exec = modified(r, p1);
    } else {
      __os__write(1, p1, strlen(p1));
      __os__write(1, "!!\n", 3);
      // if it is
      v = search_rule(p1);
      if (v) {
	__os__write(1, "found\n", 6);
	needs_exec = 1;
	// return 1;
	// printf("found a rule : %x,target : %s, cmd : %s\n",p1,v->target, v->cmd);
      } else {
	// printf("no rule found\n");
      }
    }
    if (p == 1) {
      break;
    }
    p1 = p;
  }
  return needs_exec;
}

static void* convert_path_slash(uint8_t* p) {
  for (;*p!=0;p++) {
    if (*p == '/')
      *p = '\\';
  }
}

static void do_exec(void* cmd) {
  
  char* p = ((list*)cmd)->p;
  size_t* args = __malloc(sizeof(void*) * 10);
  void* q = 0;
  int i=0;
  for (;;i++) {
    q = _strtok(p);
    if (q == 1) {
      args[i] = p;
      /* __os__write(1, p, strlen(p)); */
      /* __os__write(1, "\n", 1);   */
      break;
    }
    if (q == 0) {
      break;
    }
    args[i] = p;
    p = q;
  }
  int pid = __os__fork();
  if (pid == 0) {
    // char* envp = 0;
    char* envp = __malloc(sizeof(void*));
    uint64_t ret = __os__execve(args[0], args, envp);
    if (ret == -1) {
      __os__write(1, "exec error\n", 12);
    }
    for(;;);
  } else {
    int status;
    __os__wait4(pid, &status, WUNTRACED | WCONTINUED);
    // for(;;);
  }
  __os__write(1, p, strlen(p));
  __os__write(1, "exec\n", 5);
}

static void* get_fullpath(void* r) {

}

static void* _do_wildcard(void* path, void* hFind, uint8_t** _p) {

  /* WIN32_FIND_DATA fileInfo; */
  /* // printf("path:%s\n",path); */
  /* // fileInfo.cFileName */
  /* uint8_t alc = 0; */
  /* uint8_t* p = *_p; */
  /* uint8_t* p1; */
  /* uint8_t* p2; */
  /* void* ret; */
  /* if (!hFind) */
  /*   ret = FindFirstFile(path , &fileInfo); */
  /* else { */
  /*   ret = FindNextFile(hFind, &fileInfo); */
  /*   // add path on second time. */
  /*   if (ret) */
  /*     for (p2 = path;*p2!=0x0 && *p2!=0x2a;p2++,p++) */
  /* 	*p = *p2; */
  /* } */
  /* if (ret) { */
  /*   for (p1 = &fileInfo.cFileName;*p = *p1;p++,p1++); */
  /*   *p = 0x20; */
  /*   *_p = p+=1; */
  /*   // printf("w:%d,%s\n", strlen(fileInfo.cFileName), fileInfo.cFileName); */
  /* } else { */
  /*   FindClose(hFind); */
  /* } */
  /* return ret; */
}

static void do_wildcard(uint8_t* s, uint8_t** _p, uint8_t** _q) {
  uint8_t* p = *_p;
  uint8_t* q = *_q;
  uint8_t* p1;
  if (p1 = _strtok(s)) {
    // void* len = do_wildcard(p2, 0);
    void* h = _do_wildcard(s, 0, &q);
    for (;_do_wildcard(s, h, &q););
    for (;*p!=0x0 && *p!=0x20;p++);
    if (p1 == 1) {
    } else {
      *(p-1) = 0x20;
    }
  }
  *_p = p;
  *_q = q;
}

static void* bind_deps(uint8_t* deps, uint8_t* q) {
  uint8_t* s = has_suffix(deps, 0) ? resolve_var_suffix(deps) : deps;
  for (; *q = *s;q++,s++);
  return q;
}

static void* bind_cmd(rule* r) {
  uint8_t* p = r->cmd;
  uint8_t* q = CMD_P;
  uint8_t* t = q;
  uint8_t* s;
  uint8_t begin = 0;
  uint8_t* p2;
  for (;*p!=0;) {
    // if space
    if (*(uint8_t*)p == 0x20) {
      p2 = 0;
      begin = 1;
    } else if (begin) {
      p2 = p;
      begin = 0;
    }
    // variable
    check_assign_var(&p, &q);
    // 1st deps($<)
    if (*(uint16_t*)p == 0x3c24) {
      p+=2;
      // stpcpy
      for (s = resolve_var_suffix(r->deps) ; *q = *s;q++,s++);
      continue;
    }
    // target($@)
    if (*(uint16_t*)p == 0x4024) {
      p+=2;
      s = resolve_var_suffix(r->target);
      for ( ; *q = *s;q++,s++);
      continue;
    }
    if (*p == 0x2a/* * */) {
      do_wildcard(p2, &p, &q);
    }
    if (*p) {
      *q = *p;
      p++,q++;
    }
    else
      break;
  }
  *q = 0;
  return t;
}

static void unbind_rule() {
  uint8_t* p = CMD_P;
  for (;*p!=0x0;p++) *p = 0;
}

////////////////////////////////////////////////////////


// args1 : Specify target string.
void* search_rule(void* p) {

  __os__write(1, "search\n", 7);
  uint64_t len = Confp->rules.num;// *RULES_P;
  uint8_t* cmd;
  uint8_t* suffix;
  uint8_t i=0;
  uint8_t* c = 0;
  rule* t = Confp->rules.first_rule;// RULES_P + 1;
  // If no default target is provided, treat it as to pick up the first one.
  if (p == 0) {
    if (resolve_deps(t)) {
      do_exec(t->cmd);
    }
    return t;
  }
  for (;i<len;i++,t++) {
    // resolve_vars(t);
    if (!strcmp(t->target, p)) {
      if (resolve_deps(t)) {
#ifndef DRYRUN
	do_exec(t->cmd);
#endif
	write_node(t->target, t->cmd->p);
	return t;
      } else {
	// __os__write(1, "skip\n", 5);
	return t;
      }
    }
  }
  return 0;
}


