
#include <stdint.h>
#include <stdio.h>

#include "alloc.h"
#include "link.h"
#include "os.h"
#include "pe.h"

void* add_coff_section(char* name, uint32_t vaddr, uint32_t size, void* data) {
  IMAGE_SECTION_HEADER* sec = __malloc(sizeof(IMAGE_SECTION_HEADER));
  int i = 0;
  char* n = name;
  for (; *n; n++, i++) {
    sec->Name[i] = *n;
  }
  // strcpy(&sec->Name[0], name);
  sec->NumberOfRelocations = 0;
  sec->PointerToRelocations = 0;
  sec->PointerToRawData = 0;
  sec->SizeOfRawData = size;
  sec->Characteristics = 0xe0500020;
  // you cannot fill any data in this stage as the virtual address has not yet
  // been decided.
  SectionContainer* scon = alloc_section_container(vaddr, name, 0, 0);
  SectionChain* schain = alloc_section_chain(sec, data, scon, 0);
  schain->virtual_address = vaddr;
  return scon;
}

void* get_export_virtual_address(IMAGE_SYMBOL* is, ObjectChain* oc) {
  SectionChain* sc1 = oc->section_chain_head;
  // FIXME : if this is not the way that it should be,
  // change it to the proper way.
  int i = 0;
  IMAGE_SECTION_HEADER* ss;
  for (; sc1; sc1 = sc1->next) {
    i++;
    if (i == is->SectionNumber) {
      ss = sc1->p;
      // Value is the offset from header of the section.
      // If you find another section above the section,
      // you need to know
      /* printf("!%p,%s,%p,%p\n", sc1, ss->Name, ss->VirtualAddress, is->Value);
       */
      return ss->VirtualAddress + is->Value;
    }
  }
  return 0;
}

void* get_section_name(IMAGE_SYMBOL* is, ObjectChain* oc) {
  SectionChain* sc1 = oc->section_chain_head;
  int i = 0;
  IMAGE_SECTION_HEADER* ss;
  for (; sc1; sc1 = sc1->next) {
    i++;
    if (i == is->SectionNumber) {
      ss = sc1->p;
      return ss->Name;
    }
  }
}
extern Config* Confp;

SectionChain* get_section_chain_from_name(char* name) {
  SectionContainer* sec1 = Confp->initial_section;
  SectionChain* sec2 = 0;
  for (; sec1; sec1 = sec1->next) {
    for (sec2 = sec1->this; sec2; sec2 = sec2->this) {
      if (!strcmp(((IMAGE_SECTION_HEADER*)(sec2->p))->Name, name)) {
        return sec2;
      }
    }
  }
  return 0;
}

void add_tls(SectionChain* tlsdata) {
  void* p;
  if (1) {
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

/* IMAGE_SYMBOL* get_symbol_from_section_chain(SectionChain* sec1,char* name) {
 */
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
