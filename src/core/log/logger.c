
#include <windows.h>
#include <stdio.h>
#include <stdint.h>

static HANDLE LOG_HANDLE = 0;

void logger_init(char* name) {

  LOG_HANDLE = CreateFile
    (
     "log.txt" , GENERIC_ALL/* | GENERIC_EXECUTE*/, 0, NULL,
     CREATE_ALWAYS, 0/*FILE_SHARE_READ*/, NULL
     );
}

void logger_emit(char* log) {
  DWORD dsize = 0;
  WriteFile(LOG_HANDLE, log, strlen(log), &dsize, NULL);
}


