
#include <windows.h>
#include <stdio.h>
#include <stdint.h>
#include "win_memory.h"

#include "coff.h"
#include "link.h"

uint8_t TotalSectionNum = 0;
extern SectionChain* InitialSection;
extern SectionChain* CurrentSection;
extern uint32_t TotalImageSize;
extern ObjectChain* CurrentObject;

SectionChain* InitialSection = 0;
SectionChain* CurrentSection = 0;

void* alloc_section_chain(void* obj ,IMAGE_SECTION_HEADER* s ,SectionChain* _s) {

  SectionChain* sec1;
  SectionChain* sec2;
  if (!_s) {
    sec1 = __malloc(sizeof(SectionChain));
    sec1->num = 1;
    sec1->next = 0;
    TotalSectionNum += 1;
  } else {
    sec1 = _s;
  }
  sec2 = __malloc(sizeof(SectionChain));
  sec2->p = s;
  sec2->data = obj ? obj + s->PointerToRawData : 0;// datap;
  sec2->this = 0;
  sec2->obj = obj ? CurrentObject : 0;
  sec2->next = 0;
  sec1->this = sec2;
  if (!InitialSection) {
    InitialSection = sec1;    
  } else {
    if (!_s) {
      CurrentSection->next = sec1;
    }
  }
  if (!_s)
    CurrentSection = sec1;  
  if (CurrentObject->section_chain_head == 0) {
    CurrentObject->section_chain_head = sec2;
  } else {
    CurrentObject->section_chain_tail->next = sec2;
  }  
  CurrentObject->section_chain_tail = sec2;  
  TotalImageSize += (s->SizeOfRawData + SECTION_ALIGNMENT) & 0xFFFFF000;
  return sec2;
}

SectionChain* check_section(IMAGE_SECTION_HEADER* _sec) {

  SectionChain* s = InitialSection;
  IMAGE_SECTION_HEADER* sec;
  for (;s;s = s->next) {
    sec = s->this->p;
    printf("!!%p,%p,%s,%s\n", s, s->this->p, sec->Name, _sec->Name);    
    if (!strcmp(sec->Name, _sec->Name)) {
      for (;s->this;s=s->this);
      printf("ss:%p\n",s);
      return s;
    }
  }
  return 0;
}

void* add_section(char* name, uint32_t size) {
  
  IMAGE_SECTION_HEADER* sec = __malloc(sizeof(IMAGE_SECTION_HEADER));
  strcpy(sec->Name, name);
  sec->NumberOfRelocations = 0;
  sec->PointerToRelocations = 0;
  sec->PointerToRawData = 0;
  sec->SizeOfRawData = size;
  // you cannot fill any data in this stage as the virtual address has not yet been decided.
  return alloc_section_chain(0, sec, 0);
}

// SizeOfRawData is going to be reflected on PointerToRawData/VirtualAddress
// if you supplied new SizeOfRawData on a section,
// then it is afftected to every subsequent section.
// For instance, SizeOfRawData can be computed after relocation.
// But, Relocation needs VirtualAddress.
// To avoid this mutual dependency,
// 1st, set the section whose SizeOfRawData cannot be determined on a section where the size is least affected on
// its subsequent sections, namely last one.

void set_virtual_address() {
  SectionChain* sc = InitialSection;
  SectionChain* _sc;
  IMAGE_SECTION_HEADER* sh;
  uint32_t offset = 0x200;
  // Iterate on SectionContainer
  uint32_t virtual_address = 0x1000;
  uint32_t section_size = 0;
  for (;sc;sc=sc->next) {
    // Iterate on SectionChain
    uint32_t section_size = 0;
    for (_sc = sc->this;_sc;_sc=_sc->this) {
      sh = _sc->p;
      sh->VirtualAddress = virtual_address;
      // virtual size com@utation needs to be done here???
      sh->Misc.VirtualSize = sh->SizeOfRawData;
      // PointerToRawData
      sh->PointerToRawData = offset;
      offset += sh->SizeOfRawData;
      section_size = sh->SizeOfRawData;
#ifdef DEBUG
      printf("name:%s\n",sh->Name);
      printf("p:%p\n", sh->PointerToRawData);
      printf("s:%p\n", sh->SizeOfRawData);
      printf("v:%p\n", sh->VirtualAddress);
      printf("r:%p\n", sh->PointerToRelocations);
      printf("d:%p\n", _sc->data);
#endif
      /* offset += (sh->SizeOfRawData + FILE_ALIGNMENT - 1) & (0 - FILE_ALIGNMENT); */
      // virtual_size += sh->SizeOfRawData;
      // VirtualAddressOffset += (sh->VirtualAddress + SECTION_ALIGNMENT) & 0xFFFFF000;
      // printf("offset:%p,%p\n",offset, VirtualAddressOffset);
    }
    offset = (offset + FILE_ALIGNMENT - 1) & (0 - FILE_ALIGNMENT);
    virtual_address = (virtual_address + SECTION_ALIGNMENT) & 0xFFFFF000;    
  }
}

void* get_export_virtual_address(IMAGE_SYMBOL* is, ObjectChain* oc) {
  SectionChain* sc1 = oc->section_chain_head;
  // FIXME : if this is not the way that it should be,
  // change it to the proper way.
  int i = 0;
  IMAGE_SECTION_HEADER* ss;
  for (;sc1;sc1=sc1->next) {    
    i++;
    if (i == is->SectionNumber) {
      ss = sc1->p;
      // Value is the offset from header of the section.
      // If you find another section above the section,
      // you need to know
      printf("!%s,%p,%p\n", ss->Name, ss->VirtualAddress, is->Value);
      return ss->VirtualAddress + is->Value;
    }
  }
  return 0;
}


