
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

typedef struct {
  void* handle;
  char* filename;
} logfile;

static logfile MemoryLog = {};
static logfile MiscLog = {};

static void*/*HANDLE*/ LOG_HANDLE = 0;

void* check_handle(char* name) {

  void* handle = &LOG_HANDLE;
  if (!strcmp(name, "memory.log")) {
    handle = &MemoryLog.handle;
  } else if (!strcmp(name, "misc.log")) {
    handle = &MiscLog.handle;
  }
  return handle;
}

void logger_init(char* name) {

  size_t* handle = check_handle(name);
  /* void* lib = dlopen("lib/so/os.so.0", RTLD_LAZY); */
  /* void* f1 = dlsym(lib, "__open");   */
#ifdef __linux__
  *handle = open__(name, O_CREAT | O_WRONLY | O_TRUNC);
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
  __os__write(*handle, log, strlen(log));
#else
  DWORD dsize = 0;
  WriteFile(LOG_HANDLE, log, strlen(log), &dsize, NULL);
#endif

}


