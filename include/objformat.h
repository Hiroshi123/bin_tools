
#include "types.h"

enum OBJECT_FORMAT {
  NONE,
  ELF32,
  ELF64,
  MACHO32,
  MACHO64,
  PE32,
  PE64,  
  COFF,
};

p_host check_fname(void* meta, p_guest f_addr, enum OBJECT_FORMAT o);
// enum OBJECT_FORMAT detect_format(const int p);
