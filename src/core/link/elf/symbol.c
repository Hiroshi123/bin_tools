
#include <stdint.h>
#include <stdio.h>

#include "alloc.h"
#include "elf.h"
#include "link.h"

extern Config* Confp;

SymbolChain* alloc_symbol_chain(void* is, char* name, void* schain) {
  SymbolChain* chain = __malloc(sizeof(SymbolChain));
  chain->name = name;
  chain->p = is;
  chain->schain = schain;
  return chain;
}

void alloc_export_symbol_chain(void* is, char* name, int shndx) {
  char max_name[100] = {};
  sprintf(max_name, "[link/elf/symbol.c]\t alloc export symbol chain : %s\n",
          name);
  __z__logger_emit("misc.log", max_name);

  SectionChain* schain = Confp->current_object->section_chain_head;
  int i = 0;
  for (; schain && i == shndx; schain = schain->next, i++)
    ;

  SymbolChain* chain = alloc_symbol_chain(is, name, schain);
  // DUPRECARED :: symbol table resolution had been replaced by gnu hash which
  // is also used for runtime resolution.. size_t* table_index =
  // M1(Confp->ExportHashTable, elf_hash, name); *table_index = chain;
  if (!Confp->current_object->symbol_chain_head) {
    Confp->current_object->symbol_chain_head = chain;
  } else {
    Confp->current_object->symbol_chain_tail->next = chain;
  }
  Confp->current_object->symbol_chain_tail = chain;
  Confp->current_object->export_symbol_num++;
}

void __z__link__alloc_export_symbol_chain(void* is, char* name, int shndx) {
  return alloc_export_symbol_chain(is, name, shndx);
}

void* lookup_symbol(char* name, int ht) {
  printf("lookup symbol:%s\n", name);
  uint32_t index = 0;
  size_t* table_index = 0;
  switch (ht) {
    case 0:
      M2(index, table_index, Confp->ExportHashTable, elf_hash, name);
      break;
    case 1:
      M2(index, table_index, Confp->DynamicImportHashTable, elf_hash, name);
      break;
    default:
      break;
  }
  if (*table_index) {
    // printf("bucket index:%d,%p\n",index, *table_index);
  } else {
    // printf("empty\n");
    return 0;
  }
  SymbolChain* chain = *table_index;
  for (; chain; chain = chain->next) {
    // printf("%s\n", chain->name);
    if (!strcmp(name, chain->name)) {
      // printf("matched:%s\n", chain->name);
      return chain->p;
    }
  }
  return 0;
}

void* __z__link__elf_lookup_symbol(char* name, int ht) {
  return lookup_symbol(name, ht);
}

void __z__link__alloc_dynamic_symbol_chain(void* is, char* name, void* schain) {
  SymbolChain* chain;  // = *table_index;
  // printf("alloc dynamic symbol\n");
  size_t* table_index = M1(Confp->DynamicImportHashTable, elf_hash, name);
  if (*table_index == 0) {
    *table_index = alloc_symbol_chain(is, name, schain);
    return;
  } else {
    chain = *table_index;
    for (; chain->next; chain = chain->next) {
      // printf("n:%s\n", chain->name);
    }
    chain->next = alloc_symbol_chain(is, name, schain);
  }
  return;
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
