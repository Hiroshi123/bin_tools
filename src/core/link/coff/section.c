
#ifdef _WIN32 
#include <windows.h>
#include "coff.h"
#endif
#include <stdio.h>
#include <stdint.h>
#include "alloc.h"

#include "link.h"

uint8_t TotalSectionNum = 0;
uint32_t TotalHeaderSize = 0;
extern SectionChain* InitialSection;
extern SectionChain* CurrentSection;
extern uint32_t TotalImageSize;
extern ObjectChain* CurrentObject;
extern uint8_t _Win32;

SectionChain* InitialSection = 0;
SectionChain* CurrentSection = 0;

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
  /* TotalImageSize += (s->SizeOfRawData + SECTION_ALIGNMENT) & 0xFFFFF000; */  
  return sec2;
}

SectionChain* check_section(IMAGE_SECTION_HEADER* _sec) {

  SectionChain* s = InitialSection;
  IMAGE_SECTION_HEADER* sec;
  for (;s;s = s->next) {
    sec = s->this->p;
    if (!strcmp(sec->Name, _sec->Name)) {
      for (;s->this;s=s->this);
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
  sec->Characteristics = 0xe0500020;
  // you cannot fill any data in this stage as the virtual address has not yet been decided.
  TotalImageSize += (size + SECTION_ALIGNMENT) & 0xFFFFF000;  
  return alloc_section_chain(sec, 0, 0);
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

  TotalHeaderSize = sizeof(IMAGE_DOS_HEADER) + 4 + sizeof(IMAGE_FILE_HEADER)
    + sizeof(IMAGE_OPTIONAL_HEADER64) + sizeof(IMAGE_SECTION_HEADER) * TotalSectionNum;
  if (_Win32)
    TotalHeaderSize -= 0x10;
  uint32_t virtual_address = (TotalHeaderSize + SECTION_ALIGNMENT) & ~(SECTION_ALIGNMENT - 1);
  TotalImageSize += virtual_address;
  SectionChain* sc = InitialSection;
  SectionChain* _sc;
  IMAGE_SECTION_HEADER* sh;
  uint32_t offset = (TotalHeaderSize + FILE_ALIGNMENT) & ~(FILE_ALIGNMENT - 1);
  // Iterate on SectionContainer
  for (;sc;sc=sc->next) {
    // Iterate on SectionChain
    for (_sc = sc->this;_sc;_sc=_sc->this) {
      sh = _sc->p;
      sh->VirtualAddress = virtual_address;
      // virtual size com@utation needs to be done here???
      sh->Misc.VirtualSize = sh->SizeOfRawData;
      // PointerToRawData
      sh->PointerToRawData = offset;
      offset += sh->SizeOfRawData;
      virtual_address += sh->SizeOfRawData;
#ifdef DEBUG
      printf("name:%s\n",sh->Name);
      printf("sh:%p\n",sh);
      printf("sc:%p\n",_sc);
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

void update_pointer_to_raw_data() {
  SectionChain* sc = InitialSection;
  SectionChain* _sc;
  IMAGE_SECTION_HEADER* sh;
  uint32_t offset = 0;
  for (;sc;sc=sc->next) {
    // Iterate on SectionChain
    offset = 0;
    for (_sc = sc->this;_sc;_sc=_sc->this) {
      sh = _sc->p;
      sh->PointerToRawData = offset;
      offset += sh->SizeOfRawData;
    }
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
      /* printf("!%p,%s,%p,%p\n", sc1, ss->Name, ss->VirtualAddress, is->Value); */
      return ss->VirtualAddress + is->Value;
    }
  }
  return 0;
}

void* get_section_name(IMAGE_SYMBOL* is, ObjectChain* oc) {
  SectionChain* sc1 = oc->section_chain_head;
  int i = 0;
  IMAGE_SECTION_HEADER* ss;
  for (;sc1;sc1=sc1->next) {
    i++;
    if (i == is->SectionNumber) {
      ss = sc1->p;
      return ss->Name;
    }
  }
}

SectionChain* get_section_chain_from_name(char* name) {
  SectionChain* sec1 = InitialSection;
  SectionChain* sec2 = 0;
  for (;sec1;sec1=sec1->next) {
    for (sec2=sec1->this;sec2;sec2=sec2->this) {
      if (!strcmp(sec2->p->Name, name)) {
	return sec2;
      }
    }
  }
  return 0;
}

void add_tls(SectionChain* tlsdata) {

  void* p;
  if (_Win32) {
    // uint32_t* p = tlsdata;
    
    // raw data start va(4byte)
    
    // raw data end va(4byte)

    // address of index(4byte)

    // address of callbacks(4byte)
    
  } else {
    // uint64_t* p = tlsdata;
    // raw data start va(8byte)
    // *(uint64_t*) = 
    // raw data end va(8byte)
    p++;
    // address of index(8byte)
    p++;
    // address of callbacks(8byte)
    p++;
  }
  uint32_t* q = p;
  // sizeofzero(4byte)
  q++;
  // Characteristics(4byte)
  q++;
  *q = 0;
}

/* IMAGE_SYMBOL* get_symbol_from_section_chain(SectionChain* sec1,char* name) { */
/*   SymbolChain* sc1; */
/*   char* name; */
/*   IMAGE_SYMBOL* is; */
/*   ObjectChain* oc1 = sec1->obj; */
/*   for (sc1 = sec1->sym_head;sc1;sc1 = sc1->next) { */
/*     is = sc1->p */
/*     name = GET_NAME(is, oc1->str_table_p); */
/*     if (!strcmp(sc1->p->Name, name)) { */
/*       return is; */
/*     } */
/*   } */
/*   return 0; */
/* } */

