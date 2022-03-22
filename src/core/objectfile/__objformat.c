
#include <fcntl.h>
#include <limits.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/stat.h>

#include "elf.h"
#include "macho.h"
#include "objformat.h"
#include "os.h"
#include "pe.h"

static enum OBJECT_FORMAT check_elf(const uint32_t fd, uint8_t* e,
                                    uint32_t* size) {
  if (e[EI_MAG0] == ELFMAG0 && e[EI_MAG1] == ELFMAG1 && e[EI_MAG2] == ELFMAG2 &&
      e[EI_MAG3] == ELFMAG3) {
    uint8_t class = 0;
    if (!__os__read(fd, &class, 1)) goto error;
    if (__os__lseek(fd, 0, SEEK_SET) == -1) goto error;
    if (class == 1) {
      Elf32_Ehdr ehdr;
      if (!__os__read(fd, &ehdr, sizeof(ehdr))) goto error;
      if (size)
        *size = /*ehdr.e_ehsize + */ ehdr.e_phoff +
                ehdr.e_phnum * sizeof(Elf32_Phdr);
      return ELF32;
    } else if (class == 2) {
      Elf64_Ehdr ehdr;
      if (!__os__read(fd, &ehdr, sizeof(ehdr))) goto error;
      if (size) *size = ehdr.e_phoff + ehdr.e_phnum * sizeof(Elf64_Phdr);
      return ELF64;
    }
  }
error:
  return 0;
}

static enum OBJECT_FORMAT check_macho(const uint32_t fd, uint8_t* _p,
                                      uint32_t* size) {
  const uint32_t* p = (uint32_t*)_p;
  // what you need to do is to grab the length of header which is
  if (*p == MH_MAGIC) {
    _mach_header mh;
    if (!__os__read(fd, (void*)&mh, sizeof(_mach_header))) {
    }  // fprintf(stderr,"file read error\n");
    *size = mh.sizeofcmds + sizeof(_mach_header);
    return MACHO32;
  } else if (*p == MH_MAGIC_64) {
    _mach_header_64 mh;
    if (!__os__read(fd, (void*)&mh, sizeof(_mach_header_64))) {
    }  // fprintf(stderr,"file read error\n");
    *size = mh.sizeofcmds + sizeof(_mach_header_64);
    return MACHO64;
  }
  return NONE;
}

static enum OBJECT_FORMAT check_pe(const uint32_t fd, const uint8_t* _p,
                                   uint32_t* size) {
  const uint16_t* p = (uint16_t*)_p;
  if (*p == 0x5a4d) {
    // lseek x bytes which are matched with e_lfanew.
    // currently, we are in 4th bytes from file head.
    // we need to retrieve 4bytes from 0x3c
    if (__os__lseek(fd, 0x3c - 0x04, SEEK_CUR) == -1) goto error;
    uint32_t e_lfanew;
    if (!__os__read(fd, &e_lfanew, sizeof(e_lfanew))) goto error;
    if (__os__lseek(fd, e_lfanew, SEEK_SET) == -1) goto error;
    uint32_t signature;
    if (!__os__read(fd, &signature, sizeof(signature))) goto error;
    if (signature != 0x4550) goto error;
    uint16_t machine_t;
    if (!__os__read(fd, &machine_t, sizeof(machine_t))) goto error;
    uint16_t number_of_sections;
    if (!__os__read(fd, &number_of_sections, sizeof(number_of_sections)))
      goto error;
    // get back to beginning
    __os__lseek(fd, 0, SEEK_SET);
    if (size) {
      uint32_t _size = 0x40 + e_lfanew + 0x18 +
                       number_of_sections * sizeof(IMAGE_SECTION_HEADER);
      if (machine_t == 0x86) {
        *size = _size + sizeof(IMAGE_NT_HEADERS32);
      } else if (machine_t == 0x8664) {
        *size = _size + sizeof(IMAGE_NT_HEADERS64);
        return PE64;
      } else {
        goto error;
      }
    }
    if (machine_t == 0x14c) {
      return PE32;
    }
    if (machine_t == 0x8664) {
      return PE64;
    }
  }
  return NONE;
error:
  // fprintf(stderr,"file read error..\n");
  return NONE;
}

static enum OBJECT_FORMAT check_coff(const uint32_t fd, const uint8_t* _p,
                                     uint32_t* size) {
  const uint16_t* p = (uint16_t*)_p;
  switch (*p) {
    case 0x8664:
      return COFF_AMD64;
    case 0x14c:
      return COFF_I386;
  }
  return 0;
}

enum OBJECT_FORMAT __z__obj__detect_format(const int fd, uint32_t* h_size) {
  uint8_t p[4] = {};
  __os__read(fd, &p, 4);
  /*  if (!read(fd, &p, 4)) fprintf(stderr,"file read error\n"); */
  enum OBJECT_FORMAT o;
  o = check_elf(fd, p, h_size);
  if (o) goto done;
  o = check_macho(fd, p, h_size);
  if (o) goto done;
  o = check_pe(fd, p, h_size);
  if (o) goto done;
  o = check_coff(fd, p, h_size);
done:
  return o;
}

static int A = 2;

static void __f3() {}

void ____f1() {
  // int fd = __os__open("test02.exe", O_RDONLY, 0777);
  int b = 2;
  __f3();
}

enum OBJECT_FORMAT __z__obj__detect_format_fname(char* fname,
                                                 uint32_t* h_size) {
  // for (;;);
  int fd = __os__open(fname, O_RDONLY, 0777);
  if (fd < 0) {
    return -1;
  }
  enum OBJECT_FORMAT format_type = __z__obj__detect_format(fd, h_size);
  __os__close(fd);
  return format_type;
}
