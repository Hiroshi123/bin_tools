
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
extern uint8_t EmitType;

void wrapper_f(void* arg1, void* arg2, void* f) {
  callback_arg3(arg1,arg2,f);
}

void do_reloc(void* f) {
  SectionChain* sec1 = InitialSection;
  SectionChain* sec2;
  ObjectChain* oc1;
  IMAGE_SYMBOL* is1;
  IMAGE_SECTION_HEADER* ish;
  CoffReloc* reloc;
  CallbackArgIn arg;
  uint32_t* addr;
  char* name;
  int i = 0;
  for (;sec1;sec1=sec1->next) {
    for (sec2=sec1->this;sec2;sec2=sec2->this) {
      if (ish=sec2->p) {
	reloc = ish->PointerToRelocations;
	oc1 = sec2->obj;
	for (i=0;i<ish->NumberOfRelocations;i++,reloc++) {
	  is1 = oc1->symbol_table_p + reloc->SymbolTableIndex;
	  name = GET_NAME(is1, oc1->str_table_p);
	  arg.name = name;
	  // virtual address points to the head of address not the tail.
	  if (EmitType != EMIT_OBJ) {
	    arg.virtual_address = ish->VirtualAddress + reloc->VirtualAddress;
	  } else {
	    arg.section_name = sec2->p->Name;
	  }
	  arg.type = reloc->Type;
	  arg.storage_class = is1->StorageClass;
	  addr = sec2->data + reloc->VirtualAddress;
	  // call wrapper function of callback
	  wrapper_f(&arg,addr,f);
	  printf("n:%s\n", name);
	}
      }
    }
  }
}

void fill_address(uint32_t* addr, uint16_t type, uint32_t dst_vaddr, uint32_t src_vaddr) {
  switch (type) {
  case IMAGE_REL_AMD64_ADDR64/*1*/:
    *(uint64_t*)addr += ImageBase + dst_vaddr;
    break;
  case IMAGE_REL_AMD64_ADDR32/*2*/:
    *addr += ImageBase + dst_vaddr;
    break;
  case IMAGE_REL_AMD64_ADDR32NB/*3*/:
    *addr += dst_vaddr;
    break;
  case IMAGE_REL_AMD64_REL32/*4*/:
    *addr += dst_vaddr - (src_vaddr + 4);
    break;
  case IMAGE_REL_AMD64_REL32 | IMAGE_REL_AMD64_SSPAN32:
    *addr += dst_vaddr - (src_vaddr + 4);
    break;
  default:
    printf("not supported type\n");
    break;
    // case 5 to 9 is REL32_1 to REL32_5
    // from A to F, they are somehow special.
    // IMAGE_REL_AMD64_SECTION
    // (0x10)IMAGE_REL_AMD64_SSPAN32
  }
}

void resolve_only_in_a_section(CallbackArgIn* _in, uint32_t* addr) {
  char* name = _in->name;
  uint16_t type = _in->type;
  if (type == IMAGE_REL_AMD64_REL32) {
    ObjectChain* oc = 0;
    IMAGE_SYMBOL* is = lookup_symbol(name, &oc);
    if (is) {
      char* export_section_name = get_section_name(is, oc);
      if (!strcmp(export_section_name, _in->section_name)) {
	printf("within a section\n");
	logger_emit("relocation:resolved on another file\n");
	size_t* export_address = get_export_virtual_address(is, oc);
	fill_address(addr, type, export_address, _in->virtual_address);      
      }
      return;
    }
  }
}

void resolve(CallbackArgIn* _in,uint32_t* addr) {
  char* name = _in->name;
  uint16_t type = _in->type;
  // 1st, you should check symbols on the same object.
  // If storage class is static, it is likely that ADDR64/ADDR32 which
  // requires to fill virtual address + ImageBase on it directly.
  if (_in->storage_class == 3/*Static*/) {
    SectionChain* sc = get_section_chain_from_name(name);
    fill_address(addr, type, sc->p->VirtualAddress, _in->virtual_address);
    logger_emit("relocation:resolved on a same file(StorageClass==3)\n");    
    return;
  }
  ObjectChain* oc = 0;
  IMAGE_SYMBOL* is = lookup_symbol(name, &oc);
  if (is) {
    size_t* export_address = get_export_virtual_address(is, oc);
    logger_emit("relocation:resolved on another file\n");
    /* printf("callback called,%s,%d,%p,%d,%d,%p(!%x),%p,%p\n", */
    /* 	   name, type,ret,ret->SectionNumber,ret->Value,addr, */
    /* 	   *(uint64_t*)addr,export_address, _in->virtual_address); */
    fill_address(addr, type, export_address, _in->virtual_address);
    return;
  }
  uint32_t ever = 0;
  char* dllname = lookup_dynamic_symbol(name, 1, &ever);
  if (!dllname) {
    fprintf(stdout, "could not resolved..%s \n", name);
    return;
  }
  logger_emit("relocation:resolved on an entry of a dyanmic dll\n");
  // printf("ever:%p\n", ever);
  // PltOffset(0xff 0x25 0x00 0x00 0x00 0x00) - (addr + 1)(just after call)
  if (ever) {
    fill_address(addr, type, ever, _in->virtual_address);
    // *addr = ever - (uint32_t)_in->virtual_address - 4;
    printf("do not need to add new import entry.\n");
    return;
  }
  fill_address(addr, type, PltOffset, _in->virtual_address);
  // *addr = PltOffset - (uint32_t)_in->virtual_address - 4;	
  PltOffset += 6;
  void* new = add_dynamic_resolved_entry(name, dllname, addr);
  if (new) {
    // printf("new!,%p,%p,%p\n", addr,*addr, *(addr-1));
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
}


