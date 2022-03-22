
#include <stdint.h>
#include <stdio.h>
#include <windows.h>

#include "alloc.h"
#include "coff.h"
#include "link.h"

/* extern ObjectChain* InitialObject; */
/* extern ObjectChain* CurrentObject; */
extern struct SymbolHashTable HashTable;
extern struct SymbolHashTable DLLHashTable;
extern uint32_t PltOffset;

void* lookup_symbol(char* name, size_t* address) {
  size_t* table_index =
      (HashTable.bucket + (elf_hash(name) % HashTable.nbucket));
  // if you do not find any entry on this table.
  if (*table_index == 0) {
    return address ? 0 : table_index;
  }
  SymbolChain* pre;
  SymbolChain* pre_chain;
  IMAGE_SYMBOL* is;
  ObjectChain* oc;
  char* _name;
  IMAGE_SYMBOL* begin;
  IMAGE_SYMBOL* end;
  // if you find any entry then trace the chain which was stored.
  pre_chain = *table_index;
  // check hash collision
  for (; pre_chain; pre_chain = pre_chain->next) {
    is = pre_chain->p;
    // find actual entry
    for (oc = Confp->initial_object; oc; oc = oc->next) {
      begin = oc->symbol_table_p;
      end = begin + oc->symbol_num;
      if (begin < is && is < end) {
        _name = GET_NAME(is, oc->str_table_p);
        break;
      }
    }
    if (!strcmp(name, _name)) {
      if (address) *address = oc;
      return address ? is : 0;
    }
    pre = pre_chain;
  }
  // the another way is return a last-put entry instead 0.
  printf("pre_next:%p\n", &pre->next);
  return address ? 0 : &pre->next;
}

void* lookup_dynamic_symbol(char* name, size_t* address, uint32_t* ever) {
  size_t* table_index =
      (DLLHashTable.bucket + (elf_hash(name) % DLLHashTable.nbucket));
  // printf("!%p,%p\n", table_index, *table_index);
  if (*table_index == 0) {
    return address ? 0 : table_index;
  }
  SymbolChain3* pre_chain;
  SymbolChain3* pre;
  char* _name;
  pre_chain = *table_index;
  for (; pre_chain; pre_chain = pre_chain->next) {
    _name = pre_chain->name;
    printf("%s\n", _name);
    if (!strcmp(name, _name)) {
      printf("found,%s,%s\n", _name, pre_chain->this);
      if (address) {
        if (pre_chain->ever == 0) {
          // store PltOffset when a new entry is added.
          pre_chain->ever = PltOffset;
        } else {
          // load PltOffset when a new entry is added.
          *ever = pre_chain->ever;
        }
        return pre_chain->this;
      } else {
        return pre_chain->next;
      }
    }
    pre = pre_chain;
  }
  printf("ccc!,%p\n", pre);
  return address ? 0 : pre;
}

void alloc_symbol_chain(char* name, void* is) {
  SymbolChain* chain = __malloc(sizeof(SymbolChain));
  SymbolChain* pre_chain;
  chain->next = 0;
  chain->p = is;
  size_t* ret = lookup_symbol(name, 0);
  if (ret)
    *ret = chain;
  else {
    fprintf(stderr, "should raise an error as symbol is overrlapping.%s\n",
            name);
  }
  // you need to allocate another symbolchain for object chain.
  chain = __malloc(sizeof(SymbolChain));
  chain->next = 0;
  chain->p = is;
  if (!Confp->current_object->symbol_chain_head)
    Confp->current_object->symbol_chain_head = chain;
  else
    Confp->current_object->symbol_chain_tail->next = chain;
  Confp->current_object->symbol_chain_tail = chain;
  /*
  if (*table_index == 0) {
    *table_index = chain->p;
  } else {
    // hash collision
    pre_chain = *table_index;
    for (;pre_chain->next;pre_chain = pre_chain->next);
    // you should also check the data itself is same.
  }
  */
}

void __z__link__alloc_symbol_chain(char* name, void* is) {
  return alloc_symbol_chain(name, is);
}

void alloc_static_symbol(SectionChain* sc, SymbolChain* is) {
  SymbolChain* chain = __malloc(sizeof(SymbolChain));
  chain->p = is;
  if (sc->sym_head == 0) {
    sc->sym_head = chain;
    sc->sym_tail = chain;
  } else {
    sc->sym_tail->next = chain;
    sc->sym_tail = chain;
  }
}

void* alloc_dynamic_symbol(char* name, size_t* dllname) {
  size_t* ret = lookup_dynamic_symbol(name, 0, 0);
  if (ret) {
    SymbolChain3* chain = __malloc(sizeof(SymbolChain3));
    chain->next = 0;
    chain->this = dllname;
    chain->name = name;
    chain->ever = 0;
    *ret = chain;
  } else {
    printf("should not\n");
    // should not be happened.
  }
}

void* alloc_candidate_symbol(void* obj, void* sec, ListContainer* lc) {
  printf("alloc candidate symbol\n");
  SymbolChain3* chain = __malloc(sizeof(SymbolChain3));
  chain->next = 0;
  chain->this = obj;
  chain->name = sec;
  if (lc->init == 0) {
    lc->init = chain;
  } else {
    ((SymbolChain3*)lc->current)->next = chain;
  }
  lc->current = chain;
  printf("lccurrent:%p\n", lc->current);
  return chain;
}
