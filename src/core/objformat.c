
#include <stdio.h>
#include <stdint.h>
#include "objformat.h"

enum OBJECT_FORMAT detect_format(const char* p) {  
  if (check_elf(p)) {
    return ELF;
  } else if (check_macho(p)) {
    return MACHO;
  } else if (check_pe(p)) {
    return PE;
  }
  return NONE;
}

