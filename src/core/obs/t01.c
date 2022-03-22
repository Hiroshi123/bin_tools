
#include <stdio.h>
#include <stdint.h>
#include "os.h"
#include "alloc.h"
#include "elf.h"
#include "logger.h"

extern void* __get_ntdll_handle_64();
extern void* __get_ntdll_handle_64_end();
extern void* __enumerate_import_table_2();
extern void* __enumerate_import_table_2_end();


// this technique allows ntdll calling to direct system call given runtime ntdll version.

static void m01() {

  char* p01 = &__get_ntdll_handle_64;
  char* p02 = &__get_ntdll_handle_64_end;
  logger_emit_p(p02-p01);  
  // int size = p02 - p01;
  Elf64_Sym* sym = __malloc(sizeof(Elf64_Sym));  
  // sym->st_name;
  // sym->st_info;
  // sym->st_other;
  // sym->st_shndx;
  // sym->st_value;
  // sym->st_size = p02 - p01;
  
  // Elf64_Sym* p = sym->p;
  // add_symbol(); 
}

void __z__obs__add_call(void* rip, int diff) {
  
  // __os__malloc();
  
}

void __z__obs__t01() {
  
  m01();
  
  /* for (;a<e;a++) { */
  /*   __os__write(1, "a\n", 2); */
  /*   // *a = *e; */
  /* } */
  
  // add_symbol();
  // add_symbol();
  
  char* p03 = &__enumerate_import_table_2;
  char* p04 = &__enumerate_import_table_2_end;
  
  logger_emit_p(p04-p03);
  
  /* logger_emit_p(*a); */
  /* logger_emit_p(e); */
  
}

