
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
static uint8_t use_init_array = 1;
static uint8_t use_fini_array = 1;

#define ALLOCATE_SIZE 0x100
#define NBUCKET 10

#define DT_DYNAMIC_INDEX 0
#define INIT_ARRAY_INDEX 1
#define FINI_ARRAY_INDEX 2
#define DT_HASH_INDEX 3
#define RELA_PLT_INDEX 4
#define DYNSYM_INDEX 5
#define DYNSTR_INDEX 6
#define PLT_GOT_INDEX 7

struct DynamicParams {
  uint32_t* size_p;
  uint8_t* data_p;
  union {
    uint32_t size;
    uint32_t num;
  };
  uint32_t alloc_size;
};

struct DynamicParams D[10];

static int DEFAULT_DYNAMIC_ENTRY_NUM = 0x20;
static int DEFAULT_DYNSYM_NUM = 0x1000;
static int DEFAULT_DYNSTR_NUM = 0x1000;
static int DEFAULT_PLTGOT_NUM = 0x100;
static int DEFAULT_RELA_PLT_NUM = 0x100;
static int DEFAULT_DT_HASH_CHAIN_NUM = 0x100;
static int DEFAULT_INIT_ARRAY_NUM = 0x10;

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

static void set_dynamic_alloc_size() {

  // D[10];
  D[DT_DYNAMIC_INDEX].alloc_size = sizeof(Elf64_Dyn) * 0x20;
  D[INIT_ARRAY_INDEX].alloc_size = 0x20;  
  D[FINI_ARRAY_INDEX].alloc_size = 0x20;  

  int bucket_size = sizeof(uint32_t);
  int nchain_size = sizeof(uint32_t);
  int nbucket = sizeof(uint32_t) * NBUCKET;
  int nchain_max = DEFAULT_DT_HASH_CHAIN_NUM * sizeof(uint32_t);
  int size = bucket_size + nchain_size + nbucket + nchain_max;
  D[DT_HASH_INDEX].alloc_size = size;
  D[DT_HASH_INDEX].size = bucket_size + nchain_size + nbucket + 0x4;
  
  D[RELA_PLT_INDEX].alloc_size = DEFAULT_RELA_PLT_NUM * sizeof(Elf64_Rela);
  // 
  D[DYNSYM_INDEX].alloc_size = DEFAULT_DYNSYM_NUM * sizeof(Elf64_Sym);
  
  //
  D[DYNSTR_INDEX].alloc_size = DEFAULT_DYNSTR_NUM;
  D[DYNSTR_INDEX].size = 1;
  
}

static void check_collision(uint32_t* bucket, uint32_t* chain, uint32_t mod, uint32_t sym_index) {

  uint32_t* p = 0;
  if (*(bucket + mod)) {
    // printf("collision:%p,%p\n", *(bucket+mod), sym_index);
    // printf("chain:%p\n", chain + *(bucket+mod));
    // uint32_t index = *(bucket + mod);
    // p = chain + index;
    // uint32_t q = *p;
    // for (p = chain + *(bucket + mod) ; *p ; p = chain + *p);
    for (p = chain + *(bucket + mod) ; *p ; p = chain + *p);
    *p = sym_index;
  } else {
    // no collision
    *(bucket + mod) = sym_index;
  }
}

static void add_dt_hash_entry(char* name, int sym_index) {
  uint32_t hash = sysv_hash(name);
  int mod = hash % NBUCKET;
  DtHashTable* hash_table_p = D[DT_HASH_INDEX].data_p;
  hash_table_p->nchain ++;
  uint32_t* bucket = hash_table_p + 1;
  uint32_t* chain = bucket + hash_table_p->nbucket;
  check_collision(bucket, chain, mod, sym_index);
  D[DT_HASH_INDEX].size += 4;
  // printf("mod:%d,%d,%p,%p\n",
  //	 mod, sym_index, bucket, dt_hash_size);
  // hash_table_p->bucket = 
}

static Elf64_Shdr* alloc_elf_section(int type, int flags) {
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

static void add_dynsym_entry(uint32_t str_offset, uint32_t shndx, uint32_t value) {
  Elf64_Sym* sym = D[DYNSYM_INDEX].data_p;
  D[DYNSYM_INDEX].data_p += sizeof(Elf64_Sym);
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
  D[DYNSYM_INDEX].num += 1;
}

static uint32_t add_dynstr_entry(char* name) {
  char* p = D[DYNSTR_INDEX].data_p + D[DYNSTR_INDEX].size;
  uint32_t r = D[DYNSTR_INDEX].size;
  for (;*name!=0;p++, name++) {
    *p = *name;
  }
  D[DYNSTR_INDEX].size = (p + 1) - (char*)D[DYNSTR_INDEX].data_p;
  return r;
}

static uint32_t add_pltgot_entry() {
  uint8_t* s = D[PLT_GOT_INDEX].data_p;
  uint32_t r = plt_got_p->virtual_address + D[PLT_GOT_INDEX].num * 0x10;
  D[PLT_GOT_INDEX].num ++;
  D[PLT_GOT_INDEX].data_p += 0x10;
  uint8_t p[] = {0xff, 0x25, 0x2, 0x0, 0x0, 0x0};
  int i = 0;
  for (;i < 6;s++,i++) {
    *s = p[i];
  }
  return r;
}

static void add_rela_plt_entry(size_t offset, int index) {
  
  Elf64_Rela* d = D[RELA_PLT_INDEX].data_p;
  D[RELA_PLT_INDEX].data_p += sizeof(Elf64_Rela);
  d->r_offset = offset;
  // uint64_t sym = ELF64_R_SYM(1);
  // REL_PLT
  int sym = index;// D[DYNSYM_INDEX].num;
  int type = ELF64_R_TYPE(R_X86_64_JUMP_SLOT);
  d->r_info = ELF64_R_INFO(sym, type);
  d->r_addend = 0;
}


uint32_t add_dynamic_entry(char* str) {

  uint32_t r = (uint32_t)lookup_symbol(str, 1);
  if (r) {
    return r;
  }
  char max_name[100] = {};
  sprintf(max_name, "[link/elf/dynamic.c]\t add dynamic import symbol:%s\n", str);
  logger_emit("misc.log", max_name);
  
  r = add_pltgot_entry();
  alloc_dynamic_symbol_chain(r, str, 0);  
  // 1. Dynamic string entry
  // 2. Dynsym entry
  add_dynsym_entry(add_dynstr_entry(str), 0, 0);
  // 3. .rela.dyn or .rela.plt entry
  add_rela_plt_entry(r + 8, D[DYNSYM_INDEX].num - 1);
  return r;
}

void add_pltgot_sc(int index, char* name) {
  // 0x10 will be entry size
  SectionContainer* sc = alloc_section_container(0, name, 0, 0);
  plt_got_p = sc;
  // SHT_DYNSYM
  Elf64_Shdr* shdr = alloc_elf_section(SHT_DYNAMIC, SHF_ALLOC | SHF_WRITE);
  alloc_section_chain(shdr, 0, sc);
  int size = 0x10 * DEFAULT_PLTGOT_NUM;
  void* d = __malloc(size);
  D[index].data_p = d;
  shdr->sh_offset = d;
  D[index].size_p = &(shdr->sh_size);
  // shdr->sh_size = size;
}

void add_init() {
  
}

void add_fini() {
  
}

void add_init_array(void* addr) {
  *(size_t*)(D[INIT_ARRAY_INDEX].data_p) = addr;
  D[INIT_ARRAY_INDEX].data_p += 8;
  D[INIT_ARRAY_INDEX].size += 8;
}

void add_fini_array(void* addr) {
  *(size_t*)(D[FINI_ARRAY_INDEX].data_p) = addr;
  D[FINI_ARRAY_INDEX].data_p += 8;
  D[FINI_ARRAY_INDEX].size += 8;  
}

void add_export_symbol(void* _oc, void* arg1) {
  ObjectChain* oc = _oc;
  SymbolChain* sym = oc->symbol_chain_head;
  if (!sym) return;
  char* str = 0;
  uint32_t value = 0;
  char max_name[100] = {};
  static uint64_t* init = "____init";
  static uint64_t* fini = "____fini";
  for (;sym;sym = sym->next) {
    str = sym->name;
    sprintf(max_name, "[link/elf/dynamic.c]\t add export symbol:%s\n", str);
    logger_emit("misc.log", max_name);
    value = sym->schain->virtual_address +
      ((Elf64_Sym*)(sym->p))->st_value - Confp->base_address;
    add_dynsym_entry(add_dynstr_entry(str), 1, value);
    add_dt_hash_entry(str, D[DYNSYM_INDEX].num-1);
    if (*(uint64_t*)str == *init) {
      // use_init_array = 1;
      add_init_array(value + Confp->base_address);
    }
    if (*(uint64_t*)str == *fini) {
      // use_init_array = 1;
      add_fini_array(value + Confp->base_address);
    }
  }
}

uint32_t set_dynamic_entry(Elf64_Dyn* dyn, int tag, uint32_t offset) {

  dyn->d_tag = tag;
  switch (tag) {
  case DT_NEEDED:
    dyn->d_un.d_val = offset;
    break;
  case DT_INIT_ARRAY:
    dyn->d_un.d_val = offset;
    offset += D[INIT_ARRAY_INDEX].size;
    break;
  case DT_FINI_ARRAY:
    dyn->d_un.d_val = offset;
    offset += D[FINI_ARRAY_INDEX].size;
    break;
  case DT_INIT_ARRAYSZ:
    dyn->d_un.d_val = D[INIT_ARRAY_INDEX].size;
    *D[INIT_ARRAY_INDEX].size_p = D[INIT_ARRAY_INDEX].size;
    break;
  case DT_FINI_ARRAYSZ:
    dyn->d_un.d_val = D[FINI_ARRAY_INDEX].size;
    *D[FINI_ARRAY_INDEX].size_p = D[FINI_ARRAY_INDEX].size;
    break;
  case DT_HASH:
    dyn->d_un.d_val = offset;
    offset += D[DT_HASH_INDEX].size;
    *D[DT_HASH_INDEX].size_p = D[DT_HASH_INDEX].size;
    break;
  case DT_RELA:
    dyn->d_un.d_val = offset;
    offset += D[PLT_GOT_INDEX].num * sizeof(Elf64_Rela);
    *D[RELA_PLT_INDEX].size_p = D[PLT_GOT_INDEX].num * sizeof(Elf64_Rela);
    *D[PLT_GOT_INDEX].size_p = D[PLT_GOT_INDEX].num * 0x10;
    break;
  case DT_RELASZ:
    dyn->d_un.d_val = D[PLT_GOT_INDEX].num * sizeof(Elf64_Rela);
    break;
  case DT_RELAENT:
    dyn->d_un.d_val = sizeof(Elf64_Rela);
    break;
  case DT_SYMTAB:
    dyn->d_un.d_val = offset;
    offset += (1 + D[DYNSYM_INDEX].num) * sizeof(Elf64_Sym);
    *D[DYNSYM_INDEX].size_p = (1 + D[DYNSYM_INDEX].num) * sizeof(Elf64_Sym);
    break;
  case DT_SYMENT:
    dyn->d_un.d_val = sizeof(Elf64_Sym);
    break;
  case DT_STRTAB:
    dyn->d_un.d_val = offset;
    break;
  case DT_STRSZ:
    dyn->d_un.d_val = D[DYNSTR_INDEX].size;
    offset += D[DYNSTR_INDEX].size;
    *D[DYNSTR_INDEX].size_p = D[DYNSTR_INDEX].size;
    break;
  default:
    break;
  }
  return offset;
}

void add_dt_needed(char* name) {
  char* lib = name;
  add_dynstr_entry(lib);
  D[DT_DYNAMIC_INDEX].num += 1;
}

void set_dynanmic() {
  
  if (Confp->nodynamic) {
    Confp->out_size = Confp->virtual_address_offset - Confp->base_address
      + 0x10;
    return;
  }
  uint32_t r = plt_got_p->virtual_address + D[PLT_GOT_INDEX].num * 0x10;
  Elf64_Dyn* dyn = D[DT_DYNAMIC_INDEX].data_p;
  int i = 0;
  uint32_t p = 1;
  for (;i < D[DT_DYNAMIC_INDEX].num;i++) {
    set_dynamic_entry(dyn, DT_NEEDED, p);
    uint8_t* q = D[DYNSTR_INDEX].data_p + p;
    // go to next entry
    for (p++;*q;q++,p++);
    dyn++;
    Confp->dynamic_entry_num ++;
  }
  if (use_init_array) {
    Confp->dynamic_entry_num +=2;
  }
  r += Confp->dynamic_entry_num * sizeof(Elf64_Dyn);
  *D[0].size_p = Confp->dynamic_entry_num * sizeof(Elf64_Dyn);
  if (use_init_array) {
    r = set_dynamic_entry(dyn, DT_INIT_ARRAY, r);
    dyn++;
    r = set_dynamic_entry(dyn, DT_INIT_ARRAYSZ, r);
    dyn++;
  }
  if (use_fini_array) {
    r = set_dynamic_entry(dyn, DT_FINI_ARRAY, r);
    dyn++;
    r = set_dynamic_entry(dyn, DT_FINI_ARRAYSZ, r);
    dyn++;
  }
  i = 1;
  for (;i<=Confp->dynamic_entry_num;i++) {
    r = set_dynamic_entry(dyn, DynamicEntryTag[i], r);
    dyn++;
  }  
  Confp->out_size = r - Confp->base_address;
}

void* add_dynamic_sc(int index, char* name) {
  
  SectionContainer* sc = alloc_section_container(0, name, 0, 0);
  Elf64_Shdr* shdr = alloc_elf_section(SHT_DYNAMIC, SHF_ALLOC | SHF_WRITE);
  alloc_section_chain(shdr, 0, sc);
  // Elf64_Dyn  
  int size = D[index].alloc_size;
  // sizeof(Elf64_Dyn) * DEFAULT_DYNAMIC_ENTRY_NUM;
  void* d = __malloc(size);
  D[index].data_p = d;
  shdr->sh_offset = d;
  D[index].size_p = &(shdr->sh_size);
  // shdr->sh_size = size;
  return d;
}


void add_dynamic() {

  char max_name[50] = {};
  sprintf(max_name, "[link/elf/dynamic.c]\t add dynamic\n");
  logger_emit("misc.log", max_name);

  set_dynamic_alloc_size();
  
  // dynamic vector which is refferred from another program header.
  add_dynamic_sc(0, ".dynamic");
  if (use_init_array) {
    add_dynamic_sc(1, ".init_array");
  }
  if (use_fini_array) {
    add_dynamic_sc(2, ".fini_array");
  }  
  add_dynamic_sc(3, ".dt.hash");
  add_dynamic_sc(4, ".rela.plt");
  // these are needed for hiding 
  add_dynamic_sc(5, ".dynsym");
  add_dynamic_sc(6, ".dynstr");
  
  *(uint32_t*)D[DT_HASH_INDEX].data_p = NBUCKET;  
  add_dynsym_entry(0, 0, 0);
  
  // dependencies should be added after relocation.
  // add_deps();
  // these are not relavant to relocation.
  // but, still it needs to be done after fixing virtual address.
  
  // add_init();
  // add_fini();
  /* add_fini(); */
  
  /* add_fini_array(); */
  
  /* add_plt(); */
  /* add_got(); */
  
  // these entry should be added when you resolve something on an external shared library.
  // add_rela_dyn();
  // 
  
}

