
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
static Elf64_Sym* dynsym_d = 0;
static int dynsym_entry_num = 0;
static size_t dynstr_size = 0;
static size_t dynstr_p = 0;
static void* dynstr_entry = 0;
static int dynstr_entry_num = 0;
static int deps_lib_num = 0;

static void* rela_plt_p = 0;
static void* rela_plt_d = 0;

#define ALLOCATE_SIZE 0x100
#define NBUCKET 10
static uint32_t* dynamic_size_p = 0;
static uint32_t* rela_plt_size_p = 0;
static uint32_t* dynsym_size_p = 0;
static uint32_t* dynstr_size_p = 0;
static uint32_t* pltgot_size_p = 0;
static uint32_t* dt_hash_size_p = 0;
static DtHashTable* dt_hash_table_p;
static int dt_hash_size = 0;

static uint32_t DynamicEntryTag[10] = {
  DT_NEEDED,
  DT_HASH,
  DT_RELA,
  DT_RELASZ,
  DT_RELAENT,
  DT_SYMTAB,
  DT_SYMENT,
  DT_STRTAB,
  DT_STRSZ,
  DT_NULL,
};

static uint32_t sysv_hash(const char *s0)
{
	const unsigned char *s = (void *)s0;
	uint_fast32_t h = 0;
	while (*s) {
		h = 16*h + *s++;
		h ^= h>>24 & 0xf0;
	}
	return h & 0xfffffff;
}

static uint32_t gnu_hash(const char *s0)
{
	const unsigned char *s = (void *)s0;
	uint_fast32_t h = 5381;
	for (; *s; s++)
		h += h*32 + *s;
	return h;
}

void check_collision(uint32_t* bucket, uint32_t* chain, uint32_t mod, uint32_t sym_index) {

  uint32_t* p = 0;
  if (*(bucket + mod)) {
    printf("collision:%p,%p\n", *(bucket+mod), sym_index);
    printf("chain:%p\n", chain + *(bucket+mod));
    // uint32_t index = *(bucket + mod);
    // p = chain + index;
    // uint32_t q = *p;
    for (p = chain + *(bucket + mod) ; *p ; p = chain + *p);
    *p = sym_index;
  } else {
    // no collision
    *(bucket + mod) = sym_index;
  }
}

void add_dt_hash_entry(char* name, int sym_index) {
  uint32_t hash = sysv_hash(name);
  int mod = hash % NBUCKET;
  dt_hash_table_p->nchain ++;  
  uint32_t* bucket = dt_hash_table_p + 1;
  uint32_t* chain = bucket + dt_hash_table_p->nbucket;
  printf("mod:%d,%p\n", mod, bucket);
  check_collision(bucket, chain, mod, sym_index);
  dt_hash_size += 4;
  // hash_table_p->bucket = 
}

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

void add_dt_hash_sc() {
  SectionContainer* sc = alloc_section_container(0, ".dt.hash", 0, 0);
  dynamic = sc;
  Elf64_Shdr* shdr = alloc_elf_section(SHT_DYNAMIC, SHF_ALLOC | SHF_WRITE);
  alloc_section_chain(shdr, 0, sc);  
  // NBUCKET
  // sizeof(uint32_t) + sizeof(uint32_t) + sizeof(uint32_t)
  int nchain = 100;
  // DT_HASH_SIZE will be composed with 4 different elements.
  // The last of them has not fixed length.
  int bucket_size = sizeof(uint32_t);
  int nchain_size = sizeof(uint32_t);
  int nbucket = sizeof(uint32_t) * NBUCKET;

  printf("export symbol num : %d\n", Confp->current_object->export_symbol_num);
  
  int nchain_max = (Confp->current_object->export_symbol_num + 1) * sizeof(uint32_t);
  int size = bucket_size + nchain_size + nbucket + nchain_max;
  DtHashTable* hash = __malloc(size);
  dt_hash_table_p = hash;
  hash->nbucket = NBUCKET;
  hash->nchain = 0;
  shdr->sh_offset = hash;  
  // shdr->sh_size = size;
  dt_hash_size_p = &(shdr->sh_size);
  dt_hash_size = bucket_size + nchain_size + nbucket;
  return;
}

void* add_dynamic_sc() {
  
  // printf("add dynamic,%p\n", Confp->current_section);
  SectionContainer* sc = alloc_section_container(0, ".dynamic", 0, 0);
  dynamic = sc;
  Elf64_Shdr* shdr = alloc_elf_section(SHT_DYNAMIC, SHF_ALLOC | SHF_WRITE);
  alloc_section_chain(shdr, 0, sc);
  // Elf64_Dyn  
  int size = sizeof(Elf64_Dyn) * 20/*Confp->dynamic_entry_num*/;
  void* d = __malloc(size);
  dynamic_d = d;  
  shdr->sh_offset = d;
  dynamic_size_p = &(shdr->sh_size);
  // shdr->sh_size = size;
  return d;
}

void add_dynsym_entry(uint32_t str_offset, uint32_t shndx, uint32_t value) {
  Elf64_Sym* sym = dynsym_d;
  dynsym_d ++;
  sym->st_name = str_offset;
  int bind = STB_GLOBAL;//ELF64_ST_BIND(STB_GLOBAL);
  int type = STT_FUNC;//ELF64_ST_TYPE(STT_FUNC);
  sym->st_info = ELF64_ST_INFO(bind, type);
  // printf("st info:%p,%p\n", dynsym_d, sym->st_info);
  sym->st_other = ELF64_ST_VISIBILITY(STV_DEFAULT);
  // this tells if 
  sym->st_shndx = shndx;
  sym->st_value = value;
  sym->st_size = 0;
  dynsym_entry_num += 1;
}

void add_dynsym_sc() {
  printf("add dynamic symbol,%p\n", Confp->current_section);
  SectionContainer* sc = alloc_section_container(0, ".dynsym", 0, 0);
  dynsym = sc;
  Elf64_Shdr* shdr = alloc_elf_section(SHT_DYNSYM, SHF_ALLOC);
  alloc_section_chain(shdr, 0, sc);
  int symnum_max = Confp->current_object->symbol_num + 100;
  int size = symnum_max * sizeof(Elf64_Sym);
  void* d = __malloc(size);
  shdr->sh_offset = d;
  dynsym_size_p = &(shdr->sh_size);
  // shdr->sh_size = size;
  dynsym_d = d;// + sizeof(Elf64_Sym);
  printf("!!!!%p,%p,%p\n", size, dynsym_d, d);
  add_dynsym_entry(0, 0, 0);
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
  printf("add dynamic string,%p\n", Confp->current_section);
  dynstr = sc;
  Elf64_Shdr* shdr = alloc_elf_section(SHT_STRTAB, SHF_ALLOC);
  alloc_section_chain(shdr, 0, sc);
  int size = 0xf00;
  void* d = __malloc(size);
  dynstr_p = d;
  dynstr_size = 1;
  shdr->sh_offset = d;
  // shdr->sh_size = size;
  dynstr_size_p = &(shdr->sh_size);
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
  /* if (1) { */
  /*   char* lib = "libc.so"; */
  /*   add_dynstr_entry(lib); */
  /* } */
  // 2. Dynsym entry
  add_dynsym_entry(add_dynstr_entry(str), 0, 0);
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
  int size = 0x10 * 0x100;
  void* d = __malloc(size);
  plt_got_d = d;
  shdr->sh_offset = d;
  pltgot_size_p = &(shdr->sh_size);
  // shdr->sh_size = size;
}

void add_rela_plt_sc() {
  SectionContainer* sc = alloc_section_container(0, ".rela.plt", 0, 0);
  rela_plt_p = sc;
  Elf64_Shdr* shdr = alloc_elf_section(SHT_DYNAMIC, SHF_ALLOC | SHF_WRITE);
  alloc_section_chain(shdr, 0, sc);
  int size = 0x100 * sizeof(Elf64_Rela);
  Elf64_Rela* d = __malloc(size);
  rela_plt_d = d;
  shdr->sh_offset = d;
  rela_plt_size_p = &(shdr->sh_size);
  // shdr->sh_size = size;
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

void add_export_symbol() {

  SymbolChain* sym = Confp->current_object->symbol_chain_head;
  char* str = 0;
  uint32_t value = 0;
  char max_name[100] = {};
  
  for (;sym;sym = sym->next) {
    str = sym->name;
    sprintf(max_name, "[link/elf/dynamic.c]\t add export symbol:%s\n", str);
    logger_emit("misc.log", max_name);
    value = sym->schain->virtual_address +
      ((Elf64_Sym*)(sym->p))->st_value - Confp->base_address;
    add_dynsym_entry(add_dynstr_entry(str), 1, value);
    add_dt_hash_entry(str, dynsym_entry_num-1);
  }
  logger_emit("memory.log", "done export\n");

}

uint32_t set_dynamic_entry(Elf64_Dyn* dyn, int tag, uint32_t offset) {

  switch (tag) {
  case DT_NEEDED:
    dyn->d_tag = tag;
    dyn->d_un.d_val = offset;
    break;
  case DT_HASH:
    dyn->d_tag = tag;
    dyn->d_un.d_val = offset;
    offset += dt_hash_size;
    *dt_hash_size_p = dt_hash_size;
    break;  
  case DT_RELA:
    dyn->d_tag = tag;
    dyn->d_un.d_val = offset;
    offset += plt_got_entry_num * sizeof(Elf64_Rela);
    *rela_plt_size_p = plt_got_entry_num * sizeof(Elf64_Rela);
    *pltgot_size_p = plt_got_entry_num * 0x10;
    break;
  case DT_RELASZ:
    dyn->d_tag = tag;
    dyn->d_un.d_val = plt_got_entry_num * sizeof(Elf64_Rela);
    break;
  case DT_RELAENT:
    dyn->d_tag = tag;
    dyn->d_un.d_val = sizeof(Elf64_Rela);
    break;
  case DT_SYMTAB:
    dyn->d_tag = tag;
    dyn->d_un.d_val = offset;
    printf("dynsym ent num:%d\n", dynsym_entry_num);
    offset += (1 + dynsym_entry_num) * sizeof(Elf64_Sym);
    *dynsym_size_p = (1 + dynsym_entry_num) * sizeof(Elf64_Sym);
    break;
  case DT_SYMENT:
    dyn->d_tag = tag;
    dyn->d_un.d_val = sizeof(Elf64_Sym);
    break;
  case DT_STRTAB:
    dyn->d_tag = tag;
    dyn->d_un.d_val = offset;
    break;
  case DT_STRSZ:
    dyn->d_tag = tag;
    dyn->d_un.d_val = dynstr_size;
    offset += dynstr_size;
    *dynstr_size_p = dynstr_size;
    break;
  default:
    break;
  }
  return offset;
}

void add_dt_needed(char* name) {
  char* lib = name;
  add_dynstr_entry(lib);
  deps_lib_num += 1;
}

void set_dynanmic() {
  
  if (Confp->nodynamic) {
    Confp->out_size = Confp->virtual_address_offset - Confp->base_address
      + 0x10;
    return;
  }
  uint32_t r = plt_got_p->virtual_address + plt_got_entry_num * 0x10;  
  Elf64_Dyn* dyn = dynamic_d;  
  int i = 0;
  uint32_t p = 1;
  printf("deps lib num:%d,%p\n", deps_lib_num, r);
  for (;i < deps_lib_num;i++) {
    set_dynamic_entry(dyn, DT_NEEDED, p);
    uint8_t* q = dynstr_p + p;
    // go to next entry
    for (;*q;q++,p++);
    dyn++;
    Confp->dynamic_entry_num ++;
  }
  r += Confp->dynamic_entry_num * sizeof(Elf64_Dyn);
  *dynamic_size_p = Confp->dynamic_entry_num * sizeof(Elf64_Dyn);
  
  printf("r:%p\n", r);
  i = 1;
  for (;i<=Confp->dynamic_entry_num;i++) {
    r = set_dynamic_entry(dyn, DynamicEntryTag[i], r);
    dyn++;
  }
  Confp->out_size = r - Confp->base_address;
}

void add_dynamic() {

  char max_name[50] = {};
  sprintf(max_name, "[link/elf/dynamic.c]\t add dynamic\n");
  logger_emit("misc.log", max_name);
  
  // dynamic vector which is refferred from another program header.
  add_dynamic_sc();
  add_dt_hash_sc();
  add_rela_plt_sc();
  // these are needed for hiding 
  add_dynsym_sc();
  logger_emit("memory.log", "dynamic 04\n");
  add_dynstr_sc();
  logger_emit("memory.log", "dynamic 05\n");
  
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

