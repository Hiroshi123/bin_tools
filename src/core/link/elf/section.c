
#include <stdio.h>

#include "elf.h"
#include "link.h"
#include "alloc.h"

extern Config* Confp;

/*static */void iterate_section_container(void* callback_f) {
  
  SectionContainer* sec1 = Confp->initial_section;
  SectionChain* sec2;
  int previous_flags = 0;
  int size = 0;
  int same_flags = 0;
  for (/*sec1 = Confp->initial_section*/;sec1;sec1 = sec1->next) {
    callback_arg2_linux(sec1, callback_f);
  }
}


