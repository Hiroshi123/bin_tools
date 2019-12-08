
#include <windows.h>
#include <stdio.h>
#include <stdint.h>
#include "win_memory.h"

#include "coff.h"
#include "link.h"

extern ObjectChain* InitialObject;
extern ObjectChain* CurrentObject;
extern struct SymbolHashTable HashTable;
extern struct SymbolHashTable DLLHashTable;

void* lookup_symbol(char* name, size_t* address) {
  size_t* table_index = (HashTable.bucket + (elf_hash(name) % HashTable.nbucket));
  // if you do not find any entry on this table.
  if (*table_index == 0) {
    return address ? 0 : table_index;
  }
  SymbolChain* pre_chain;
  IMAGE_SYMBOL* is;
  ObjectChain* oc;
  char* _name;
  IMAGE_SYMBOL* begin;
  IMAGE_SYMBOL* end;
  // if you find any entry then trace the chain which was stored.
  pre_chain = *table_index;
  printf("lookup,%p.\n", pre_chain);
  // check hash collision

  printf("lookup,%s.\n", name);
  for (;pre_chain;pre_chain = pre_chain->next) {
    is = pre_chain->p;
    printf("lookup,%s.\n", name);
    // find actual entry
    for (oc=InitialObject;oc;oc=oc->next) {
    printf("lookup,%s...\n", name);

      begin = oc->symbol_table_p;
      end = begin + oc->symbol_num;
      if (begin<is && is<end) {
	_name = GET_NAME(is, oc->str_table_p);
	break;
      }
    }
    
    if (!strcmp(name,_name)) {
      printf("matched\n");
      if (address)
	*address = get_export_virtual_address(is, oc);
      return address ? is : 0;
    }
  }
  // the another way is return a last-put entry instead 0.
  return address ? 0 : pre_chain;
}

void* lookup_dynamic_symbol(char* name, size_t* address) {
  size_t* table_index = (DLLHashTable.bucket + (elf_hash(name) % DLLHashTable.nbucket));
  printf("!%p,%p\n", table_index,*table_index);
  if (*table_index == 0) {    
    return address ? 0 : table_index;
  }
  SymbolChain3* pre_chain;
  char* _name;
  pre_chain = *table_index;
  for (;pre_chain;pre_chain = pre_chain->next) {
    _name = pre_chain->name;
    if (!strcmp(name,_name)) {
      printf("found,%s,%s\n", _name, pre_chain->this);
      return address ? pre_chain->this : pre_chain->next;
    }
  }
  return address ? 0 : pre_chain;
}


void alloc_symbol_chain(char* name, void* is) {
  SymbolChain* chain = __malloc(sizeof(SymbolChain));
  SymbolChain* pre_chain;
  chain->next = 0;
  chain->p = is;
  size_t* ret = lookup_symbol(name, 0);
  printf("!!!!alloc sym:%p\n", chain);
  if (ret) *ret = chain;
  else {
    printf("should raise an error as symbol is overrlapping.\n");
  }
  printf("aaaa:%s,%p,%p,%p\n", name, ret,chain,*ret);  
  // you need to allocate another symbolchain for object chain.
  chain = __malloc(sizeof(SymbolChain));
  chain->next = 0;
  chain->p = is;
  if (!CurrentObject->symbol_chain_head)
    CurrentObject->symbol_chain_head = chain;
  else
    CurrentObject->symbol_chain_tail->next = chain;
  CurrentObject->symbol_chain_tail = chain;
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

void* alloc_dynamic_symbol(char* name, size_t* dllname) {
  size_t* ret = lookup_dynamic_symbol(name, 0);
  if (ret) {
    SymbolChain3* chain = __malloc(sizeof(SymbolChain3));
    chain->next = 0;
    chain->this = dllname;
    chain->name = name;
    *ret = chain;
  } else {
    // should not be happened.
  }
}

