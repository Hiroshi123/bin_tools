

#include <windows.h>
#include <stdio.h>
#include <stdint.h>
#include "win_memory.h"

#include "coff.h"
#include "link.h"

struct SymbolHashTable HashTable = {};
struct SymbolHashTable DLLHashTable = {};

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

void init_hashtable() {
  
  HashTable.nbucket = 100;
  HashTable.nchain = 0;
  int hashSize = HashTable.nbucket*sizeof(void*);
  HashTable.bucket = __malloc(hashSize);
  memset(HashTable.bucket, 0, hashSize);  
  DLLHashTable.nbucket = 10;
  DLLHashTable.nchain = 0;
  hashSize = DLLHashTable.nbucket*sizeof(void*);
  DLLHashTable.bucket = __malloc(hashSize);
  memset(DLLHashTable.bucket, 0, hashSize);
  alloc_dynamic_symbol("ExitProcess","kernel32");
  alloc_dynamic_symbol("GetStdHandle","kernel32");
  alloc_dynamic_symbol("WriteFile","kernel32");  
}


