
#ifdef _WIN32
#include <windows.h>

#include "coff.h"

#endif
#include <stdint.h>
#include <stdio.h>

#include "alloc.h"
#include "elf.h"
#include "link.h"
#include "objformat.h"

extern Config* Confp;

uint32_t elf_hash(const uint8_t* name) {
  uint32_t h = 0, g;
  for (; *name; name++) {
    h = (h << 4) + *name;
    if (g = h & 0xf0000000) {
      h ^= g >> 24;
    }
    h &= ~g;
  }
  return h;
}

uint32_t sysv_hash(const char* s0) {
  const unsigned char* s = (void*)s0;
  uint_fast32_t h = 0;
  while (*s) {
    h = 16 * h + *s++;
    h ^= h >> 24 & 0xf0;
  }
  return h & 0xfffffff;
}

uint32_t gnu_hash(const char* s0) {
  const unsigned char* s = (void*)s0;
  uint_fast32_t h = 5381;
  for (; *s; s++) h += h * 32 + *s;
  return h;
}

/* static Sym *sysv_lookup(const char *s, uint32_t h, struct dso *dso) */
/* { */
/*   size_t i; */
/*   Sym *syms = dso->syms; */
/*   Elf_Symndx *hashtab = dso->hashtab; */
/*   char *strings = dso->strings; */
/*   for (i=hashtab[2+h%hashtab[0]]; i; i=hashtab[2+hashtab[0]+i]) { */
/*     if ((!dso->versym || dso->versym[i] >= 0) */
/* 	&& (!strcmp(s, strings+syms[i].st_name))) */
/*       return syms+i; */
/*   } */
/*   return 0; */
/* } */

static int gnu_lookup(uint32_t h1, uint32_t* hashtab, const char* s) {
  uint32_t nbuckets = hashtab[0];
  uint32_t* buckets = hashtab + 4 + hashtab[2] * (sizeof(size_t) / 4);
  uint32_t i = buckets[h1 % nbuckets];
  Elf64_Sym* sym = Confp->dynsym_head;

  if (!i) return 0;
  uint32_t* hashval = buckets + nbuckets + (i - hashtab[1]);
  for (h1 |= 1;; i++) {
    uint32_t h2 = *hashval++;
    // && (!dso->versym || dso->versym[i] >= 0)
    if ((h1 == (h2 | 1)) &&
        !strcmp(s, Confp->dynstr_head +
                       sym[i].st_name /*dso->strings + dso->syms[i].st_name*/))
      return i;
    if (h2 & 1) break;
  }
  return 0;
}

int sysv_lookup_for_coff(
    const char* s) {  //, void* base, uint32_t* address_of_function) {

  uint32_t* syms = Confp->export_data.address_of_names;
  uint32_t* fp = Confp->export_data.address_of_functions;
  size_t export_vaddr_p = Confp->export_data.vaddr_p;
  char* ied_p = Confp->export_data.ied_p;
  // IMAGE_EXPORT_DIRECTORY* ied = Confp->export_data_p;
  // ied->
  size_t i;
  uint32_t h = sysv_hash(s);
  // uint32_t* syms = address_of_function;
  uint32_t* hashtab = Confp->hash_table_p;
  // char *strings = base;
  for (i = hashtab[2 + h % hashtab[0]]; i; i = hashtab[2 + hashtab[0] + i]) {
    if (  //(!dso->versym || dso->versym[i] >= 0) &&
        (!strcmp(s, ied_p + syms[i] - export_vaddr_p)))
      return fp[i];
  }
  return 0;
}

int sysv_lookup(const char* s, uint32_t h /*struct dso *dso*/) {
  size_t i;
  Elf64_Sym* syms = Confp->dynsym_head;  // dso->syms;
  // Elf_Symndx
  uint32_t* hashtab = Confp->hash_table_p;  // dso->hashtab;
  char* strings = Confp->dynstr_head;       // dso->strings;
  for (i = hashtab[2 + h % hashtab[0]]; i; i = hashtab[2 + hashtab[0] + i]) {
    if (  //(!dso->versym || dso->versym[i] >= 0) &&
        (!strcmp(s, strings + syms[i].st_name)))
      return i;
  }
  return 0;
}

static int gnu_lookup_filtered(uint32_t h1, uint32_t* hashtab, const char* s,
                               uint32_t fofs, size_t fmask) {
  const size_t* bloomwords = (const void*)(hashtab + 4);
  size_t f = bloomwords[fofs & (hashtab[2] - 1)];
  if (!(f & fmask)) return 0;
  f >>= (h1 >> hashtab[3]) % (8 * sizeof f);
  if (!(f & 1)) return 0;
  return gnu_lookup(h1, hashtab, s);
}

int find_sym2(char* s) {
  uint32_t h = 0, gh = gnu_hash(s), gho = gh / (8 * sizeof(size_t)), *ght;
  size_t ghm = 1ul << gh % (8 * sizeof(size_t));
  // TODO :: DTHUSH support
  ght = Confp->gnu_hash_head;
  int p = gnu_lookup_filtered(gh, ght, s, gho, ghm);
  return p;
}

int __z__link__find_sym2(char* s) { return find_sym2(s); }

uint8_t isSqlite(uint64_t* p) {
  // printf("%p\n", *(p+1));
  if (*p == 0x66206574694C5153 && *(p + 1) == 0x00332074616D726F) {
    return 1;
  }
  return 0;
}

void* get_next_table(uint8_t* p, char** name) {
  /* printf("%p,%p\n",p + 0xd, *p + 0xd); */
  uint8_t* strbegin = p + 0xd;
  /* printf("%p,%p\n",p + 0x4, (*(p + 0x4) - 13)/ 2); */
  uint8_t len = (*(p + 0x4) - 13) / 2;
  *(strbegin + len) = 0;
  /* printf("%s\n", strbegin); */
  *name = strbegin;
  return p + *p + 2;
}

void* get_next_record(uint8_t* p, char** name) {
  /* printf("%p,%p,%p\n",p + 0x5, *p,*(p + 0x5)); */
  uint8_t* strbegin = p + 0x5;
  uint8_t len = *p - 3;
  uint8_t* a = __malloc(len + 1);
  uint8_t* a1 = a;
  uint8_t* _p = strbegin;
  uint8_t i = 0;
  for (; i < len; i++, _p++, a++) {
    *a = *_p;
  }
  // uint8_t* _name = *name;
  *name = a1;
  // memcpy(strbegin, a, len);
  /* printf("%p,%s,%d\n", a1,a1, len); */
  return p + *p + 2;
}

void iterate_table(uint8_t* p) {
  uint8_t* p1 = p;
  // TODO :: https://www.sqlite.org/fileformat.html
  // DataBaseHeader should be skipped.
  p += 100;
  // Probably, next is [1.6. B-tree Pages]
  // The two-byte integer at offset 5 designates the start of the cell content
  // area
  p += 5;
  printf("[%p]\n", *(uint16_t*)p);
  void* t = p1 + (256 * *p + *(p + 1));
  uint8_t i = 0;
  uint8_t* r;
  uint8_t* r_begin;
  char* table_name;
  uint8_t* record_name;
  for (;; i++) {
    t = get_next_table(t, &table_name);
    r = (p1 + 0x4000 - i * 0x1000);
    r_begin = r;
    r += 256 * *(r + 5) + *(r + 6);
    for (;;) {
      r = get_next_record(r, &record_name);
      printf("rec:tablee,%s,%s\n", record_name, table_name);
      alloc_dynamic_symbol(record_name, table_name);
      if (r_begin + 0x1000 <= r) {
        break;
      }
    }
    if (p1 + 0x1000 <= t) {
      break;
    }
  }
}

static void check_dt_hash_collision(uint32_t* bucket, uint32_t* chain,
                                    uint32_t mod, uint32_t sym_index) {
  uint32_t* p = 0;
  if (*(bucket + mod)) {
    // printf("collision:%p,%p\n", *(bucket+mod), sym_index);
    // printf("chain:%p\n", chain + *(bucket+mod));
    for (p = chain + *(bucket + mod); *p; p = chain + *p)
      ;
    *p = sym_index;
  } else {
    // no collision
    *(bucket + mod) = sym_index;
  }
}

void add_dt_hash_entry(DtHashTable* hash_table_p, char* name, int sym_index) {
  uint32_t hash = sysv_hash(name);
  int mod = hash % hash_table_p->nbucket;
  // DtHashTable* hash_table_p = D[DT_HASH_INDEX].data_p;
  hash_table_p->nchain++;
  uint32_t* bucket = hash_table_p + 1;
  uint32_t* chain = bucket + hash_table_p->nbucket;
  check_dt_hash_collision(bucket, chain, mod, sym_index);
  // TODO :: If size is not enough, it should allocate new area reffered from
  // new section chain. But chain is referred from bucket and will be
  // collappsed.
  /* if (D[DT_HASH_INDEX].size == D[DT_HASH_INDEX].alloc_size) { */
  /*   add_dynamic_sc(DT_HASH_INDEX, 0, D[DT_HASH_INDEX].sc_p); */
  /* } */
}

void __z__link__add_dt_hash_entry(DtHashTable* hash_table_p, char* name,
                                  int sym_index) {
  return add_dt_hash_entry(hash_table_p, name, sym_index);
}

void init_hashtable(char* fname) {
  if (Confp->use_dt_hash) {
    DtHashTable* dt = __malloc(1000);
    Confp->hash_table_p = dt;
    dt->nbucket = 100;
    dt->nchain = 0;
  }

  Confp->ExportHashTable.nbucket = 100;
  Confp->ExportHashTable.nchain = 0;
  int hashSize = Confp->ExportHashTable.nbucket * sizeof(void*);
  Confp->ExportHashTable.bucket = __malloc(hashSize);
  memset(Confp->ExportHashTable.bucket, 0, hashSize);

  Confp->DynamicImportHashTable.nbucket = 10;
  Confp->DynamicImportHashTable.nchain = 0;
  hashSize = Confp->DynamicImportHashTable.nbucket * sizeof(void*);
  Confp->DynamicImportHashTable.bucket = __malloc(hashSize);
  // memset(Confp->DynamicImportHashTable.bucket, 0, hashSize);

  if (Confp->file_format == ELF32 || Confp->file_format == ELF64) {
    return;
  }

  Confp->DLLHashTable.nbucket = 10;
  Confp->DLLHashTable.nchain = 0;
  hashSize = Confp->DLLHashTable.nbucket * sizeof(void*);
  Confp->DLLHashTable.bucket = __malloc(hashSize);
  memset(Confp->DLLHashTable.bucket, 0, hashSize);

  uint8_t* p = __z__mem__alloc_file(fname);
  if (0 /*isSqlite(p)*/) {
    iterate_table(p);
  } else {
    alloc_dynamic_symbol("f1", "ex65");
    alloc_dynamic_symbol("ExitProcess", "kernel32");
    alloc_dynamic_symbol("GetStdHandle", "kernel32");
    alloc_dynamic_symbol("WriteFile", "kernel32");
    alloc_dynamic_symbol("MessageBoxA", "user32");
  }
}
