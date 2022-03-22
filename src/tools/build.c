
#include <stdint.h>
#include <stdio.h>
#include <windows.h>

#define SRC_FILE_MAX_LEN 16

// #define CONTEXT_RULE 0x2
#define CONTEXT_RULE_CMD 0x1
#define CONTEXT_RULE_TARGET 0x2
#define CONTEXT_VARIABLE 0x4
#define CONTEXT_READ_NOW 0x8

// #define DRYRUN 0

typedef struct {
  uint8_t* name;
  uint8_t* value;
} var;

typedef struct {
  uint8_t* target;
  uint8_t* deps;
  uint8_t* cmd;
} rule;

typedef struct {
  size_t addr;
  size_t size;
} heap;

typedef struct {
  size_t bind_full_str;
  size_t bind_str;
} _bind;

/* memory_list MEMORY_ROOT = {}; */

void* search_rule(void* p);
void* bind_cmd(rule* r);
uint8_t check_assign_var(uint8_t** _p, uint8_t** _q);

static size_t* VARS_P;
static size_t* RULES_P;
static _bind* BUF_P;
static size_t* CMD_P;
static size_t* HEAP_HEAD;
static heap* HEAP_CUR;

static HANDLE DotFileHandle;

__attribute__((constructor)) void set_heap_header() {
  printf("alloc\n");
  VARS_P = malloc(50 * sizeof(void*));
  RULES_P = malloc(50 * sizeof(void*));
  memset(RULES_P, 0, 50 * sizeof(void*));
  BUF_P = malloc(2 * sizeof(void*));
  BUF_P->bind_full_str = malloc(100);
  BUF_P->bind_str = malloc(30);

  CMD_P = malloc(30 * sizeof(void*));
  HEAP_HEAD = malloc(100 * sizeof(void*));
  HEAP_CUR = HEAP_HEAD;
  char fname[] = "deps.dot";
  DotFileHandle = CreateFile(fname, GENERIC_ALL /* | GENERIC_EXECUTE*/, 0, NULL,
                             CREATE_ALWAYS, 0 /*FILE_SHARE_READ*/, NULL);

  char str[] = "digraph graph_name {\n";
  DWORD bytesWritten;
  WriteFile(DotFileHandle, str, strlen(str), &bytesWritten, NULL);
}

__attribute__((destructor)) void dealloc() {
  printf("dealloc\n");
  free(VARS_P);
  free(RULES_P);
  free(BUF_P->bind_full_str);
  free(BUF_P->bind_str);
  free(BUF_P);
  free(CMD_P);
  heap* h = HEAP_HEAD;
  for (; h < HEAP_CUR; h++) free(h->addr);
  free(HEAP_HEAD);

  char str[] = "}";
  DWORD bytesWritten;
  WriteFile(DotFileHandle, str, strlen(str), &bytesWritten, NULL);
  CloseHandle(DotFileHandle);
}

void* alloc(int size) {
  void* h = malloc(size);
  HEAP_CUR->addr = h;
  HEAP_CUR->size = size;
  HEAP_CUR += 1;
  return h;
}

uint8_t lookup_vars(uint8_t* query, uint64_t* p) {
  uint64_t num = *p;
  var* v = p + 1;
  uint64_t i = 0;
  for (; i < num; i++, v++) {
    if (!strcmp(query, v->name)) {
      return i + 1;
      // return v;
    }
  }
  return 0;
}

uint8_t check_var(uint8_t** _p1, uint8_t** _t1, uint64_t* vars) {
  uint8_t* p1 = *_p1;
  uint8_t* t1 = *_t1;
  if (*(uint16_t*)p1 == 0x2824 /*$(*/) {
    void* p2 = p1 + 2;
    for (;; p1++) {
      if (*p1 == 0x29 /*)*/) {
        *p1 = 0;
        break;
      }
    }
    uint8_t index = lookup_vars(p2, vars);
    if (index) {
      // when a referred variable is used, this expression is expanded properly.
      *t1 = 0x24;
      t1++;
      *t1 = 0x28;
      t1++;
      *t1 = index;
      t1++;
      *t1 = 0x29;
      t1++;
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

void* retrieve(uint64_t** dest, uint8_t* p, uint8_t* p1, uint8_t var_check) {
  uint8_t* t;
  uint8_t* t1;
  t = t1 = alloc(1 + p - p1);
  // *dest = t1;
  uint8_t in_context = 0;
  for (; p1 < p; p1++) {
    if (!in_context && (*p1 == 0x20)) continue;
    in_context = 1;
    if (var_check) check_var(&p1, &t1, VARS_P);
    *t1 = *p1;
    t1++;
  }
  *t1 = 0;
  for (p1--, t1--; *p1 == 0x20; p1--, t1--) *t1 = 0;
  return t;
}

void* parse(uint8_t* p, uint8_t* e) {
  uint8_t* p1 = 0;
  uint8_t* t1;
  uint8_t context = 0;
  //
  uint64_t* vars = VARS_P;
  // VARS_P = vars;
  *vars = 0;
  var* a_var = vars + 1;
  uint64_t* rules = RULES_P;
  // RULES_P = rules;
  // number of rules should be recorded on top of all records.
  *rules = 0;
  uint64_t* r = rules;
  rule* a_rule = rules + 1;
  for (; p < e; p++) {
    // :=
    //  =
    // ?=
    if (*p == 0x20 && !(context & CONTEXT_READ_NOW)) continue;
    // if this is the end of line, you just have to know if this line was rule
    // or variables, if this is a rule, you must already know a target & set of
    // its dependents. after rule, you must have a command.
    if (*p == 0x0a) {
      // if you have a line, and the begining of line was recorded properly.
      if (p1 && p1 <= p) {
        // check_rule_cmd(context, p, p1, vars);
        if (context & CONTEXT_RULE_CMD) {
          a_rule->cmd = retrieve(&a_rule->cmd, p, p1, 1);
          p1 = 0;
          /////////////////////////////////////
          printf("rule:%d\n", *rules);
          printf("\ttarget:%s\n", a_rule->target);
          printf("\tdeps:%s\n", a_rule->deps);
          printf("\tcmd:%s\n", a_rule->cmd);
          a_rule++;
          *rules += 1;
          context &= ~CONTEXT_RULE_CMD;
          context &= ~CONTEXT_READ_NOW;
          continue;
        }
        if (context & CONTEXT_RULE_TARGET) {
          a_rule->deps = retrieve(&a_rule->deps, p, p1, 1);
          p1 = 0;
          context &= ~CONTEXT_RULE_TARGET;
          context |= CONTEXT_RULE_CMD;
          context &= ~CONTEXT_READ_NOW;
          continue;
        }
        ////////////////////////////////
        if (context & CONTEXT_VARIABLE) {
          a_var->value = retrieve(&a_var->value, p, p1, 1);
          p1 = 0;
          context &= ~CONTEXT_VARIABLE;
          context &= ~CONTEXT_READ_NOW;
          printf("var : %d\n", *vars);
          printf("\tname:%s\n", a_var->name);
          printf("\tvalue:%s\n", a_var->value);
          *vars += 1;
          a_var++;
          continue;
        }
      }
      continue;
    }
    // check_simple_assign(p, p1, );
    if (*(uint16_t*)p == 0x3d3a) {
      a_var->name = retrieve(&a_var->name, p, p1, 0);
      p1 = p + 2;
      context |= CONTEXT_VARIABLE;
      context &= ~CONTEXT_READ_NOW;
      continue;
      // :
    } else if (*p == 0x3a) {
      a_rule->target = retrieve(&a_rule->target, p, p1, 1);
      context |= CONTEXT_RULE_TARGET;
      context &= ~CONTEXT_READ_NOW;
      p1 = p + 1;  // p+1;
      continue;
    } /* else if (*p == 0x2a/\* $* *\/) { */
    /* uint8_t* p1 = _strtok(p); */
    /* if (p1) { */
    /* 	do_wildcard(p); */
    /* 	if (p1 == 1) {	   */
    /* 	} else { */
    /* 	  *(p-1) = 0x20; */
    /* 	} */
    /* } */
    if (!p1) {
      p1 = p;
      context |= CONTEXT_READ_NOW;
    }
  }
  return r;
}

void* _strtok(uint8_t* p) {
  uint8_t has = 0;
  for (;; p++) {
    if (*p == 0x20) {
      *p = 0x0;
      return p + 1;
    }
    if (*p == 0x0) {
      return has;
    }
    has = 1;
  }
  return 0;
}

uint8_t isFile(void* p) {
  HANDLE hFile =
      CreateFile(p, GENERIC_READ /* | GENERIC_EXECUTE*/, 0, NULL,
                 OPEN_EXISTING /*CREATE_NEW*/, FILE_SHARE_READ, NULL);
  if (hFile == -1) return 0;
  CloseHandle(hFile);
  return 1;
}

void write_to_dot_file(void* str) {
  // char str[] = " alpha;\n";
  // "Example text testing WriteFile";
  DWORD bytesWritten;
  WriteFile(DotFileHandle, str, strlen(str), &bytesWritten, NULL);
}

void write_node(void* p1, void* p2) {
  write_to_dot_file(p1);
  write_to_dot_file(" [label = \"target : ");
  write_to_dot_file(p1);
  write_to_dot_file("\n");
  write_to_dot_file("command : ");
  write_to_dot_file(p2);
  write_to_dot_file("\n");
  write_to_dot_file("\"]\n");
}

void write_edge(void* p1, void* p2) {
  // construct node -> node;
  write_to_dot_file(p1);
  write_to_dot_file(" -> ");
  write_to_dot_file(p2);
  write_to_dot_file(";\n");
}

void* has_suffix(uint8_t* p, uint8_t** last) {
  // if the rule contains "%"
  uint8_t count = 0;
  uint8_t i = 0;
  for (; *p != 0x20 && *p != 0x00; p++, i++) {
    // if (!begin) printf("!%c", *p);
    if (*p == 0x25) {
      count = i;
      if (!last) return p + 1;
    }
  }
  printf("\n");
  if (count) *last = count;
  if (count) printf("count:%d\n", count);
  return (count) ? p : 0;
}

void* bind_suffix(uint8_t* suffix, uint8_t* p, uint8_t c) {
  uint8_t* p1;
  uint8_t* p2;
  uint8_t* p3;
  p1 = p2 = p;
  uint8_t* s1 = suffix;
  // go to last
  for (; *p1 != 0x0; p1++)
    ;
  for (; *suffix != 0x25 /*%*/; p1--, suffix--) {
    if (*p1 != *suffix) {
      suffix = 0;
      break;
    }
  }
  if (suffix) {
    p3 = BUF_P->bind_str;
    for (p2 += c; p2 <= p1; p2++, p3++) *p3 = *p2;
    *p3 = 0x0;
    return 1;
  }
  return 0;
}

void clear_buf() {
  uint8_t* p = BUF_P->bind_full_str;
  for (; *p != 0x0; p++) {
    *p = 0;
  }
}

void unbind_suffix() {
  clear_buf();
  uint8_t* p = BUF_P->bind_str;
  for (; *p != 0x0; p++) {
    *p = 0;
  }
}

void* resolve_var_suffix(uint8_t* j) {
  uint8_t* s = j;
  uint8_t* p = BUF_P->bind_str;
  if (!j) return p;
  uint8_t* k = BUF_P->bind_full_str;
  for (; *j != 0x20 && *j != 0x0; k++, j++) {
    check_assign_var(&j, &k);
    if (*j == 0x2a /* * */) {
      do_wildcard(s, &j, &k);
    }
    if (*j == 0x25 /* % */) {
      for (; *k = *p; k++, p++)
        ;
      k--;
    } else {
      *k = *j;
    }
  }
  *k = 0x0;
  return BUF_P->bind_full_str;
}

void resolve_deps(rule* r) {
  uint8_t* p1 = r->deps;
  uint8_t* p;
  rule* v;
  for (;;) {
    p = _strtok(p1);
    printf("look for %s\n", p1);
    if (!p) break;
    // uint8_t* j =
    if (has_suffix(p1, 0)) p1 = resolve_var_suffix(p1);
    write_edge(r->target, p1);
    if (isFile(p1)) {
      printf("edge,%s\n", p1);
    } else {
      printf("search rule\n");
      v = search_rule(p1);
      if (v) {
        printf("found a rule : %x,target : %s, cmd : %s\n", p1, v->target,
               v->cmd);
      } else {
        printf("no rule found\n");
      }
    }
    if (p == 1) {
      printf("break\n");
      break;
    }
    p1 = p;
  }
  printf("resolve deps done\n");
}

void* convert_path_slash(uint8_t* p) {
  for (; *p != 0; p++) {
    if (*p == '/') *p = '\\';
  }
}

void do_exec(void* cmd) {
  PROCESS_INFORMATION pi;
  STARTUPINFO si = {sizeof(si)};
  // char _cmd[] = "nasm.exe -I./include/ -f win64 ./src/core/asm/x64/common.asm
  // -o ./obj/common.o";
  if (CreateProcessA(0, cmd, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi)) {
    printf("ok\n");
  } else {
    printf("create process error\n");
    return;
  }
  WaitForSingleObject(pi.hProcess, INFINITE);
}

void* search_rule(void* p) {
  uint64_t len = *RULES_P;
  uint8_t* cmd;
  uint8_t* suffix;
  uint8_t i = 0;
  uint8_t* c = 0;
  rule* t = RULES_P + 1;
  for (; i < len; i++, t++) {
    if (!strcmp(t->target, p)) {
      printf("target:%s\n", t->target);
      printf("deps:%s\n", t->deps);
      resolve_deps(t);
      cmd = bind_cmd(t);
      printf("do exec:%s\n", cmd);
#ifndef DRYRUN
      do_exec(cmd);
#endif
      write_node(t->target, cmd);
      unbind_rule();
      printf("ok.\n");
      return t;
    } else {
      suffix = has_suffix(t->target, &c);
      if (suffix && bind_suffix(suffix, p, c)) {
        resolve_deps(t);
        cmd = bind_cmd(t);
        printf("!do exec:%s\n", cmd);
        // convert_path_slash(cmd);
#ifndef DRYRUN
        do_exec(cmd);
#endif
        unbind_rule();
        unbind_suffix(suffix);
        return t;
      }
    }
  }
  return 0;
}

void* get_fullpath(void* r) {}

void* _do_wildcard(void* path, void* hFind, uint8_t** _p) {
  WIN32_FIND_DATA fileInfo;
  printf("path:%s\n", path);
  // fileInfo.cFileName
  uint8_t alc = 0;
  uint8_t* p = *_p;
  uint8_t* p1;
  uint8_t* p2;
  void* ret;
  if (!hFind)
    ret = FindFirstFile(path, &fileInfo);
  else {
    ret = FindNextFile(hFind, &fileInfo);
    // add path on second time.
    if (ret)
      for (p2 = path; *p2 != 0x0 && *p2 != 0x2a; p2++, p++) *p = *p2;
  }
  if (ret) {
    for (p1 = &fileInfo.cFileName; *p = *p1; p++, p1++)
      ;
    *p = 0x20;
    *_p = p += 1;
    printf("w:%d,%s\n", strlen(fileInfo.cFileName), fileInfo.cFileName);
  } else {
    FindClose(hFind);
  }
  return ret;
}

void do_wildcard(uint8_t* s, uint8_t** _p, uint8_t** _q) {
  uint8_t* p = *_p;
  uint8_t* q = *_q;
  uint8_t* p1;
  if (p1 = _strtok(s)) {
    // void* len = do_wildcard(p2, 0);
    void* h = _do_wildcard(s, 0, &q);
    for (; _do_wildcard(s, h, &q);)
      ;
    for (; *p != 0x0 && *p != 0x20; p++)
      ;
    if (p1 == 1) {
    } else {
      *(p - 1) = 0x20;
    }
  }
  *_p = p;
  *_q = q;
}

void* assign_var(uint8_t* p, uint8_t* q) {
  var* v = VARS_P + 1;
  // retrieve index value
  v += (*(p + 2) - 1);
  uint8_t* s;
  for (s = v->value; *q = *s; q++, s++)
    ;
  return q;
}

uint8_t check_assign_var(uint8_t** _p, uint8_t** _q) {
  uint8_t* p = *_p;
  uint8_t* q = *_q;
  if (*(uint16_t*)p == 0x2824 /*$*/) {
    q = assign_var(p, q);
    p += 4;
  } else
    return 0;
  *_p = p;
  *_q = q;
  return 1;
}

void* bind_deps(uint8_t* deps, uint8_t* q) {
  uint8_t* s = has_suffix(deps, 0) ? resolve_var_suffix(deps) : deps;
  for (; *q = *s; q++, s++)
    ;
  return q;
}

void* bind_cmd(rule* r) {
  uint8_t* p = r->cmd;
  uint8_t* q = CMD_P;
  uint8_t* t = q;
  uint8_t* s;
  uint8_t begin = 0;
  uint8_t* p2;
  for (; *p != 0;) {
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
      p += 2;
      // stpcpy
      for (s = resolve_var_suffix(r->deps); *q = *s; q++, s++)
        ;
      continue;
    }
    // target($@)
    if (*(uint16_t*)p == 0x4024) {
      p += 2;
      s = resolve_var_suffix(r->target);
      for (; *q = *s; q++, s++)
        ;
      continue;
    }
    if (*p == 0x2a /* * */) {
      do_wildcard(p2, &p, &q);
    }
    if (*p) {
      *q = *p;
      p++, q++;
    } else
      break;
  }
  *q = 0;
  return t;
}

void unbind_rule() {
  uint8_t* p = CMD_P;
  for (; *p != 0x0; p++) *p = 0;
}

int main(int argc, char** argv) {
  if (argc < 3) {
    printf("usage\n");
    printf("almost equivalent GNU make\n");
    printf("1st : specify a build file\n");
    printf("2nd : specify a target name\n");
    return 0;
  } else {
    HANDLE hFile =
        CreateFile(argv[1], GENERIC_ALL /* | GENERIC_EXECUTE*/, 0, NULL,
                   OPEN_EXISTING /*CREATE_NEW*/, 0 /*FILE_SHARE_READ*/, NULL);
    DWORD wReadSize;
    int size = GetFileSize(hFile, NULL);
    uint8_t* p = alloc(size);
    ReadFile(hFile, p, size, &wReadSize, NULL);

    uint64_t* t = parse(p, p + wReadSize);
    rule* v = search_rule(argv[2]);
    CloseHandle(hFile);
    printf("done\n");
    return 0;
  }
}
