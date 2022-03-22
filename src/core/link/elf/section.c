
#include <stdio.h>

#include "alloc.h"
#include "elf.h"
#include "link.h"

extern Config* Confp;

/*static */ void iterate_section_container(void* callback_f) {
  SectionContainer* sec1 = Confp->initial_section;
  SectionChain* sec2;
  int previous_flags = 0;
  int size = 0;
  int same_flags = 0;
  for (/*sec1 = Confp->initial_section*/; sec1; sec1 = sec1->next) {
    callback_arg2_linux(sec1, callback_f);
  }
}

void __z__link__iterate_section_container(void* callback_f) {
  return iterate_section_container(callback_f);
}

Elf64_Shdr* __z__link__alloc_elf_section(int type, int flags) {
  Elf64_Shdr* shdr = __malloc(sizeof(Elf64_Shdr));
  shdr->sh_type = type;
  shdr->sh_name = 0;  // ".dynamic";
  shdr->sh_flags = flags;
  shdr->sh_addr = 0;
  shdr->sh_size = 0;  // size;
  shdr->sh_link = 5;
  shdr->sh_info = 0;
  shdr->sh_addralign = 0;
  shdr->sh_entsize = 0;
  return shdr;
}
