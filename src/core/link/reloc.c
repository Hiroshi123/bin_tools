
#include <windows.h>
#include <stdio.h>
#include <stdint.h>
#include "win_memory.h"

#include "coff.h"
#include "link.h"

extern callback_arg3(void* arg1,void* arg2,void* f);
extern SectionChain* InitialSection;
extern uint32_t ImportDirectoryLen;

extern uint32_t PltBegin;
extern uint32_t PltOffset;
extern uint8_t _Win32;
extern uint64_t ImageBase;

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
  char* name;
  for (;sec1;sec1=sec1->next) {
    printf("sec1:%p\n",sec1);
    for (sec2=sec1->this;sec2;sec2=sec2->this) {
      printf("sec2:%p,%s\n",sec2, sec2->p->Name);
      if (ish=sec2->p) {
	reloc = ish->PointerToRelocations;
	oc1 = sec2->obj;
	for (i=0;i<ish->NumberOfRelocations;i++,reloc++) {
	  is1 = oc1->symbol_table_p + reloc->SymbolTableIndex;
	  name = GET_NAME(is1, oc1->str_table_p);
	  arg.name = name;
	  arg.virtual_address = ish->VirtualAddress + reloc->VirtualAddress;
	  arg.type = reloc->Type;
	  arg.storage_class = is1->StorageClass;
	  if (is1->StorageClass == 3/*Static*/) {
	    printf("name:%s,%p,%p\n", name,sec1, sec1->this);
	    SectionChain* sc = get_section_chain_from_name(name);
	    // sc->p->VirtualAddress;
	    // arg.dst_virtual_address = sc->p->VirtualAddress + is1->Value;
	    if (sc) {
	    printf("storage class static !%p,%s,%p,%p\n",
	    	   sc->p->VirtualAddress,
	    	   arg.name,is1->Value,is1->StorageClass);
	    } else {
	      printf("sc:%p\n", sc);
	    }
	  }
	  addr = sec2->data + reloc->VirtualAddress;
	  if (*addr) {
	    printf("applied to!!\n");
	  }
	  // call wrapper function of callback
	  wrapper_f(&arg,addr,f);
	}
      }
    }
  }
}

void resolve(CallbackArgIn* _in,uint32_t* addr) {
  char* name = _in->name;
  uint16_t type = _in->type;
  size_t* export_address = 0;
  // 1st, you should check symbols on the same object.  
  if (_in->storage_class == 3/*Static*/) {
    SectionChain* sc = get_section_chain_from_name(name);
    *addr += sc->p->VirtualAddress;
    if (_Win32) {
      *addr += ImageBase;
    }
    return;
  }
  IMAGE_SYMBOL* ret = lookup_symbol(name, &export_address);  
  if (ret) {
    printf("callback called,%s,%d,%p,%d,%d,%p(!%x),%p\n",
	   name, type,ret,ret->SectionNumber,ret->Value,addr,
	   *(uint64_t*)addr,export_address);
    if (type == IMAGE_REL_AMD64_ADDR32)
      *addr = export_address;
    else if
      (type == IMAGE_REL_AMD64_REL32 ||
       type == IMAGE_REL_AMD64_REL32 | IMAGE_REL_AMD64_SSPAN32) {
      *addr = (uint32_t)export_address - ((uint32_t)_in->virtual_address + 4);
    } else {
      printf("unknown type:%p,%p\n",type,IMAGE_REL_AMD64_SREL32);
    }
  } else {
    printf("type:%p\n",type);
    char* dllname = lookup_dynamic_symbol(name, 1);
    if (dllname) {
      // PltOffset(0xff 0x25 0x00 0x00 0x00 0x00) - (addr + 1)(just after call)
      *addr = ((uint8_t*)PltOffset - (uint8_t*)_in->virtual_address) - 4;
      PltOffset += 6;
      void* new = add_dynamic_resolved_entry(name, dllname, addr);
      if (new) {
	if (ImportDirectoryLen == 0) {
	  // if it is 1st ever insertion of IID, you need to set an empty one
	  // at the end of it.
	  ImportDirectoryLen += sizeof(IMAGE_IMPORT_DESCRIPTOR);
	}
	ImportDirectoryLen += sizeof(IMAGE_IMPORT_DESCRIPTOR) + 1 + strlen(dllname);
	// needs empty image_thunk_data at the end of it for indicating its end.
	ImportDirectoryLen += 2 * sizeof(void*/*IMAGE_THUNK_DATA*/);
      }
      // 1entry = (IAT) + (INT) + HINT + strlen + NULL
      ImportDirectoryLen += 2 * sizeof(void*/*IMAGE_THUNK_DATA*/) + 2 + strlen(name) + 1;
      printf("%s was resolved on %s\n", name, dllname);
    } else {
      printf("could not resolved.. \n");
    }
    // if you could not resolve, it will be re-evaluated with the dynamic loader at 2nd stage.
    // Dynamic loader
  }
}


