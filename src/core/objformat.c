
#include <stdio.h>
#include <stdint.h>
#include "objformat.h"
#include "macho.h"
#include "elf.h"
/* #include "coff.h" */
#include "pe.h"


enum OBJECT_FORMAT detect_format(const char* p) {  
  if (check_elf(p)) {
    return ELF;
  } else if (check_macho(p)) {
    return MACHO;
  } else if (check_pe(p)) {
    return PE;
  } else if (check_coff(p)) {
    return COFF;
  }
  return NONE;
}

