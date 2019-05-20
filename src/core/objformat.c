
#include <stdio.h>
#include <stdint.h>
#include "objformat.h"
#include "macho.h"
#include "elf.h"
/* #include "coff.h" */
#include "pe.h"

enum OBJECT_FORMAT check_elf(const uint32_t fd, uint8_t* e, uint32_t* size) {
  if (e[EI_MAG0] == ELFMAG0 && e[EI_MAG1] == ELFMAG1 &&
      e[EI_MAG2] == ELFMAG2 && e[EI_MAG3] == ELFMAG3) {
    uint8_t class;
    if (!read(fd, &class, 1)) goto error;
    if (lseek(fd,0,SEEK_SET) == -1) goto error;
    if (class == 1) {
      Elf32_Ehdr ehdr;
      if (!read(fd, &ehdr, sizeof(ehdr))) goto error;
      *size = /*ehdr.e_ehsize + */ehdr.e_phoff + ehdr.e_phnum * sizeof(Elf32_Phdr);
      return ELF32;
    }
    else if (class == 2) {
      Elf64_Ehdr ehdr;
      if (!read(fd, &ehdr, sizeof(ehdr))) goto error;
      *size = ehdr.e_phoff + ehdr.e_phnum * sizeof(Elf64_Phdr);
      return ELF64;
    }
  }
  return NONE;
 error:
  fprintf(stderr,"file read error\n");
  return NONE;
}

enum OBJECT_FORMAT check_macho(const uint32_t fd, const uint32_t* p, uint32_t* size) {
  // what you need to do is to grab the length of header which is
  if (*p == MH_MAGIC) {
    _mach_header mh;
    if (!read(fd, mh, sizeof(_mach_header))) fprintf(stderr,"file read error\n");      
    *size = mh.sizeofcmds + sizeof(_mach_header);
    return MACHO32;
  } else if (*p == MH_MAGIC_64) {
    _mach_header_64 mh;
    if (!read(fd, mh, sizeof(_mach_header_64))) fprintf(stderr,"file read error\n");      
    *size = mh.sizeofcmds + sizeof(_mach_header_64);
    return MACHO64;
  }
  return NONE;
}

enum OBJECT_FORMAT check_pe(const uint32_t fd, const uint16_t* p, uint32_t* size) {
  if (*p == 0x5a4d) {
    // lseek x bytes which are matched with e_lfanew.
    // currently, we are in 4th bytes from file head.
    // we need to retrieve 4bytes from 0x3c
    if (lseek(fd,0x3c - 0x04,SEEK_CUR) == -1) goto error;
    uint32_t e_lfanew;
    if (!read(fd, &e_lfanew, sizeof(e_lfanew))) goto error;
    if (lseek(fd,e_lfanew,SEEK_SET) == -1) goto error;
    uint32_t signature;
    if (!read(fd, &signature, sizeof(signature))) goto error;
    if (signature != 0x4550) goto error;
    uint16_t machine_t;
    if (!read(fd, &machine_t, sizeof(machine_t))) goto error;
    uint16_t number_of_sections;
    if (!read(fd, &number_of_sections, sizeof(number_of_sections))) goto error;
    uint32_t _size = 0x40 + e_lfanew + 0x18 + number_of_sections * sizeof(IMAGE_SECTION_HEADER);
    printf("sig:%x\n",machine_t);
    if (machine_t == 0x86) {
      *size = _size + sizeof(IMAGE_NT_HEADERS32);
      return PE32;
    } else if (machine_t == 0x8664) {
      *size = _size + sizeof(IMAGE_NT_HEADERS64);
      return PE64;
    } else {
      goto error;
    }
  }
  return NONE;
 error:
  fprintf(stderr,"file read error..\n");
  return NONE;
}

enum OBJECT_FORMAT detect_format(const int fd, uint32_t* h_size) {
  uint8_t p[4] = {};
  if (!read(fd, &p, 4)) fprintf(stderr,"file read error\n");
  enum OBJECT_FORMAT o;
  o = check_elf(fd, p, h_size);
  if (o) goto done;
  o = check_macho(fd, p, h_size);
  if (o) goto done;
  o = check_pe(fd, p, h_size);
  if (o) goto done;
  /* o = check_coff(p, h_size); */

 done:
  return o;
  
}
