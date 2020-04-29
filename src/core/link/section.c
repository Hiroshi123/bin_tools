
#include <stdio.h>
#include <stdint.h>
#include "alloc.h"

#include "link.h"

uint8_t TotalSectionNum = 0;
uint32_t TotalHeaderSize = 0;
/* extern SectionContainer* InitialSection; */
// extern SectionChain* InitialSection;
/* extern SectionChain* CurrentSection; */
extern uint32_t TotalImageSize;
extern ObjectChain* CurrentObject;
extern uint8_t _Win32;

/* SectionContainer* InitialSection = 0; */
/* SectionContainer* CurrentSection = 0; */

extern ObjectChain* InitialObject;// = 0;
extern ObjectChain* CurrentObject;// = &InitialObject;

/* ObjectChain* InitialObject = 0; */
/* ObjectChain* CurrentObject = &InitialObject; */

extern Config* Confp;

SectionContainer* alloc_section_container_init
(uint32_t va, void* name, void* candidate_list, ListContainer* Sc) {
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

SectionContainer* alloc_section_container
  (uint32_t va, void* name, void* candidate_list, ListContainer* Sc) {
  SectionContainer* sc = alloc_section_container_init(va, name, candidate_list, Sc);
  Confp->current_section->next = sc;
  Confp->current_section = sc;
  char max_name[100] = {};
  sprintf(max_name, "[link/section.c]\t alloc section container : %s\n", name);
  logger_emit("misc.log", max_name);
  return sc;
}

void* alloc_section_chain(void* s, void* offset, SectionContainer* scon) {
  
  logger_emit("misc.log", "[link/section.c]\t alloc section chain\n");
  SectionContainer* sec1;
  SectionChain* sec2;
  sec2 = __malloc(sizeof(SectionChain));
  sec2->p = s;
  sec2->data = offset;// datap;
  // sec2->data = obj ? obj + s->PointerToRawData : 0;// datap;
  sec2->obj = CurrentObject;
  // Confp->current_section;
  // sec2->obj = obj ? CurrentObject : 0;
  if (scon->init) {
    scon->this->this = sec2;
    scon->this = sec2;
  } else {
    scon->init = sec2;
    scon->this = sec2;
  }
  /* if (CurrentObject->section_chain_head == 0) { */
  /*   CurrentObject->section_chain_head = sec2; */
  /* } else { */
  /*   CurrentObject->section_chain_tail->next = sec2; */
  /* } */
  /* CurrentObject->section_chain_tail = sec2; */
  /* TotalImageSize += (s->SizeOfRawData + SECTION_ALIGNMENT) & 0xFFFFF000; */
  return sec2;
}

SectionContainer* match_section(char* name) {
  
  SectionContainer* sec1;  
  for (sec1 = Confp->initial_section;sec1;sec1 = sec1->next) {
    if (sec1->name) {
      if (!strcmp(sec1->name, name)) {
	printf("matched section:%s\n", name);
	return sec1;
      }
    }
  }
  return 0;
}

