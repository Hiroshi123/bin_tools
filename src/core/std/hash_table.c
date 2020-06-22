
#include <stdio.h>
#include <stdint.h>
#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>

#include "os.h"
#include "alloc.h"
#include "hash_table.h"
#include "string.h"

static hash_table* hash_table_p;

static uint32_t __z__sysv_hash(const char *s0) {
  const unsigned char *s = (void *)s0;
  uint_fast32_t h = 0;
  while (*s) {
    h = 16*h + *s++;
    h ^= h>>24 & 0xf0;
  }
  return h & 0xfffffff;
}

void __z__std__init_hash_table(void* p) {
  hash_table_p = p;
}

char* __z__std__hash_find(char* key) {
  size_t* s = hash_table_p->bucket +
    (__z__sysv_hash(key) % hash_table_p->nbucket);
  if (*s == 0) {
    return 0;
  }
  chain* c;
  for (c = *s;;c=c->next) {
    if (!strcmp(c->key, key)) {
      return c->value;
    }
  }
  return 0;
}

void __z__std__hash_set(char* key, char* value, int mode) {

  size_t* s = hash_table_p->bucket +
    (__z__sysv_hash(key) % hash_table_p->nbucket);
  chain* c;
  chain* new_c = __malloc(sizeof(chain));
  new_c->key = key;
  new_c->value = value;
  new_c->next = 0;
  if (*s == 0) {
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

