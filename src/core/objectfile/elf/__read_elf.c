
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>

#include "elf.h"
#include "link.h"
#include "logger.h"
#include "memory.h"
#include "objformat.h"
#include "types.h"

/* void* read_elf(char* base) { */
/*   Elf64_Ehdr* p = base; */
/*   logger_emit_p(p); */
/*   logger_emit_p(p->e_phnum); */
/*   void* r = p->e_entry - 0x400000; */
/*   uint64_t* e = base + (size_t)r; */
/*   logger_emit_p(e); */
/*   uint8_t i = *(uint8_t*)e; */
/*   logger_emit_m(*e); */
/*   return (void*)e; */
/* } */

static void f1() {}

char* __z__obj__load_elf(char* name, void* out) {
  void* scr = __z__mem__alloc_file(name);

  /* int fd = __os__open(name, O_RDONLY, 0777); */
  /* if (fd == -1) { */
  /*   char str[] = "loadlibrary error\n"; */
  /*   __os__write(1, str, sizeof(str)); */
  /* } */
  /* enum OBJECT_FORMAT format = __z__obj__detect_format(fd, 0); */
  /* if (format != ELF32 && format != ELF64) { */
  /*   __os__write(1, "error\n", 6); */
  /*   return 0; */
  /* } */
  /* void* scr = __os__mmap */
  /*   (NULL, 0x1000 */
  /*    /\*PAGE_SIZE*\/, PROT_READ | PROT_WRITE | PROT_EXEC, */
  /*    MAP_PRIVATE, fd, 0); */

  ObjectChain* ocp = 0;
  run_through_elf_phdr3(scr, &f1, &ocp);

  // run_through_elf_shdr3(scr, &f1, &ocp);
  for (;;)
    ;
  run_through_elf_symtable3(
      ocp->symbol_table_p, ((Elf64_Sym*)ocp->symbol_table_p) + ocp->symbol_num,
      &__z__link__on_elf_symtab_callback_for_link, ocp->str_table_p);
}
