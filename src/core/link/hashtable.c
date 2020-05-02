
#ifdef _WIN32
#include <windows.h>
#include "coff.h"

#endif
#include <stdio.h>
#include <stdint.h>

#include "alloc.h"
#include "link.h"

/* struct SymbolHashTable HashTable = {}; */
/* struct SymbolHashTable DLLHashTable = {}; */

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

uint8_t isSqlite(uint64_t* p) {
  printf("%p\n", *(p+1));
  if (*p == 0x66206574694C5153 && *(p+1) == 0x00332074616D726F) {
    return 1;
  }
  return 0;
}

void* get_next_table(uint8_t* p, char** name) {
  /* printf("%p,%p\n",p + 0xd, *p + 0xd); */
  uint8_t* strbegin = p + 0xd;
  /* printf("%p,%p\n",p + 0x4, (*(p + 0x4) - 13)/ 2); */
  uint8_t len = (*(p + 0x4) - 13)/ 2;
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
  for (;i < len;i++,_p++,a++) {
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
  // The two-byte integer at offset 5 designates the start of the cell content area
  p += 5;
  printf("[%p]\n", *(uint16_t*)p);
  void* t = p1 + (256 * *p + *(p+1));
  uint8_t i = 0;
  uint8_t* r;
  uint8_t* r_begin;
  char* table_name;
  uint8_t* record_name;
  for (;;i++) {
    t = get_next_table(t, &table_name);
    r = (p1 + 0x4000 - i * 0x1000);
    r_begin = r;
    r += 256 * *(r+5) + *(r+6);      
    for (;;) {
      r = get_next_record(r, &record_name);
      printf("rec:tablee,%s,%s\n", record_name,table_name);
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

void init_hashtable(char* fname) {

  Confp->ExportHashTable.nbucket = 100;
  Confp->ExportHashTable.nchain = 0;
  int hashSize = Confp->ExportHashTable.nbucket*sizeof(void*);
  Confp->ExportHashTable.bucket = __malloc(hashSize);
  memset(Confp->ExportHashTable.bucket, 0, hashSize);
  
  Confp->DLLHashTable.nbucket = 10;
  Confp->DLLHashTable.nchain = 0;
  hashSize = Confp->DLLHashTable.nbucket*sizeof(void*);
  Confp->DLLHashTable.bucket = __malloc(hashSize);
  memset(Confp->DLLHashTable.bucket, 0, hashSize);
  
  Confp->DynamicImportHashTable.nbucket = 10;
  Confp->DynamicImportHashTable.nchain = 0;
  hashSize = Confp->DynamicImportHashTable.nbucket*sizeof(void*);
  Confp->DynamicImportHashTable.bucket = __malloc(hashSize);
  // memset(Confp->DynamicImportHashTable.bucket, 0, hashSize);
  
  return;

  uint8_t* p = alloc_file(fname);
  if (isSqlite(p)) {
    iterate_table(p);
  } else {
    alloc_dynamic_symbol("f1","ex65");
    alloc_dynamic_symbol("ExitProcess","kernel32");
    alloc_dynamic_symbol("GetStdHandle","kernel32");
    alloc_dynamic_symbol("WriteFile","kernel32");
    alloc_dynamic_symbol("MessageBoxA","user32");
  }
}


