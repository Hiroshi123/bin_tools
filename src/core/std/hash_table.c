
#include <stdio.h>
#include <stdint.h>
#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>

#include "os.h"
#include "logger.h"
#include "alloc.h"
#include "hash_table.h"
#include "string.h"

// static hash_table* hash_table_p;

/*static */
uint32_t __z__std__sysv_hash(const char *s0) {
  const unsigned char *s = (void *)s0;
  uint_fast32_t h = 0;
  while (*s) {
    h = 16*h + *s++;
    h ^= h>>24 & 0xf0;
  }
  return h & 0xfffffff;
}

uint32_t __z__std__with_escape_hash(const char* s0, uint8_t* _param) {

  uint8_t rev = 0;
  uint8_t num = 0;
  if (_param) {
    rev = *_param;
    num = *(_param + 1);
  }
  const unsigned char *s = (void *)s0;
  uint_fast32_t h = 0;
  while (*s) {
    if (*s != '%')
      h += *s;
    if (rev) s--;
    else s++;
    if (num > 0) {
      num--;
      if (!num) break;
    }
  }
  logger_emit_p(0x888);
  logger_emit_p(h);
  return h & 0xfffffff;
}

void __z__std__init_hash_table(void* p) {
  // hash_table_p = p;
}

static uint8_t strncmp(char* p1, char* p2, int i) {
  for (;i && *p1 && *p2 && *p1 == *p2;p1++,p2++, i--);
  if (i == 0) return 0;
  return *p1 - *p2;
}

static uint8_t rstrncmp(char* p1, char* p2, int i) {
  for (;i && *p1 && *p2 && *p1 == *p2;p1--,p2--, i--);
  return *p1 - *p2;
}


char* __z__std__hash_partial_find
(hash_table* hash_table_p, char* key, void* _l) {

  // hash_table_p->->rules.implicit_pre_suffix_len_p
  list* list = _l;
  struct _implicit_trim_len* trim_p = list->p;
  __os__write(1, key, strlen(key));
  __os__write(1, "\n", 1);
  logger_emit_p(trim_p->begin);
  logger_emit_p(trim_p->end);
  uint32_t ss = 0;
  uint8_t param[2] = {};
  if (trim_p->begin && strlen(key) > trim_p->begin) {
    param[0] = 0;
    param[1] = trim_p->begin;
    ss = hash_table_p->hash_f(key, &param);
  }
  if (trim_p->end && strlen(key) > trim_p->end) {
    param[0] = 1;
    param[1] = trim_p->end;
    char* last = key;
    for (;*last;last++);
    ss += hash_table_p->hash_f(last-1, &param);
  }
  logger_emit_p(strlen(key));
  logger_emit_p(ss);
  logger_emit_p(ss);

  // logger_emit_p(trim_p->end);
  size_t* s = hash_table_p->bucket +
    (ss % hash_table_p->nbucket);

  int begin_trim = 0;
  int end_trim = 0;
  if (*s == 0) {
    __os__write(1, "noent\n", 6);
    return 0;
  }
  __os__write(1, "ok!\n", 4);
  chain* c;
  uint8_t r;
  for (c = *s;c;c=c->next) {
    r = strncmp(c->key, key, trim_p->begin);
    if (!r) {
      __os__write(1, "good\n", 5);
      return c->value;
    }

    /* r = strcmp(c->key, key); */
    /* if (!r) { */
    /*   return c->value; */
    /* } */
    /* if (r == 0x25/\*%*\/) { */
    /*   __os__write(1, "i\n", 2); */
    /*   return c->value; */
    /* } */
  }
  return 0;
}

char* __z__std__hash_find(hash_table* hash_table_p, char* key) {

  __os__write(1, "noen!\n", 6);
  size_t* s = 0;
  if (hash_table_p->hash_f == 0) {
    s = hash_table_p->bucket +
      (__z__std__sysv_hash(key) % hash_table_p->nbucket);
  } else {
    s = hash_table_p->bucket +
      (hash_table_p->hash_f(key, 0) % hash_table_p->nbucket);
  }
  if (*s == 0) {
    __os__write(1, "noenh\n", 6);
    return 0;
  }
  chain* c;
  uint8_t r;
  for (c = *s;c;c=c->next) {
    r = strcmp(c->key, key);
    if (!r) {
      return c->value;
    }
    if (r == 0x25/*%*/) {
      __os__write(1, "i\n", 2);
      return c->value;
    }
  }
  return 0;
}

void __z__std__hash_set(hash_table* hash_table_p, char* key, char* value, int mode) {

  size_t* s = 0;
  if (hash_table_p->hash_f == 0) {
    s = hash_table_p->bucket +
      (__z__std__sysv_hash(key) % hash_table_p->nbucket);
  } else {
    s = hash_table_p->bucket +
      (hash_table_p->hash_f(key, 0) % hash_table_p->nbucket);
  }
  chain* c;
  chain* new_c = __malloc(sizeof(chain));
  new_c->key = key;
  new_c->value = value;
  new_c->next = 0;
  if (*s == 0) {
    logger_emit_p(hash_table_p->hash_f(key, 0));
    *s = new_c;
    return;
  }
  char* ss;
  for (c = *s;;c=c->next) {
    if (mode != COL_NOSTRCMP && !strcmp(c->key, key)) {
      // this mode updates only value.
      if (mode == COL_OVERRIDE) {
	c->value = new_c;
	return;
      }
      if (mode == COL_ADD) {
	char* str = __malloc(strlen(c->value) + strlen(value) + 1);
	char* tt = str;
	for (ss = c->value;*str = *ss;ss++,str++);
	for (ss = value;*str = *ss;ss++,str++);
	__os__write(1, tt, strlen(tt));
	return;
      }
    }
    if (c->next == 0) break;
  }
  c->next = new_c;
  return;
}

