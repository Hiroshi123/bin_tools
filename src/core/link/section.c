
#include <stdint.h>
#include <stdio.h>

#include "alloc.h"
#include "link.h"

extern Config* Confp;

SectionContainer* alloc_section_container_init(uint32_t va, void* name,
                                               void* candidate_list,
                                               ListContainer* Sc) {
  SectionContainer* sc = __malloc(sizeof(SectionContainer));
  sc->virtual_address = va;
  sc->name = name;
  sc->candidate_list = candidate_list;
  if (Sc) {
    if (Sc->init) {
      ((SectionContainer*)Sc->current)->next = sc;
    } else {
      Sc->init = sc;
    }
    Sc->current = sc;
  }
  return sc;
}

SectionContainer* __z__link__alloc_section_container_init(uint32_t va,
                                                          void* name,
                                                          void* candidate_list,
                                                          ListContainer* Sc) {
  return alloc_section_container_init(va, name, candidate_list, Sc);
}

SectionContainer* alloc_section_container(uint32_t va, void* name,
                                          void* candidate_list,
                                          ListContainer* Sc) {
  SectionContainer* sc =
      alloc_section_container_init(va, name, candidate_list, Sc);
  /* printf("init sectionc:%p\n", Confp->initial_section); */
  Confp->current_section->next = sc;
  Confp->current_section = sc;
  /* printf("init sectiond:%p\n", Confp->initial_section); */
  char max_name[100] = {};
  sprintf(max_name, "[link/section.c]\t alloc section container : %s\n", name);
  __z__logger_emit("misc.log", max_name);
  return sc;
}

SectionContainer* __z__link__alloc_section_container(uint32_t va, void* name,
                                                     void* candidate_list,
                                                     ListContainer* Sc) {
  return alloc_section_container(va, name, candidate_list, Sc);
}

void* alloc_section_chain(void* s, void* offset, SectionContainer* scon,
                          void* obj) {
  __z__logger_emit("misc.log", "[link/section.c]\t alloc section chain\n");

  SectionContainer* sec1;
  SectionChain* sec2;
  sec2 = __malloc(sizeof(SectionChain));
  sec2->p = s;
  sec2->data = offset;
  // sec2->data = obj ? obj + s->PointerToRawData : 0;// datap;
  // sec2->obj = obj;
  // Confp->current_section;
  sec2->obj = obj ? obj : Confp->current_object;
  if (scon->init) {
    scon->this->this = sec2;
    scon->this = sec2;
  } else {
    scon->init = sec2;
    scon->this = sec2;
  }
  if (Confp->current_object) {
    if (Confp->current_object->section_chain_head == 0) {
      Confp->current_object->section_chain_head = sec2;
    } else {
      Confp->current_object->section_chain_tail->next = sec2;
    }
    Confp->current_object->section_chain_tail = sec2;
  }
  /* TotalImageSize += (s->SizeOfRawData + SECTION_ALIGNMENT) & 0xFFFFF000; */
  return sec2;
}

void* __z__link__alloc_section_chain(void* s, void* offset,
                                     SectionContainer* scon, void* obj) {
  return alloc_section_chain(s, offset, scon, obj);
}

SectionContainer* match_section(char* name) {
  SectionContainer* sec1;
  for (sec1 = Confp->initial_section; sec1; sec1 = sec1->next) {
    if (sec1->name) {
      if (!strcmp(sec1->name, name)) {
        /* printf("matched section:%s\n", name); */
        return sec1;
      }
    }
  }
  return 0;
}

SectionContainer* __z__link__match_section(char* name) {
  return match_section(name);
}

// index is composed of container's index and chain index.
uint16_t get_section_index_by_name(char* name, void* q) {
  SectionContainer* sec1;
  SectionChain* schain;
  uint8_t i = 0;
  uint8_t j = 0;
  for (sec1 = Confp->initial_section; sec1; sec1 = sec1->next, i++) {
    if (sec1->name) {
      if (!strcmp(sec1->name, name)) {
        j = 0;
        for (schain = sec1->init; schain; schain = schain->this, j++) {
          if (schain->p == q) {
            return (i << 8) | j;
          }
        }
      }
    }
  }
  return 0;
}

uint16_t __z__link__get_section_index_by_name(char* name, void* q) {
  return get_section_index_by_name(name, q);
}

SectionChain* get_section_chain_by_index(uint16_t index) {
  uint8_t index1 = index >> 8;
  uint8_t index2 = 0xff & index;
  SectionContainer* sec1;
  SectionChain* schain;
  uint8_t i = 0;
  uint8_t j = 0;
  for (sec1 = Confp->initial_section; sec1; sec1 = sec1->next, i++) {
    // printf("get section index:%p,%p,%p,%p\n", i, j, index1, index2);
    j = 0;
    for (schain = sec1->init; schain; schain = schain->this, j++) {
      // printf("get section index!!:%p,%p,%p,%p\n", i, j, index1, index2);
      if (i == index1 && j == index2) {
        printf("match!\n");
        return schain;
      }
    }
  }
  return 0;
}

SectionChain* __z__link__get_section_chain_by_index(uint16_t index) {
  return get_section_chain_by_index(index);
}
