
#include <stdio.h>

#include "elf.h"
#include "link.h"
#include "alloc.h"

extern SectionChain* InitialSection;
extern Config* Confp;

/*static */void iterate_section_container(void* callback_f) {
  
  SectionContainer* sec1;
  SectionChain* sec2;
  Elf64_Shdr* shdr;
  int previous_flags = 0;
  int size = 0;
  int same_flags = 0;
  for (sec1 = Confp->initial_section;sec1;sec1 = sec1->next) {
    /* printf("s:%p,%p,%p\n", sec1, sec1->this, sec1->next); */
    printf("ite\n");
    callback_arg2_linux(sec1, callback_f);
  }
}


