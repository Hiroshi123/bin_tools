
#include <windows.h>
#include <stdio.h>
#include <stdint.h>
#include "win_memory.h"

#include "coff.h"
#include "link.h"

extern callback_arg3(void* arg1,void* arg2,void* f);
extern SectionChain* InitialSection;
extern uint32_t ImportDirectoryLen;

void wrapper_f(void* arg1, void* arg2, void* f) {
  callback_arg3(arg1,arg2,f);
}

void do_reloc(void* f) {
  SectionChain* sec1 = InitialSection;
  SectionChain* sec2;
  ObjectChain* oc1;
  IMAGE_SYMBOL* is1;
  IMAGE_SECTION_HEADER* ish;
  int i = 0;
  CoffReloc* reloc;
  CallbackArgIn arg;
  uint32_t* addr;
  for (;sec1;sec1=sec1->next) {
    for (sec2=sec1->this;sec2;sec2=sec2->this) {
      if (ish=sec2->p) {
	reloc = ish->PointerToRelocations;
	oc1 = sec2->obj;
	for (i=0;i<ish->NumberOfRelocations;i++,reloc++) {
	  is1 = oc1->symbol_table_p + reloc->SymbolTableIndex;
	  arg.name = GET_NAME(is1, oc1->str_table_p);
	  arg.virtual_address = ish->VirtualAddress + reloc->VirtualAddress;
	  arg.type = reloc->Type;
	  addr = sec2->data + reloc->VirtualAddress;
	  if (*addr) {
	    printf("should skip!!\n");
	  }
	  //printf("a,%p,%d,%p,%p,%p\n",ish->VirtualAddress + reloc->VirtualAddress
	  //	 ,ish->NumberOfRelocations,addr,*addr,reloc->Type);
	  // call wrapper function of callback
	  wrapper_f(&arg,addr,f);
	  // printf("a,%p,%d,%p,%p\n",sec2,ish->NumberOfRelocations,addr,*addr);
	}
      }
    }
  }
}

void resolve(CallbackArgIn* _in,uint32_t* addr) {
  char* name = _in->name;
  uint16_t type = _in->type;
  size_t* export_address = 0;
  printf("j\n");
  IMAGE_SYMBOL* ret = lookup_symbol(name, &export_address);
  printf("i\n");
  if (ret) {
  printf("o\n");
    printf("callback called,%s,%d,%p,%d,%d,%p(!%x),%p\n",
	   name, type,ret,ret->SectionNumber,ret->Value,addr,*(uint64_t*)addr,export_address);
    if (type == IMAGE_REL_AMD64_ADDR32)
      *addr = export_address;
    else if (type == IMAGE_REL_AMD64_REL32)
      *addr = _in->virtual_address - export_address;
  } else {
    char* dllname = lookup_dynamic_symbol(name, 1);
    printf("s\n");
    if (dllname) {
      void* new = add_dynamic_resolved_entry(name, dllname, addr);
      if (new)
	ImportDirectoryLen += sizeof(IMAGE_IMPORT_DESCRIPTOR) + 1 + strlen(dllname);
      ImportDirectoryLen += 4 + 2 + strlen(name);
      *addr = 0;
      printf("%s was resolved on %s\n", name, dllname);
    } else {
      printf("could not resolved.. \n");
    }
    // if you could not resolve, it will be re-evaluated with the dynamic loader at 2nd stage.
    // Dynamic loader
  }
}


