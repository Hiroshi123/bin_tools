
#include <stdint.h>
#include <stdio.h>

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

static SectionContainer* PLT_GOT_P = 0;
static uint8_t use_init_array = 1;
static uint8_t use_fini_array = 1;

#define ALLOCATE_SIZE 0x1000
#define NBUCKET 10
#define DEFAULT_ENTRY_NUM 14

#define DT_DYNAMIC_INDEX 0
#define INIT_ARRAY_INDEX 1
#define FINI_ARRAY_INDEX 2
#define DT_HASH_INDEX 3
#define RELA_PLT_INDEX 4
#define DYNSYM_INDEX 5
#define DYNSTR_INDEX 6
#define PLT_GOT_INDEX 7
#define GNU_HASH_INDEX 8
#define DT_NEEDED_INDEX 9

static char* DT_DYNAMIC_NAME = ".dynamic";
static char* DT_INIT_ARRAY_NAME = ".init_array";
static char* DT_FINI_ARRAY_NAME = ".fini_array";
static char* DT_HASH_NAME = ".dt.hash";
static char* DT_GNU_HASH_NAME = ".gnu.hash";
static char* DT_RELA_PLT_NAME = ".rela.plt";
static char* DT_DYNSYM_NAME = ".dynsym";
static char* DT_DYNSTR_NAME = ".dynstr";

struct DynamicParams {
  SectionContainer* sc_p;
  uint32_t* size_p;
  uint8_t* data_p;
  union {
    uint32_t size;
    uint32_t num;
  };
  uint32_t alloc_size;
  uint32_t remain_size;
  char* name;
};

struct DynamicParams D[11];

static int DEFAULT_DYNAMIC_ENTRY_NUM = 0x3000;
static int DEFAULT_DYNSYM_NUM = 0x3000;
static int DEFAULT_DYNSTR_NUM = 0x3000;
static int DEFAULT_PLTGOT_NUM = 0x3000;
static int DEFAULT_RELA_PLT_NUM = 0x3000;
static int DEFAULT_DT_HASH_CHAIN_NUM = 0x3000;
static int DEFAULT_INIT_ARRAY_NUM = 0x10;

static uint32_t DynamicEntryTag[DEFAULT_ENTRY_NUM] = {
    DT_NEEDED,  // no need
    DT_INIT_ARRAY, DT_INIT_ARRAYSZ, DT_FINI_ARRAY, DT_FINI_ARRAYSZ,
    DT_GNU_HASH,  // or DT_HASH
    DT_RELA,      // need
    DT_RELASZ,    // no need
    DT_RELAENT,   // no need
    DT_SYMTAB,    // need
    DT_SYMENT,    // no need
    DT_STRTAB,    // no need
    DT_STRSZ,     // need
    DT_NULL,
};

static int need_allocate(int tag) {
  switch (tag) {
    case DT_INIT_ARRAY:
      return INIT_ARRAY_INDEX;
    case DT_FINI_ARRAY:
      return FINI_ARRAY_INDEX;
    case DT_GNU_HASH:
      return GNU_HASH_INDEX;
    case DT_RELA:
      return RELA_PLT_INDEX;
    case DT_SYMTAB:
      return DYNSYM_INDEX;
    case DT_STRSZ:
      return DYNSTR_INDEX;
    default:
      break;
      /* case DT_NEEDED: */
      /* case DT_RELASZ: */
      /* case DT_RELAENT: */
      /* case DT_SYMENT: */
      /* case DT_STRTAB: */
      /*   return 0;     */
  }
  return 0;
}

// what this function does is essentially put default dynamic headers on the
// area where dynamic entry on program header specifies.
//
static void init_dynamic() {
  // D[10];
  D[DT_DYNAMIC_INDEX].alloc_size = sizeof(Elf64_Dyn) * 0x20;
  D[DT_DYNAMIC_INDEX].remain_size = sizeof(Elf64_Dyn) * 0x20;
  D[DT_DYNAMIC_INDEX].name = DT_DYNAMIC_NAME;

  D[INIT_ARRAY_INDEX].alloc_size = 0x20;
  D[INIT_ARRAY_INDEX].remain_size = 0x20;
  D[INIT_ARRAY_INDEX].name = DT_INIT_ARRAY_NAME;

  D[FINI_ARRAY_INDEX].alloc_size = 0x20;
  D[INIT_ARRAY_INDEX].remain_size = 0x20;
  D[FINI_ARRAY_INDEX].name = DT_FINI_ARRAY_NAME;

  int nchain_max = DEFAULT_DT_HASH_CHAIN_NUM * sizeof(uint32_t);

  int size;
  if (Confp->use_dt_hash) {
    // DT_HASH initialization
    int bucket_size = sizeof(uint32_t);
    int nchain_size = sizeof(uint32_t);
    int nbucket = sizeof(uint32_t) * (NBUCKET + 1);
    size = bucket_size + nchain_size + nbucket;
    D[DT_HASH_INDEX].alloc_size = size + nchain_max;
    D[DT_HASH_INDEX].size = size;
    D[DT_HASH_INDEX].name = DT_HASH_NAME;
    int i = 0;
    for (; i < Confp->dynamic_entry_num; i++) {
      if (DynamicEntryTag[i] == DT_GNU_HASH) DynamicEntryTag[i] = DT_HASH;
    }
    // DynamicEntryTag
    //
  } else {
    // GNU_HASH initialization
    gnu_hash_table* hash_table_p = D[GNU_HASH_INDEX].data_p;
    int bloom = (Confp->hash_table_param.bloom_size + 1) * sizeof(size_t);
    int bucket = (Confp->hash_table_param.nbucket /*nbucket*/) * sizeof(size_t);
    size = sizeof(gnu_hash_table) + bloom + bucket;
    D[GNU_HASH_INDEX].alloc_size = size + nchain_max;
    D[GNU_HASH_INDEX].remain_size = size + nchain_max;
    D[GNU_HASH_INDEX].size = size;
    D[GNU_HASH_INDEX].name = DT_GNU_HASH_NAME;
  }

  D[RELA_PLT_INDEX].alloc_size = DEFAULT_RELA_PLT_NUM * sizeof(Elf64_Rela);
  D[RELA_PLT_INDEX].remain_size = DEFAULT_RELA_PLT_NUM * sizeof(Elf64_Rela);
  D[RELA_PLT_INDEX].name = DT_RELA_PLT_NAME;

  //
  D[DYNSYM_INDEX].alloc_size = DEFAULT_DYNSYM_NUM * sizeof(Elf64_Sym);
  D[DYNSYM_INDEX].remain_size = DEFAULT_DYNSYM_NUM * sizeof(Elf64_Sym);
  D[DYNSYM_INDEX].name = DT_DYNSYM_NAME;

  //
  D[DYNSTR_INDEX].alloc_size = DEFAULT_DYNSTR_NUM;
  D[DYNSTR_INDEX].remain_size = DEFAULT_DYNSTR_NUM;
  D[DYNSTR_INDEX].size = 1;
  D[DYNSTR_INDEX].name = DT_DYNSTR_NAME;
}

static void* add_dynamic_sc(int index, char* _name, SectionContainer* _sc) {
  char* name = _name != 0 ? _name : D[index].name;
  SectionContainer* sc;
  if (!_sc) {
    sc = __z__link__alloc_section_container(0, name, 0, 0);
    D[index].sc_p = sc;
  } else {
    // set (maximum) size for previous one as it is full.
    *D[index].size_p = D[index].size;
    sc = _sc;
  }
  Elf64_Shdr* shdr =
      __z__link__alloc_elf_section(SHT_DYNAMIC, SHF_ALLOC | SHF_WRITE);
  __z__link__alloc_section_chain(shdr, 0, sc, 0);
  void* d = __malloc(D[index].alloc_size);
  D[index].data_p = d;
  shdr->sh_offset = d;
  D[index].size_p = &(shdr->sh_size);
  return d;
}

/* static void add_dt_hash_entry(DtHashTable* hash_table_p, char* name, int
 * sym_index) { */
/*   uint32_t hash = sysv_hash(name); */
/*   int mod = hash % NBUCKET; */
/*   // DtHashTable* hash_table_p = D[DT_HASH_INDEX].data_p; */
/*   hash_table_p->nchain ++; */
/*   uint32_t* bucket = hash_table_p + 1; */
/*   uint32_t* chain = bucket + hash_table_p->nbucket; */
/*   check_dt_hash_collision(bucket, chain, mod, sym_index); */
/*   // TODO :: If size is not enough, it should allocate new area reffered from
 * new section chain. */
/*   // But chain is referred from bucket and will be collappsed. */
/*   /\* if (D[DT_HASH_INDEX].size == D[DT_HASH_INDEX].alloc_size) { *\/ */
/*   /\*   add_dynamic_sc(DT_HASH_INDEX, 0, D[DT_HASH_INDEX].sc_p); *\/ */
/*   /\* } *\/ */
/* } */

static void check_gnu_hash_collision(uint32_t* bucket, uint32_t* chain,
                                     uint32_t gh, uint32_t mod,
                                     uint32_t sym_index, uint32_t sym_offset) {
  uint32_t* p = bucket + mod;
  /* printf("a:%p,%p,%p,%p\n",*p,p,bucket,mod); */
  if (*p) {
    p = chain + *p - sym_offset;
    for (; *p; p++)
      ;
    *p = gh & -2;
  } else {
    *p = sym_index;  // gh | 1;
    *(chain + sym_index - sym_offset) = gh & -2;
    /* printf("set sym_index:%p\n", sym_index); */
    /* printf("b,%p\n", gh & -2); */
  }
}

static void set_bloom_bits(uint32_t gh) {
  gnu_hash_table* hash_table_p = D[GNU_HASH_INDEX].data_p;
  size_t shift1 = gh % (8 * sizeof(size_t));
  size_t shift2 = (gh >> hash_table_p->bloom_shift) % (8 * sizeof(size_t));
  size_t bit1 = 1ul << shift1;
  size_t bit2 = 1ul << shift2;

  // 32bit hash is devided by 64 or 32 and get the index of bloom vector.
  uint32_t index = gh / (8 * sizeof(size_t));
  size_t* vector =
      &hash_table_p->bloom_array + (index & (hash_table_p->bloom_size - 1));
  *vector = *vector | bit1 | bit2;

  /* printf("hash:%p\n", gh); */
  /* printf("index:%p,%p\n", index, (index & (hash_table_p->bloom_size-1))); */
  /* printf("shift1:%p,%p\n", shift1, bit1); */
  /* printf("shift2:%p,%p\n", shift2, bit2); */
  /* printf("v:%p\n", *vector); */
}

static void add_gnu_hash_entry(char* name, int sym_index) {
  // printf("add gnu:%s\n", name);
  gnu_hash_table* hash_table_p = D[GNU_HASH_INDEX].data_p;
  uint32_t gh = gnu_hash(name);
  set_bloom_bits(gh);
  int mod = gh % hash_table_p->nbuckets;
  uint32_t* bucket =
      (size_t*)&hash_table_p->bloom_array + hash_table_p->bloom_size;
  uint32_t* chain = bucket + hash_table_p->nbuckets;
  check_gnu_hash_collision(bucket, chain, gh, mod, sym_index,
                           hash_table_p->symoffset);
  D[GNU_HASH_INDEX].size += 4;
  // only allocate chain when needed
  /* if (D[GNU_HASH_INDEX].size == D[GNU_HASH_INDEX].alloc_size) { */
  /*   add_dynamic_sc(GNU_HASH_INDEX, 0, D[GNU_HASH_INDEX].sc_p); */
  /* } */
}

static void add_dynsym_entry(uint32_t str_offset, uint32_t shndx,
                             uint32_t value) {
  Elf64_Sym* sym = D[DYNSYM_INDEX].data_p;
  D[DYNSYM_INDEX].data_p += sizeof(Elf64_Sym);
  // D[DYNSYM_INDEX].remain_size -= sizeof(Elf64_Sym);
  sym->st_name = str_offset;
  int bind = STB_GLOBAL;  // ELF64_ST_BIND(STB_GLOBAL);
  int type = STT_FUNC;    // ELF64_ST_TYPE(STT_FUNC);
  sym->st_info = ELF64_ST_INFO(bind, type);
  sym->st_other = ELF64_ST_VISIBILITY(STV_DEFAULT);
  sym->st_shndx = shndx;
  sym->st_value = value;
  sym->st_size = 0;
  D[DYNSYM_INDEX].num += 1;
  if (D[DYNSYM_INDEX].num * sizeof(Elf64_Sym) == D[DYNSYM_INDEX].alloc_size) {
    D[DYNSYM_INDEX].size = D[DYNSYM_INDEX].alloc_size;
    add_dynamic_sc(DYNSYM_INDEX, 0, D[DYNSYM_INDEX].sc_p);
  } else if (D[DYNSYM_INDEX].remain_size < 0) {
    printf("allocation size needs to be fixed as moduler.\n");
  }
}

// returns head offset of the allocated string.
static uint32_t add_dynstr_entry(char* name) {
  char* p = D[DYNSTR_INDEX].data_p + D[DYNSTR_INDEX].size;
  // if it is likely to exceed the remaining size, leave the left as blank,
  // and start to fill on the new one.
  if (D[DYNSTR_INDEX].alloc_size < D[DYNSTR_INDEX].size + strlen(name)) {
    add_dynamic_sc(DYNSTR_INDEX, 0, D[DYNSTR_INDEX].sc_p);
    p = D[DYNSTR_INDEX].data_p;
  }
  uint32_t r = D[DYNSTR_INDEX].size;
  for (; *name != 0; p++, name++) {
    *p = *name;
  }
  D[DYNSTR_INDEX].size = (p + 1) - (char*)D[DYNSTR_INDEX].data_p;
  return r;
}

// return heads pointer
static uint32_t add_pltgot_entry() {
  uint8_t* s = D[PLT_GOT_INDEX].data_p;
  uint32_t r = PLT_GOT_P->virtual_address + D[PLT_GOT_INDEX].num * 0x10;
  D[PLT_GOT_INDEX].num++;
  D[PLT_GOT_INDEX].data_p += 0x10;
  static uint8_t p[] = {0xff, 0x25, 0x2, 0x0, 0x0, 0x0};
  int i = 0;
  for (; i < 6; s++, i++) {
    *s = p[i];
  }
  if (D[PLT_GOT_INDEX].num * 0x10 == D[PLT_GOT_INDEX].alloc_size) {
    D[PLT_GOT_INDEX].size = D[PLT_GOT_INDEX].alloc_size;
    add_dynamic_sc(PLT_GOT_INDEX, 0, PLT_GOT_P /*D[DYNSTR_INDEX].sc_p*/);
  }
  return r;
}

static void* add_rela_plt_entry(size_t offset, int sym_index, int _type,
                                size_t addend) {
  Elf64_Rela* d = D[RELA_PLT_INDEX].data_p;
  D[RELA_PLT_INDEX].data_p += sizeof(Elf64_Rela);
  D[RELA_PLT_INDEX].size += sizeof(Elf64_Rela);
  d->r_offset = offset - Confp->base_address;
  int sym = sym_index;  // D[DYNSYM_INDEX].num;
  int type = ELF64_R_TYPE(_type);

  d->r_info = ELF64_R_INFO(sym, type);
  printf("debug:%p,%p,%p\n", d->r_offset, d->r_info, d->r_addend);

  d->r_addend = addend - Confp->base_address;
  if (D[RELA_PLT_INDEX].size == D[RELA_PLT_INDEX].alloc_size) {
    add_dynamic_sc(RELA_PLT_INDEX, 0, D[RELA_PLT_INDEX].sc_p);
  }
  return d;
}

void* __z__link__add_rela_plt_entry(size_t offset, int sym_index, int _type,
                                    size_t addend) {
  return add_rela_plt_entry(offset, sym_index, _type, addend);
}

static void add_dyn_relative_entry(size_t offset, size_t addend) {
  Elf64_Rela* d = D[RELA_PLT_INDEX].data_p;
  D[RELA_PLT_INDEX].data_p += sizeof(Elf64_Rela);
  D[RELA_PLT_INDEX].size += sizeof(Elf64_Rela);
  d->r_offset = offset;
  int sym = 0;
  int type = ELF64_R_TYPE(R_X86_64_RELATIVE);
  d->r_info = ELF64_R_INFO(sym, type);
  d->r_addend = addend;
  if (D[RELA_PLT_INDEX].size == D[RELA_PLT_INDEX].alloc_size) {
    add_dynamic_sc(RELA_PLT_INDEX, 0, D[RELA_PLT_INDEX].sc_p);
  }
}

uint32_t __z__link__add_dynamic_entry(char* str, int type, int check) {
  uint32_t r;
  void* rela = 0;
  if (check) rela = __z__link__elf_lookup_symbol(str, 1);
  uint8_t add_symbol_entry = 1;
  uint8_t add_alloc_symbol_chain = 1;
  if (rela) {
    // return head of plt got entry.
    return rela;
    /* if (ELF64_R_TYPE(rela->r_info) == R_X86_64_JUMP_SLOT) { */
    /*   return (size_t)rela->r_offset - 8; */
    /* } */
    /* add_symbol_entry = 0; */
    /* add_alloc_symbol_chain = 0; */
  }
  char max_name[100] = {};
  sprintf(max_name, "[link/elf/dynamic.c]\t add dynamic import symbol:%s\n",
          str);
  __z__logger_emit("misc.log", max_name);
  r = add_pltgot_entry();
  // 1. Dynamic string entry
  // 2. Dynsym entry
  if (add_symbol_entry) add_dynsym_entry(add_dynstr_entry(str), 0, 0);
  // 3. .rela.dyn or .rela.plt entry
  rela = add_rela_plt_entry(r + 8, D[DYNSYM_INDEX].num - 1, type, 0);
  // if (add_alloc_symbol_chain)
  __z__link__alloc_dynamic_symbol_chain(r, str, 0);
  return r;
}

uint32_t __z__link__add_dynamic_entry2(size_t offset, char* str) {
  char max_name[100] = {};
  sprintf(max_name, "[link/elf/dynamic.c]\t add dynamic GLOB_DAT symbol:%s\n",
          str);
  __z__logger_emit("misc.log", max_name);
  add_dynsym_entry(add_dynstr_entry(str), 0, 0);
  Elf64_Rela* rela =
      add_rela_plt_entry(offset, D[DYNSYM_INDEX].num - 1, R_X86_64_GLOB_DAT, 0);
  // alloc_dynamic_symbol_chain(rela, str, 0);
  return rela;
}

void __z__link__add_pltgot_sc(int index, char* name) {
  if (Confp->nodynamic == 1) return;

  // 0x10 will be entry size
  SectionContainer* sc = __z__link__alloc_section_container(0, name, 0, 0);
  PLT_GOT_P = sc;
  // SHT_DYNSYM
  Elf64_Shdr* shdr =
      __z__link__alloc_elf_section(SHT_DYNAMIC, SHF_ALLOC | SHF_WRITE);

  __z__link__alloc_section_chain(shdr, 0, sc, 0);
  int size = 0x10 * DEFAULT_PLTGOT_NUM;
  void* d = __malloc(size);
  D[index].data_p = d;

  shdr->sh_offset = d;

  D[index].size_p = &(shdr->sh_size);
  // shdr->sh_size = size;
}

void add_init() {}

void add_fini() {}

void __z__link__add_init_array(void* addr) {
  *(size_t*)(D[INIT_ARRAY_INDEX].data_p) = addr;
  D[INIT_ARRAY_INDEX].data_p += 8;
  D[INIT_ARRAY_INDEX].size += 8;
  // if the remaining size is not left, allocate new section as another
  // SectionChain

  if (D[INIT_ARRAY_INDEX].size == D[INIT_ARRAY_INDEX].alloc_size) {
    add_dynamic_sc(INIT_ARRAY_INDEX, 0, D[INIT_ARRAY_INDEX].sc_p);
  } else if (D[INIT_ARRAY_INDEX].remain_size < 0) {
    printf("error\n");
  }
}

void __z__link__add_fini_array(void* addr) {
  *(size_t*)(D[FINI_ARRAY_INDEX].data_p) = addr;
  D[FINI_ARRAY_INDEX].data_p += 8;
  D[FINI_ARRAY_INDEX].size += 8;

  /* if (D[FINI_ARRAY_INDEX].size == D[FINI_ARRAY_INDEX].alloc_size) { */
  /*   add_dynamic_sc(FINI_ARRAY_INDEX, 0, D[FINI_ARRAY_INDEX].sc_p); */
  /* } else if (D[FINI_ARRAY_INDEX].remain_size < 0) { */
  /*   printf("error\n"); */
  /* } */
}

// this function is going to be called through iteration of object chain
// main task here is to add export symbol on gnu hash entry for subsequent
// relocation.
void __z__link__add_export_symbol(void* _oc, void* arg1) {
  ObjectChain* oc = _oc;
  SymbolChain* sym = oc->symbol_chain_head;
  if (!sym) return;
  char* str = 0;
  uint32_t value = 0;
  char max_name[100] = {};
  static uint64_t* init = "____init";
  static uint64_t* fini = "____fini";
  for (; sym; sym = sym->next) {
    str = sym->name;
    // __z__link__
    SectionChain* sc1 =
        get_section_chain_by_index(((Elf64_Sym*)(sym->p))->st_shndx);
    printf("%s,%p,%p\n", str, ((Elf64_Sym*)(sym->p))->st_shndx, sc1);
    value = sc1->virtual_address + ((Elf64_Sym*)(sym->p))->st_value -
            Confp->base_address;
    sprintf(max_name,
            "[link/elf/dynamic.c]\t add export symbol:%s,%d,%p,%p,%p\n", str,
            D[DYNSYM_INDEX].num, sym->schain->virtual_address,
            sc1->virtual_address, value);
    __z__logger_emit("misc.log", max_name);

    printf("a01,%s,%p\n", str, ((Elf64_Sym*)(sym->p))->st_shndx);

    add_dynsym_entry(add_dynstr_entry(str), ((Elf64_Sym*)(sym->p))->st_shndx,
                     value);

    if (Confp->use_dt_hash) {
      __z__link__add_dt_hash_entry(D[DT_HASH_INDEX].data_p, str,
                                   D[DYNSYM_INDEX].num - 1);
      D[DT_HASH_INDEX].size += 4;
    } else {
      add_gnu_hash_entry(str, D[DYNSYM_INDEX].num - 1);
    }
    if (*(uint64_t*)str == *init) {
      // use_init_array = 1;
      // add_init_array(value + Confp->base_address);
    }
    if (*(uint64_t*)str == *fini) {
      // use_init_array = 1;
      // add_fini_array(value + Confp->base_address);
    }
    // so far, function which offers entry point address is limited by global
    // functions which are subject of relocation.
    //
    if (Confp->entry_function_name) {
      if (strcmp(Confp->entry_function_name, str) == 0) {
        Confp->entry_address = value;
        printf("%s!%p\n", (size_t*)(str), Confp->entry_address);
      }
    }
  }
}

static uint32_t set_dynamic_entry(Elf64_Dyn* dyn, int tag, uint32_t offset) {
  dyn->d_tag = tag;
  switch (tag) {
    case DT_NEEDED:
      dyn->d_un.d_val = offset;
      break;
    case DT_INIT_ARRAY:
      dyn->d_un.d_val = offset;  // D[INIT_ARRAY_INDEX].data_p;
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
    case DT_GNU_HASH:
      dyn->d_un.d_val = offset;
      offset += D[GNU_HASH_INDEX].size;
      *D[GNU_HASH_INDEX].size_p = D[GNU_HASH_INDEX].size;
      break;
    case DT_RELA:
      dyn->d_un.d_val = offset;
      // RELA_PLT_INDEX might be more than PLT_GOT_INDEX.
      *D[RELA_PLT_INDEX].size_p = D[RELA_PLT_INDEX].size;
      offset += D[RELA_PLT_INDEX].size;
      // *D[RELA_PLT_INDEX].size_p = D[PLT_GOT_INDEX].num * sizeof(Elf64_Rela);
      *D[PLT_GOT_INDEX].size_p = D[PLT_GOT_INDEX].num * 0x10;
      break;
    case DT_RELASZ:
      dyn->d_un.d_val = D[RELA_PLT_INDEX].size;
      // dyn->d_un.d_val = D[PLT_GOT_INDEX].num * sizeof(Elf64_Rela);
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
    case 0:
      break;
    default:
      // printf("tag:%p\n", tag);
      // for (;;);
      break;
  }
  return offset;
}

static void add_dt_needed(char* name) {
  char* lib = name;
  add_dynstr_entry(lib);
  D[DT_DYNAMIC_INDEX].num += 1;
}

void __z__link__add_dt_needed() {
  if (Confp->dynlib) {
    char** p = Confp->dynlib;
    for (; *p; p++) {
      printf("dt needed:%s\n", *p);
      if (memmem(*p, strlen(*p), ".so", 3)) {
        add_dt_needed(*p);
      } else {
        break;
      }
    }
  }
}

void __z__link__set_dynanmic() {
  if (Confp->nodynamic) {
    Confp->out_size =
        Confp->virtual_address_offset - Confp->base_address + 0x10;
    return;
  }
  uint32_t r = PLT_GOT_P->virtual_address + D[PLT_GOT_INDEX].num * 0x10 -
               Confp->base_address;
  ;
  Elf64_Dyn* dyn = D[DT_DYNAMIC_INDEX].data_p;
  int i = 0;
  uint32_t p = 1;
  uint8_t* q = 0;
  for (; i < D[DT_DYNAMIC_INDEX].num; i++) {
    set_dynamic_entry(dyn, DT_NEEDED, p);
    q = D[DYNSTR_INDEX].data_p + p;
    // go to next entry
    for (p++; *q; q++, p++)
      ;
    dyn++;
    Confp->dynamic_entry_num++;
  }
  r += Confp->dynamic_entry_num * sizeof(Elf64_Dyn);
  *D[0].size_p = Confp->dynamic_entry_num * sizeof(Elf64_Dyn);
  // for (;;);

  i = 1;
  for (; i <= Confp->dynamic_entry_num; i++) {
    printf("dent:%p,%p\n", DynamicEntryTag[i], r);
    r = set_dynamic_entry(dyn, DynamicEntryTag[i], r);
    dyn++;
  }
  Confp->out_size = r;  // - Confp->base_address;
}

static alloc_dynamic() {
  int i = 0;
  int res = 0;
  for (; i < DEFAULT_ENTRY_NUM; i++) {
    res = need_allocate(DynamicEntryTag[i]);
    if (res) {
      add_dynamic_sc(res, 0, 0);
    } else {
      // printf("no add entry:%d\n", DynamicEntryTag[i]);
    }
  }
}

static void add_dynamic() {
  char max_name[50] = {};
  sprintf(max_name, "[link/elf/dynamic.c]\t add dynamic\n");
  __z__logger_emit("misc.log", max_name);

  init_dynamic();
  // dynamic vector which is refferred from another program header.
  add_dynamic_sc(DT_DYNAMIC_INDEX, DT_DYNAMIC_NAME, 0);
  alloc_dynamic();

  if (Confp->use_dt_hash) {
    // dt_hash_initialization
    *(uint32_t*)D[DT_HASH_INDEX].data_p = NBUCKET;
  } else {
    // gnu_hash_initialization
    gnu_hash_table* gh = D[GNU_HASH_INDEX].data_p;
    gh->nbuckets = Confp->hash_table_param.nbucket;
    gh->symoffset = 1;
    gh->bloom_size = Confp->hash_table_param.bloom_size;
    gh->bloom_shift = Confp->hash_table_param.bloom_shift;
  }
  // dynamic symbol initialization
  Confp->dynstr_head = D[DYNSTR_INDEX].data_p;
  Confp->dynsym_head = D[DYNSYM_INDEX].data_p;
  Confp->gnu_hash_head = D[GNU_HASH_INDEX].data_p;

  add_dynsym_entry(0, 0, 0);
}

void __z__link__add_dynamic() {
  if (Confp->nodynamic == 0) {
    add_dynamic();
  } else {
    printf("no-dynamic is not yet implemented\n");
    // add_dynamic_sc();
  }
}
