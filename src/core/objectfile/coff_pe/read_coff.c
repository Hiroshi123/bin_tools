
#include <stdio.h>

#include "coff.h"

// this is an intermidiate detection for coff.

#define DEBUG

/*static inline */ const char check_coff(const uint16_t* p) {
  return *p == 0x8664;
}

char read_coff(const char* const begin, info_on_coff* e) {
  IMAGE_FILE_HEADER* img_file_header = (IMAGE_FILE_HEADER*)begin;
  int sec_num = img_file_header->NumberOfSections;

  e->sec_begin = (IMAGE_SECTION_HEADER*)(img_file_header + 1);
  e->sec_end = e->sec_begin + sec_num;
  IMAGE_SECTION_HEADER* s = e->sec_begin;
  char filled = 0;
  for (; s != e->sec_end; s++) {
    if (!strcmp(s->Name, ".text") && !filled) {
      filled = 1;  // first come section, first served
      e->text_begin = begin + s->PointerToRawData;
      e->text_end = e->text_end + s->SizeOfRawData;
      e->text_reloc_begin = begin + s->PointerToRelocations;
      e->text_reloc_end = e->text_reloc_begin +
                          sizeof(relocation_entry) * (s->NumberOfRelocations);
    }
    if (!strcmp(s->Name, ".pdata")) {
      // you should do something...
    }
#ifdef DEBUG
    printf("%s\n", s->Name);
    printf("%x\n", s->NumberOfRelocations);
    printf("%x\n", s->PointerToRelocations);
    printf("%x\n", s->PointerToRawData);
    printf("%x\n", s->SizeOfRawData);
    printf("---\n");
#endif
  }

  IMAGE_SYMBOL* sym_begin =
      (IMAGE_SYMBOL*)(begin + img_file_header->PointerToSymbolTable);
  IMAGE_SYMBOL* sym_end = sym_begin + img_file_header->NumberOfSymbols;
  e->sym_begin = (IMAGE_SYMBOL*)(begin + img_file_header->PointerToSymbolTable);
  e->sym_end = e->sym_begin + img_file_header->NumberOfSymbols;
  IMAGE_SYMBOL* is = e->sym_begin;
  const char* str_begin = e->sym_end;
  char* name;
  for (; is != e->sym_end; is++) {
    name = (*(uint32_t*)is == 0)
               ? (char*)((size_t)str_begin + *((uint32_t*)is + 1))
               : is->N.ShortName;
#ifdef DEBUG
    printf("val:%x\n", is->Value);
    printf("%s\n", name);
    printf("value:%x\n", is->Value);
    printf("sectionNum:%x\n", is->SectionNumber);
    printf("type:%x\n", is->Type);
    printf("storageClass:%x\n", is->StorageClass);
    printf("auxNum:%x\n", is->NumberOfAuxSymbols);
    printf("----\n");
#endif
    // jump aux vector if it has
    for (; is != is + is->NumberOfAuxSymbols; is++)
      ;
  }
}

char* do_reloc_coff(/*const char* query,*/ info_on_coff* e1, info_on_coff* e2) {
  relocation_entry* r = e1->text_reloc_begin;
  for (; r != e1->text_reloc_end; r++) {
    uint32_t* caller_arg_addr = (uint32_t*)(e1->text_begin + r->rel_addr);
    char* caller_next_addr = (char*)(caller_arg_addr + 1);
    IMAGE_SYMBOL* caller_sym = e1->sym_begin + r->sym_index;
    char* fname =
        (*(uint32_t*)caller_sym == 0)
            ? (char*)((size_t)e1->str_begin + *((uint32_t*)caller_sym + 1))
            : caller_sym->N.ShortName;
#ifdef DEBUG
    printf("%s\n", fname);
#endif
    char* n;
    char* callee_addr;
    // iterate on self-symbol table.
    IMAGE_SYMBOL* sym = e1->sym_begin;
    for (; sym != e1->sym_end; sym++) {
      n = (*(uint32_t*)sym == 0)
              ? (char*)((size_t)e1->str_begin + *((uint32_t*)sym + 1))
              : sym->N.ShortName;
      if (!strcmp(fname, n)) {
        callee_addr = e1->text_begin + sym->Value;
        *caller_arg_addr = (uint32_t)(callee_addr - caller_next_addr);
        // resolved = 1;
        break;
      }
      // jump aux vector if it has
      for (; sym != sym + sym->NumberOfAuxSymbols; sym++)
        ;
    }
    // iterate on a symbol table of another coff file.
    sym = e2->sym_begin;
    for (; sym != e2->sym_end; sym++) {
      n = (*(uint32_t*)sym == 0)
              ? (char*)((size_t)e2->str_begin + *((uint32_t*)sym + 1))
              : sym->N.ShortName;
      if (!strcmp(fname, n)) {
        callee_addr = e2->text_begin + sym->Value;
        *caller_arg_addr = (uint32_t)(callee_addr - caller_next_addr);
        // resolved = 1;
        break;
      }
      // jump aux vector if it has
      for (; sym != sym + sym->NumberOfAuxSymbols; sym++)
        ;
    }
  }
}

char* find_test_(char* query, info_on_coff* e) {
  IMAGE_SYMBOL* s = (IMAGE_SYMBOL*)(e->sym_begin);
  char* n;
  for (; s != e->sym_end; s++) {
    n = (*(uint32_t*)s == 0)
            ? (char*)((size_t)e->str_begin + *((uint32_t*)s + 1))
            : s->N.ShortName;
    if (!memcmp(n, query, strlen(query))) {
      return e->text_begin + s->Value;
    }
    for (; s != s + s->NumberOfAuxSymbols; s++)
      ;
  }
  return 0;
}
