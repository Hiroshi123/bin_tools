
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

extern void add_dyn_relative_entry(size_t offset, size_t addend);
/* extern uint32_t add_plt_got_entry(char* str); */
extern uint32_t add_dynamic_entry2(size_t offset, char* str);

extern Config* Confp;

static int dynamic_resolve_num = 0;
static int external_resolve_num = 0;
static int internal_resolve_num = 0;
static int pre_dynamic_resolve_num = 0;
static int pre_external_resolve_num = 0;
static int pre_internal_resolve_num = 0;

void count_program_header() {
  Confp->virtual_address_offset +=
    Confp->program_header_num * sizeof(Elf64_Phdr) + sizeof(Elf64_Ehdr);
}

void __z__link__elf_set_virtual_address(void* arg1) {

  SectionContainer* sc = arg1;
  SectionChain* schain = sc->init;
  if (sc->init == 0) return;
  int size = 0;
  Elf64_Shdr* shdr;
  if (sc->virtual_address == 0) {
    sc->virtual_address = Confp->virtual_address_offset;
  }
  for (;schain;schain = schain->this) {
    shdr = schain->p;
    // schain->virtual_address = sc->virtual_address + sc->size;
    // printf("%p,%p\n", shdr->sh_offset, schain->virtual_address);
    shdr->sh_addr = schain->virtual_address = Confp->virtual_address_offset + sc->size;
    sc->size += shdr->sh_size;
  }
  int tmp = sc->size;//((sc->size + 3/*3shdr->sh_addralign*/) >> 2) << 2;
  Confp->virtual_address_offset += Confp->output_vaddr_alignment + tmp;
  char max_name[100] = {};
  sprintf(max_name, "[link/elf/reloc.c] size:%p, virtual address;%p\n",
	  sc->size, sc->virtual_address);
  logger_emit("misc.log", max_name);
}

static void fill_address(void* addr, uint8_t type, uint32_t src_addr, uint32_t dst_addr) {
  /* printf("fill,%p,%p,%p\n",addr, type, R_X86_64_64); */
  char max_name[100] = {};
  sprintf(max_name, "[link/elf/reloc.c]\t addr:%p, reloc type:%d, src addr:%p, dst_addr:%p\n",
	  addr, type, src_addr, dst_addr);
  logger_emit("misc.log", max_name);
  if (*(uint32_t*)addr) {
    printf("strange\n");
  }
  switch (type) {
  case R_X86_64_64/*1*/:
    // if this have to be PIE, should be converted to R_X86_64_RELATIVE(8) on dynamic link
    // note it does not require name of symbols.
    printf("strange\n");
    printf("fill!,%p,%p,%p,%p\n",addr,
    	   *(uint64_t*)addr, *((uint64_t*)(addr) - 1), dst_addr);
    // add_dyn_relative_entry(src_addr, dst_addr);
    add_rela_plt_entry(src_addr, 0, R_X86_64_RELATIVE, dst_addr);
    // *((uint64_t*)addr) = dst_addr;// shdr->sh_addr + rela->r_addend;
    break;
  case R_X86_64_REX_GOTPCRELX:// function pointer    
    printf("[R_X86_64_REX_GOTPCRELX]\n");
  case R_X86_64_PC32/*2*/:
  case R_X86_64_PLT32:
    /* printf("PC32:%p\n" , dst_addr - (src_addr/\* + 4*\/)); */
    *((uint32_t*)addr) = dst_addr - (src_addr/* + 4*/);
    break;
  case R_X86_64_32S/*11*/:
    *((uint32_t*)addr) = dst_addr;
    printf("R_X86_64_32S on 64bit?\n");
    break;
  default:
    printf("did not matched!!!!\n");
    break;
  }
}

/* static uint32_t resolve_external(char* name) {   */
/*   size_t* table_index = M1(Confp->ExportHashTable, elf_hash, name); */
/*   if (*table_index) { */
/*     // printf("bucket is %d\n", *table_index); */
/*   } */
/*   SymbolChain* chain = *table_index; */
/*   for (;chain;chain = chain->next) { */
/*     if (!strcmp(name, chain->name)) { */
/*       Elf64_Sym* p = chain->p; */
/*       return chain->schain->virtual_address + p->st_value; */
/*     } */
/*   } */
/*   return 0; */
/* } */

static void resolve(ObjectChain* oc, Elf64_Rela* rela, Elf64_Shdr* sub_shdr/*, void* addr*/) {
  void* addr = sub_shdr->sh_offset + rela->r_offset;
  Elf64_Sym* sym = oc->symbol_table_p;
  int num = oc->symbol_num;
  sym += ELF64_R_SYM(rela->r_info);
  int sym_type = ELF64_ST_TYPE(sym->st_info);
  int rel_type = ELF64_R_TYPE(rela->r_info);
  uint32_t src_addr = sub_shdr->sh_addr + rela->r_offset;
  uint32_t dst_addr = 0;
  char* name = oc->str_table_p + sym->st_name;
  char max_name[100] = {};
  // ATTENTION :: addend seems to be negative.
  // if it is not -4, then should pay attention the bug which relates this.
  size_t addend = 0 - rela->r_addend;
  switch (sym_type) {

  case STT_NOTYPE:
  case STT_OBJECT:
  case STT_FUNC: {
    // dst_addr = resolve_external(name);
    Elf64_Sym* p = (Elf64_Sym*)Confp->dynsym_head + find_sym2(name);
    dst_addr = p->st_value;
    if (dst_addr == 0) {
      // add dynsym entry
      sprintf(max_name, "[link/elf/reloc.c]\t resolve dynamically:%s\n",
	  name);
      logger_emit("misc.log", max_name);
      dynamic_resolve_num += 1;
      // in this case, this must be data.
      // you do not need to add got.plt playload but dynsym + dynstr + rela.plt(REL_GOT)      
      if (rel_type == R_X86_64_64) {
	uint32_t r = add_dynamic_entry2(src_addr, name);
	// printf("sym + dynamic,%p\n", r);
      } else if (rel_type == R_X86_64_REX_GOTPCRELX) {
	dst_addr = add_dynamic_entry(name);
	// TODO :: curreint implementation is ADHOC.
	// function address is substited with R_X86_64_REX_GOTPCRELX.
	// In this case, you do not need to feed only GOT not with PLT because
	// you do not need to call it directly.
	// +8 means ignoring plt which are put just before got.
	fill_address(addr, rel_type, src_addr + addend, dst_addr + 8);
      }
      else {
	dst_addr = add_dynamic_entry(name);
	fill_address(addr, rel_type, src_addr + addend, dst_addr);
      }
    } else {

      external_resolve_num += 1;
      sprintf(max_name, "[link/elf/reloc.c]\t resolve externally:%s\n", name);
      logger_emit("misc.log", max_name);
      // printf("resolve externally,%p,%p,\n", sub_shdr->sh_addr + rela->r_offset, dst_addr);
      if (rel_type == R_X86_64_64) {
	add_rela_plt_entry(src_addr, 0, R_X86_64_RELATIVE, dst_addr);
      } else {
	fill_address(addr, rel_type, src_addr + addend, dst_addr);
      }
    }
    break;
  }
  case STT_SECTION: {
    Elf64_Shdr* shdr = ((Elf64_Shdr*)oc->section_head) + sym->st_shndx;
    /* printf("st shndx,%d,%p,%p\n", sym->st_shndx, shdr, shdr->sh_addr); */
    // #ISSUES, if addend is negative, it means the type size of data is defined here.
    // I wont reflect the negation.
    if (rela->r_addend > 0) {
      dst_addr = shdr->sh_addr + rela->r_addend;
      /* printf("dst:%p,%p,%p,%p\n", rela->r_offset,dst_addr,shdr->sh_addr,rela->r_addend); */
    } else {
      dst_addr = shdr->sh_addr + rela->r_addend;
      /* printf("neg:%p,%p,%p,%p\n", rela->r_offset,dst_addr,shdr->sh_addr,rela->r_addend); */
      //dst_addr = shdr->sh_addr;
    }
    internal_resolve_num += 1;
    sprintf(max_name, "[link/elf/reloc.c]\t resolve internally\n");
    logger_emit("misc.log", max_name);
    // ISSUES :: Compiler provides apparently -4 offset value before data?????
    // But src.addr is also negated by 4, it will be ok to be as it is.
    if (rel_type == R_X86_64_64) {
      /* printf("internal\n"); */
      add_rela_plt_entry(src_addr, 0, R_X86_64_RELATIVE, dst_addr);
    } else {
      // printf("fill\n");
      fill_address(addr, rel_type, src_addr, dst_addr);
    }
    break;
  }
  default:
    printf("not yet supported type:%p\n", sym_type);
    break;
  }
}

void __z__link__elf_do_reloc(void* _oc, void* arg1) {

  ObjectChain* oc = _oc;
  Elf64_Rela* rela;
  Elf64_Rela* rela_end;
  Elf64_Shdr* rel_shdr;
  Elf64_Shdr* sub_shdr;
  // printf("reloc,%p,%p\n", oc, oc->reloc_section_head);
  if (oc->reloc_section_head) {
    rel_shdr = oc->reloc_section_head;
    rela = rel_shdr->sh_offset;
    rela_end = (uint8_t*)rela + rel_shdr->sh_size;
    sub_shdr = (Elf64_Shdr*)oc->section_head + rel_shdr->sh_info;
    uint8_t* addr = rel_shdr->sh_offset + rela->r_offset;
    for (;rela<rela_end;rela++) {
      resolve(oc, rela, sub_shdr);
    }
  }
  if (Confp->verbose) {
    printf("dynamic:%d,%d\n", dynamic_resolve_num - pre_dynamic_resolve_num, dynamic_resolve_num);
    printf("extenal:%d,%d\n", external_resolve_num - pre_external_resolve_num, external_resolve_num);
    printf("internal:%d,%d\n", internal_resolve_num - pre_internal_resolve_num, internal_resolve_num);
    printf("-------------\n");

    pre_dynamic_resolve_num = dynamic_resolve_num;
    pre_external_resolve_num = external_resolve_num;
    pre_internal_resolve_num = internal_resolve_num;
  }
}


