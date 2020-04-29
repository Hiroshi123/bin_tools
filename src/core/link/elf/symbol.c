
#include <stdio.h>
#include <stdint.h>

#include "alloc.h"
#include "elf.h"
#include "link.h"

extern Config* Confp;

#define COMPUTE_HASH(X) (Confp->HashTable.bucket + (elf_hash(X) % Confp->HashTable.nbucket))

void alloc_symbol_chain(void* is, char* name, int shndx) {

  // Confp->HashTable.bucket
  // is;
  size_t* table_index = COMPUTE_HASH(name);
  SectionChain* schain = Confp->current_object->section_chain_head;
  int i = 0;
  for (;schain && i == shndx;schain = schain->next,i++);
  char max_name[100] = {};
  sprintf(max_name, "[link/elf/symbol.c]\t alloc symbol chain\n");
  logger_emit("misc.log", max_name);
  /* printf("alloc symbol chain,%p,%p,%d,%p\n", */
  /* 	 table_index, Confp->current_object, shndx, schain);   */
  SymbolChain* chain = __malloc(sizeof(SymbolChain));
  chain->name = name;
  chain->p = is;
  chain->schain = schain;
  *table_index = chain;
  if (!Confp->current_object->symbol_chain_head) {
    Confp->current_object->symbol_chain_head = chain;
    Confp->current_object->symbol_chain_tail = chain;
  } else {
    Confp->current_object->symbol_chain_tail->next = chain;
    Confp->current_object->symbol_chain_tail = chain;    
  }
  Confp->current_object->export_symbol_num ++;
  // (!Confp->current_object->symbol_chain_head)
  // *table_index
  // size_t* ret = lookup_symbol(name, 0);
  // if (ret) *ret = chain;
  
}

void iterate_symbol_chain() {

  SymbolChain* sym1 = Confp->current_object->symbol_chain_head;
  SymbolChain* sym = sym1->next;
  if (sym) {
    printf("%p\n", sym);
    printf("!!%s\n", sym->name);
    printf("%p\n", sym->schain);
    printf("%p\n", sym->p);
    Elf64_Sym* p = sym->p;
    printf("val:%p\n", p->st_value);
    SectionChain* schain = sym->schain;
    printf("%p\n", schain->virtual_address + p->st_value);
  }
  
  /* SectionChain* schain = Confp->current_object->section_chain_head; */
  /* int i = 0; */
  /* printf("a\n"); */
  /* for (;schain;schain = schain->next,i++) { */
  /*   printf("pp::%p\n", schain->p); */
  /* } */
  
}

void alloc_dynamic_symbol(void* a, void* b) {
  
}

