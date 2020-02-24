
#include <stdio.h>
#include <stdint.h>

#include "elf.h"
#include "link.h"

// elf relocation
// address which needs to be filled is given by index of section and the offset.
// supplier of the address is always given by an entry or two entries of symbol table.

// [rela_info(type) == 1(STV_INTERNAL)]
// If it is resolved within this object, the section name as well as the name of entry is provided.
// Since a section may contain multiple symbols which corresponds to its entries, r_addend will be used for identifying the value of the entry where you need to traverse symbol table.

// [rela info(type) == 2(STV_HIDDEN)]
// If it is resolved externally, section information has not yet been known, and only index of an symbol table entry is provided.

extern Config* Confp;

void count_program_header() {

  Confp->virtual_address_offset +=
    Confp->program_header_num * sizeof(Elf64_Phdr) + sizeof(Elf64_Ehdr);
}

void set_virtual_address(void* arg1) {
  printf("set\n");
  SectionContainer* sc = arg1;
  SectionChain* schain = sc->init;
  int size = 0;
  Elf64_Shdr* shdr;
  if (sc->virtual_address == 0) {
    sc->virtual_address = Confp->virtual_address_offset;
  }
  for (;schain;schain = schain->this) {
    shdr = schain->p;
    schain->virtual_address = sc->virtual_address + sc->size;
    sc->size += shdr->sh_size;
    printf("%p,%p\n", shdr->sh_offset, schain->virtual_address);
    shdr->sh_addr = Confp->virtual_address_offset;
  }
  printf("\n");
  int tmp = ((sc->size + /*size + */3) >> 2) << 2;
  Confp->virtual_address_offset += Confp->output_vaddr_alignment + tmp;
  printf("f2,%p,%d,%p,%d\n",sc, sc->size, sc->virtual_address, tmp);  
}

static void fill_address(void* addr, uint8_t type, uint32_t src_addr, uint32_t dst_addr) {
  switch (type) {
  case R_X86_64_64:
    *((uint64_t*)addr) = dst_addr;// shdr->sh_addr + rela->r_addend;
    break;
  case R_X86_64_32S:
    *((uint32_t*)addr) = dst_addr;// shdr->sh_addr + rela->r_addend;
    printf("this type!!!,%p,%p\n", addr, *((uint32_t*)addr-1));
    break;
  default:
    break;
  }
}

static uint32_t resolve_external(char* name) {
  size_t* table_index = (Confp->HashTable.bucket + (elf_hash(name) % Confp->HashTable.nbucket));
  if (*table_index) {
    printf("bucket is %d\n", *table_index);
  } 
  SymbolChain* chain = *table_index;
  if (!strcmp(name, chain->name)) {
    printf("name:%s\n", chain->name);
    printf("vaddr:%p\n", chain->schain->virtual_address);
    return chain->schain->virtual_address;
  }
}

static void resolve(ObjectChain* oc, Elf64_Rela* rela, void* addr) {
  Elf64_Sym* sym = oc->symbol_table_p;
  int num = oc->symbol_num;
  printf("%p,%p,%p\n", sym, num, ELF64_R_SYM(rela->r_info));
  sym += ELF64_R_SYM(rela->r_info);
  int type = ELF64_ST_TYPE(sym->st_info);
  switch (type) {
  case STT_NOTYPE:
    printf("resolve externally,%p,%s\n", rela->r_addend, oc->str_table_p + sym->st_name);    
    uint32_t dst_addr = resolve_external(oc->str_table_p + sym->st_name);
    fill_address(addr, type, 0, dst_addr);
    break;
  case STT_SECTION:
    printf("resolve internally\n");
    Elf64_Shdr* shdr = ((Elf64_Shdr*)oc->section_head) + sym->st_shndx;
    fill_address(addr, type, 0, shdr->sh_addr + rela->r_addend);
    break;
  }
}

void do_reloc(void* _oc, void* arg1) {

  ObjectChain* oc = _oc;// Confp->initial_object;
  // Confp->initial_object;
  Elf64_Rela* rela;
  Elf64_Rela* rela_end;
  Elf64_Shdr* shdr;
  
  printf("reloc,%p,%p\n", oc, oc->reloc_section_head);
  if (oc->reloc_section_head) {
    shdr = oc->reloc_section_head;
    rela = shdr->sh_offset;
    rela_end = (uint8_t*)rela + shdr->sh_size;
    uint8_t* addr = shdr->sh_offset + rela->r_offset;
    for (;rela<rela_end;rela++) {
      printf("r:%p\n", rela);
      addr = shdr->sh_offset + rela->r_offset;
      resolve(oc, rela, addr);
    }
  }
}


