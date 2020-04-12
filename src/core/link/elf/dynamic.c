
#include <stdio.h>
#include <stdint.h>

#include "alloc.h"
#include "elf.h"
#include "link.h"

extern Config* Confp;

// you need to allocate 4 things to let dynamic section work properly.
// 1. SectionContainer
// 2. SectionChain
// 3. SectionHeader
// 4. Data or code which are reffered from SectionHeader.

// |sh_name|sh_type|sh_flags|
// |---|---|---|
// |.dynamic|SHT_STRTAB|SHF_ALLOC , SHF_WRITE|SHF_ALLOC|
// |.dynstr |SHT_STRTAB|SHF_ALLOC |SHF_ALLOC|SHF_ALLOC|
// |.dynsym |SHT_DYNSYM|SHF_ALLOC |SHF_ALLOC|SHF_ALLOC|

static SectionContainer* plt_got_p = 0;
static void* plt_got_d = 0;
static int plt_got_entry_num = 0;
static SectionContainer* dynamic = 0;
static SectionContainer* dynsym = 0;
static SectionContainer* dynstr = 0;
static void* dynamic_d = 0;
static void* dynsym_d = 0;
static size_t dynstr_size = 0;
static size_t dynstr_p = 0;
static void* dynstr_entry = 0;
static void* rela_plt_p = 0;
static void* rela_plt_d = 0;

Elf64_Shdr* alloc_elf_section(int type, int flags) {
  Elf64_Shdr* shdr = __malloc(sizeof(Elf64_Shdr));  
  shdr->sh_type = type;
  shdr->sh_name = 0;// ".dynamic";
  shdr->sh_flags = flags;
  shdr->sh_addr = 0;
  shdr->sh_size = 0;// size;
  shdr->sh_link = 5;
  shdr->sh_info = 0;
  shdr->sh_addralign = 0;
  shdr->sh_entsize = 0;
  return shdr;
}

void* add_dynamic_sc() {
  
  printf("add dynamic,%p\n", Confp->current_section);
  SectionContainer* sc = alloc_section_container(0, ".dynamic", 0, 0);
  dynamic = sc;
  Elf64_Shdr* shdr = alloc_elf_section(SHT_DYNAMIC, SHF_ALLOC | SHF_WRITE);
  alloc_section_chain(shdr, 0, sc);
  // Elf64_Dyn
  printf("elf64_dyn:%p\n", sizeof(Elf64_Dyn));
  int size = sizeof(Elf64_Dyn) * Confp->dynamic_entry_num;
  void* d = __malloc(size);
  dynamic_d = d;
  shdr->sh_offset = d;
  shdr->sh_size = size;
  return d;
}

void add_dynsym_sc() {
  printf("add dynamic symbol,%p\n", Confp->current_section);
  SectionContainer* sc = alloc_section_container(0, ".dynsym", 0, 0);
  dynsym = sc;
  Elf64_Shdr* shdr = alloc_elf_section(SHT_DYNSYM, SHF_ALLOC);
  alloc_section_chain(shdr, 0, sc);
  int size = 3 * sizeof(Elf64_Sym);
  void* d = __malloc(size);
  shdr->sh_offset = d;
  shdr->sh_size = size;
  dynsym_d = d + sizeof(Elf64_Sym);
  printf("!!!!%p,%p,%p\n", size, dynsym_d, d);
}

void add_dynsym_entry(uint32_t str_offset) {
  Elf64_Sym* sym = dynsym_d;
  sym->st_name = str_offset;
  int bind = STB_GLOBAL;//ELF64_ST_BIND(STB_GLOBAL);
  int type = STT_FUNC;//ELF64_ST_TYPE(STT_FUNC);
  sym->st_info = ELF64_ST_INFO(bind, type);
  printf("st info:%p,%p\n", dynsym_d, sym->st_info);
  sym->st_other = ELF64_ST_VISIBILITY(STV_DEFAULT);
  // this tells if 
  sym->st_shndx = 0;
  sym->st_value = 0;
  sym->st_size = 0;
}

uint32_t add_dynstr_entry(char* name) {
  char* p = dynstr_p + dynstr_size;
  uint32_t r = dynstr_size;
  for (;*name!=0;p++, name++) {
    *p = *name;
  }
  dynstr_size = (p + 1) - dynstr_p;
  return r;
}

// you cannot determine how much amount of entry is required at this stage.

void add_dynstr_sc() {
  printf("add dynamic string,%p\n", Confp->current_section);
  SectionContainer* sc = alloc_section_container(0, ".dynstr", 0, 0);
  dynstr = sc;
  Elf64_Shdr* shdr = alloc_elf_section(SHT_STRTAB, SHF_ALLOC);
  alloc_section_chain(shdr, 0, sc);
  int size = 0x20;
  void* d = __malloc(size);
  dynstr_p = d;
  dynstr_size = 1;
  shdr->sh_offset = d;
  shdr->sh_size = size;
}

uint32_t add_pltgot_entry() {
  uint8_t* s = plt_got_d;
  uint32_t r = plt_got_p->virtual_address + plt_got_entry_num * 0x10;
  plt_got_entry_num ++;
  plt_got_d += 0x10;
  uint8_t p[] = {0xff, 0x25, 0x2, 0x0, 0x0, 0x0};
  int i = 0;
  for (;i < 6;s++,i++) {
    *s = p[i];
  }
  s+=2;
  return r;
}

uint32_t add_dynamic_entry(char* str) {

  // 4. .got.plt
  // 5. .got
  uint32_t r = add_pltgot_entry();
  // 1. Dynamic string entry
  printf("aa:%s\n", str);
  if (1) {
    char* lib = "libc.so.6";
    add_dynstr_entry(lib);
  }
  // 2. Dynsym entry
  add_dynsym_entry(add_dynstr_entry(str));
  // 3. .rela.dyn or .rela.plt entry
  add_rela_plt_entry(r + 8);
  return r;
}

void add_pltgot_sc() {
  
  // 0x10 will be entry size
  SectionContainer* sc = alloc_section_container(0, ".plt.got", 0, 0);
  plt_got_p = sc;
  // SHT_DYNSYM
  Elf64_Shdr* shdr = alloc_elf_section(SHT_DYNAMIC, SHF_ALLOC | SHF_WRITE);
  alloc_section_chain(shdr, 0, sc);
  int size = 0x10;
  void* d = __malloc(size);
  plt_got_d = d;
  shdr->sh_offset = d;
  shdr->sh_size = size;
}

void add_rela_plt_sc() {
  SectionContainer* sc = alloc_section_container(0, ".rela.plt", 0, 0);
  rela_plt_p = sc;
  Elf64_Shdr* shdr = alloc_elf_section(SHT_DYNAMIC, SHF_ALLOC | SHF_WRITE);
  alloc_section_chain(shdr, 0, sc);
  int size = sizeof(Elf64_Rela);
  Elf64_Rela* d = __malloc(size);
  rela_plt_d = d;
  shdr->sh_offset = d;
  shdr->sh_size = size;
}

void add_rela_plt_entry(size_t offset) {
  
  Elf64_Rela* d = rela_plt_d;
  d->r_offset = offset;
  // uint64_t sym = ELF64_R_SYM(1);
  // REL_PLT
  int sym = plt_got_entry_num;
  int type = ELF64_R_TYPE(R_X86_64_JUMP_SLOT);
  d->r_info = ELF64_R_INFO(sym, type);
  d->r_addend = 0;
}

void add_init() {
  
}

void add_fini() {
  
}

void add_init_array() {
  
}

void add_fini_array() {
  
}

void set_dynanmic() {
  
  uint32_t r = plt_got_p->virtual_address + plt_got_entry_num * 0x10;
  
  Elf64_Dyn* dyn = dynamic_d;
  dyn->d_tag = DT_NEEDED;
  dyn->d_un.d_val = 1;
  dyn++;
  
  r += Confp->dynamic_entry_num * sizeof(Elf64_Dyn);
  dyn->d_tag = DT_RELA;
  dyn->d_un.d_ptr = r;
  dyn++;
  
  dyn->d_tag = DT_RELASZ;
  dyn->d_un.d_ptr = plt_got_entry_num * sizeof(Elf64_Rela);
  dyn++;
  
  r += plt_got_entry_num * sizeof(Elf64_Rela);
  dyn->d_tag = DT_SYMTAB;
  dyn->d_un.d_ptr = r;
  dyn++;

  r += (2 + plt_got_entry_num) * sizeof(Elf64_Sym);
  dyn->d_tag = DT_STRTAB;
  dyn->d_un.d_ptr = r;
  dyn++;

  dyn->d_tag = DT_STRSZ;
  dyn->d_un.d_ptr = dynstr_size;
  
  r += dynstr_size;
  Confp->out_size = r - Confp->base_address;
  
}

void add_dynamic() {
  
  // dynamic vector which is refferred from another program header.
  add_dynamic_sc();
  add_rela_plt_sc();
  // these are needed for hiding 
  add_dynsym_sc();
  add_dynstr_sc();
  
  // dependencies should be added after relocation.
  // add_deps();
  // these are not relavant to relocation.
  // but, still it needs to be done after fixing virtual address.
  
  /* add_init(); */
  /* add_fini(); */
  /* add_init_array(); */
  /* add_fini_array(); */
  
  /* add_plt(); */
  /* add_got(); */
  
  // these entry should be added when you resolve something on an external shared library.
  // add_rela_dyn();
  // 
  
}

