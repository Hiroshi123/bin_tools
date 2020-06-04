
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#ifdef __linux__
#include <dlfcn.h>
#include <fcntl.h>
#else
#include <windows.h>
#endif

#include "os.h"
#define ULONG_MAX 0xffffffff


#define MAX_BUF 100

static BUF[MAX_BUF] = {};

typedef struct {
  void* handle;
  char* filename;
} logfile;

static logfile MemoryLog = {
  .handle = 1,
  .filename = 1
};
static logfile MiscLog = {
  .handle = 1,
  .filename = 1
};

static void*/*HANDLE*/ LOG_HANDLE = 1;

static int _strcmp(char* s, char* t) {
  for (;*s && *t;s++,t++) {
    if (*s != *t) {
      return -1;
    }
  }
  return !(*s == *t);
}

static int _strlen(char* s) {
  int i=0;
  for (;*s;s++,i++);
  return i;
}

static const char xdigits[16] = {
  "0123456789ABCDEF"
};

static char *fmt_x(uintmax_t x, char *s, int lower)
{
  for (; x; x>>=4) *--s = xdigits[(x&15)]|lower;
  return s;
}

static char *_fmt_x(uintmax_t x, char *s, int lower, uint8_t max)
{
  uint8_t i = max-2;
  uint8_t m = 8;
  for (;  ; i-=2) {
    /* if (m == i) *s++ = ',';  */
    *s++ = xdigits[(x>>(4*(i+1)))&15]|lower;
    *s++ = xdigits[(x>>(4*i))&15]|lower;
    if (i == 0) break;
  }
  return s;
}

static char *fmt_mx(uintmax_t x, char *s, int lower, uint8_t max)
{
  int i = 0;
  for (; i < max; i+=2) {
    *s++ = xdigits[(x>>(4*(i+1)))&15]|lower;
    *s++ = xdigits[(x>>(4*i))&15]|lower;
  }
  return s;
}

/* static char *fmt_mx(uintmax_t x, char *s) */
/* { */
/*   unsigned long y; */
/*   for (   ; x>ULONG_MAX; x>>=4) *s++ = '0' + x%16; */
/*   for (y=x;           y; y>>=4) *s++ = '0' + y%16; */
/*   return s; */
/* } */

static char *fmt_u(uintmax_t x, char *s)
{
  unsigned long y;
  for (   ; x>ULONG_MAX; x/=10) *--s = '0' + x%10;
  for (y=x;           y; y/=10) *--s = '0' + y%10;
  return s;
}

static void _sprintf(char* buf, char* p, size_t** va_arg) {
  uint8_t* va = *va_arg;
  uint8_t tmp[8] = {};
  uint8_t* q;
  for (;*p;buf++,p++) {
    if (*p == '%') {
      p++;
      switch (*p) {
      case 's':
	if (va) {
	  for (;*va;buf++,va++) {
	    *buf = *va;
	  }
	}
	break;
      case 'x':
      case 'p':
	q = fmt_x(va, &tmp[7], 32);
	for (;q < &tmp[7];q++,buf++) *buf = *q;
	break;
      case 'd':
        q = fmt_u(va, &tmp[7]);
	for (;q < &tmp[7];q++,buf++) *buf = *q;
	break;
      default:
	break;

      va_arg++;
      }
      p++;
    }
    *buf = *p;
  }
  *buf = 0;
}

static void* check_handle(char* name) {

  void* handle = &LOG_HANDLE;
  if (!_strcmp(name, "memory.log")) {
    handle = &MemoryLog.handle;
  } else if (!_strcmp(name, "misc.log")) {
    handle = &MiscLog.handle;
  }
  return handle;
}

void logger_init(char* name) {

  size_t* handle = check_handle(name);
  /* void* lib = dlopen("lib/so/os.so.0", RTLD_LAZY); */
  /* void* f1 = dlsym(lib, "__open");   */
#ifdef __linux__
  *handle = __os__open(name, O_CREAT | O_WRONLY | O_TRUNC, 0777);
#else
  LOG_HANDLE = CreateFile
    (
     name , GENERIC_ALL/* | GENERIC_EXECUTE*/, 0, NULL,
     CREATE_ALWAYS, 0/*FILE_SHARE_READ*/, NULL
     );
#endif
}

void logger_emit(char* name, char* log) {
  size_t* handle = check_handle(name);
#ifdef __linux__
  __os__write(*handle, log, _strlen(log));
#else
  DWORD dsize = 0;
  WriteFile(LOG_HANDLE, log, strlen(log), &dsize, NULL);
#endif

}

/* void logger_emit_x(void* p) { */
/*   size_t* h = &MiscLog.handle; */
/*   __os__write(*h, "0x", 2); */
/*   uint8_t tmp[20] = {}; */
/*   tmp[15] = '\n'; */
/*   uint8_t* q = fmt_x(p, &tmp[15], 32); */
/*   __os__write(*h, q, _strlen(q)); */
/* } */

void logger_emit_p(void* p) {
  size_t* h = &MiscLog.handle;
  __os__write(*h, "0x", 2);
  uint8_t tmp[20] = {};
  uint8_t* q = _fmt_x(p, &tmp[0], 32, 16);
  *q = '\n';
  __os__write(*h, tmp, _strlen(tmp));
}

void logger_emit_m(void* p) {
  size_t* h = &MiscLog.handle;
  __os__write(*h, "0x", 2);
  uint8_t tmp[20] = {};
  uint8_t* q = fmt_mx(p, &tmp[0], 32, 16);
  *q = '\n';
  __os__write(*h, tmp, _strlen(tmp));
}

void logger_emit_d(void* p) {
  size_t* h = &MiscLog.handle;
  uint8_t tmp[17] = {};
  tmp[15] = '\n';
  uint8_t* q = fmt_u(p, &tmp[7]);
  __os__write(*h, q, _strlen(q));
}

void logger_emit3(char* name, char* log, size_t** va) {
  size_t* handle = check_handle(name);
  _sprintf(&BUF, log, va);
  __os__write(*handle, &BUF, _strlen(BUF));
}

