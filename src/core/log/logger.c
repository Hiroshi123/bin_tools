
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

static void*/*HANDLE*/ LOG_HANDLE = 0;

void logger_init(char* name) {
  printf("fopen\n");

  /* void* lib = dlopen("lib/so/os.so.0", RTLD_LAZY); */
  /* void* f1 = dlsym(lib, "__open");   */
#ifdef __linux__
  LOG_HANDLE = open__("log.txt", O_WRONLY | O_TRUNC);
#else
  LOG_HANDLE = CreateFile
    (
     "log.txt" , GENERIC_ALL/* | GENERIC_EXECUTE*/, 0, NULL,
     CREATE_ALWAYS, 0/*FILE_SHARE_READ*/, NULL
     );
#endif
}

void logger_emit(char* log) {

#ifdef __linux__
  __os__write("log.txt", log, strlen(log));
#else
  DWORD dsize = 0;
  WriteFile(LOG_HANDLE, log, strlen(log), &dsize, NULL);
#endif

}


