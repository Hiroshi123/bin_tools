
#include <stdint.h>
#include "types.h"

enum OBJECT_FORMAT {
  NONE,
  ELF32,
  ELF64,
  MACHO32,
  MACHO64,
  PE32,
  PE64,
  COFF_I386,
  COFF_AMD64,
};

p_host check_fname(void* meta, p_guest f_addr, enum OBJECT_FORMAT o);
// enum OBJECT_FORMAT detect_format(const int p);

enum OBJECT_FORMAT __z__obj__detect_format(int fd, uint32_t* header_size);
enum OBJECT_FORMAT __z__obj__detect_format_fname(char* fname, uint32_t* header_size);

#define detect_format __z__obj__detect_format

