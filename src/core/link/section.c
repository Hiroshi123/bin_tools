
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

SectionContainer* InitialSection = 0;
SectionContainer* CurrentSection = 0;

extern ObjectChain* InitialObject;// = 0;
extern ObjectChain* CurrentObject;// = &InitialObject;

/* ObjectChain* InitialObject = 0; */
/* ObjectChain* CurrentObject = &InitialObject; */

SectionContainer* alloc_section_container
(uint32_t va, void* name, void* candidate_list, ListContainer* Sc) {
  SectionContainer* sc = __malloc(sizeof(SectionContainer));
  sc->virtual_address = va;
  sc->name = name;
  sc->candidate_list = candidate_list;
  if (Sc->init) {
    ((SectionContainer*)Sc->current)->next = sc;
  } else {
    Sc->init = sc;
  }
  Sc->current = sc;
  return sc;
}

void* alloc_section_chain(void* s, void* offset, SectionContainer* _s) {

  SectionContainer* sec1;
  SectionChain* sec2;
  if (!_s) {
    sec1 = __malloc(sizeof(SectionContainer));    
    TotalSectionNum += 1;
  } else {
    sec1 = _s;
  }
  sec2 = __malloc(sizeof(SectionChain));
  sec2->p = s;
  sec2->data = offset;// datap;
  // sec2->data = obj ? obj + s->PointerToRawData : 0;// datap;
  sec2->this = 0;  
  sec2->obj = CurrentObject;
  // sec2->obj = obj ? CurrentObject : 0;
  sec2->next = 0;
  sec1->this = sec2;
  if (!InitialSection) {
    InitialSection = sec1;
    printf("init had been set,%p\n", sec1);
  } else {
    if (!_s) {
      printf("next:%p\n", sec1);
      CurrentSection->next = sec1;
    }
  }
  if (!_s)
    CurrentSection = sec1;
  /* if (CurrentObject->section_chain_head == 0) { */
  /*   CurrentObject->section_chain_head = sec2; */
  /* } else { */
  /*   CurrentObject->section_chain_tail->next = sec2; */
  /* } */
  /* CurrentObject->section_chain_tail = sec2; */
  /* TotalImageSize += (s->SizeOfRawData + SECTION_ALIGNMENT) & 0xFFFFF000; */  
  return sec2;
}


