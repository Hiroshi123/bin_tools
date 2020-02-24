
#include <stdio.h>
#include <stdint.h>

#include "alloc.h"
#include "elf.h"
#include "link.h"

extern Config* Confp;

void alloc_symbol_chain(void* is, char* name, int shndx) {

  // Confp->HashTable.bucket
  // is;
  size_t* table_index = (Confp->HashTable.bucket + (elf_hash(name) % Confp->HashTable.nbucket));
  SectionChain* schain = Confp->current_object->section_chain_head;
  int i = 0;
  for (;schain && i == shndx;schain = schain->next,i++);
  printf("alloc symbol chain,%p,%p,%d,%p\n",
	 table_index, Confp->current_object, shndx, schain);  
  SymbolChain* chain = __malloc(sizeof(SymbolChain));
  chain->name = name;
  chain->p = is;
  chain->schain = schain;
  *table_index = chain;  
  // (!Confp->current_object->symbol_chain_head)
  // *table_index
  // size_t* ret = lookup_symbol(name, 0);
  // if (ret) *ret = chain;
  
}

void alloc_dynamic_symbol(void* a, void* b) {
  
}

