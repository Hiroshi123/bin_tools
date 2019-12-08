
#include <windows.h>
#include <stdio.h>
#include <stdint.h>
#include "win_memory.h"

#include "coff.h"
#include "link.h"

#define DEBUG 1

/* extern int run_through_coff_shdr3(void*,void*); */
/* static uint32_t EXPORT_SYMBOL_NUM = 0; */
/* static uint32_t IMPORT_SYMBOL_NUM = 0; */

extern void resolve(CallbackArgIn* _in,uint32_t* addr);
extern uint32_t ExportDirectoryLen;
extern uint32_t ExportFuncCount;
extern struct SymbolHashTable HashTable;
void* OutputFileName;

__attribute__((constructor)) void set_heap_header() {  
}

__attribute__((destructor)) void dealloc() {
  printf("dealloc\n");
}

/*static inline */const char check_coff(const uint16_t* p) {
  return *p == 0x8664;
}

char read_coff(const char *const begin,
               info_on_coff *e) {

  IMAGE_FILE_HEADER* img_file_header = (IMAGE_FILE_HEADER*)begin;
  int sec_num = img_file_header->NumberOfSections;  
  IMAGE_SYMBOL* sym_begin = (IMAGE_SYMBOL*)(begin + img_file_header->PointerToSymbolTable);
  IMAGE_SYMBOL* sym_end = sym_begin + img_file_header->NumberOfSymbols;
  e->sym_begin = (IMAGE_SYMBOL*)(begin + img_file_header->PointerToSymbolTable);
  e->sym_end = e->sym_begin + img_file_header->NumberOfSymbols;
  IMAGE_SYMBOL* is = e->sym_begin;
  const char* str_begin = e->sym_end;  
  alloc_obj_chain(sym_begin, str_begin, img_file_header->NumberOfSymbols);
  e->sec_begin = (IMAGE_SECTION_HEADER*)(img_file_header+1);
  e->sec_end = e->sec_begin + sec_num;  
  IMAGE_SECTION_HEADER* s = e->sec_begin;
  char filled = 0;
  IMAGE_SECTION_HEADER* _sec;
  for (;s!=e->sec_end;s++) {
    // if you find an existing section which has the same section name as
    // the coming section name, then you need to merge them.
    // To do this, you need to check the list of existing section,
    _sec = check_section(s);
    alloc_section_chain(begin, s, _sec);
    if (s->NumberOfRelocations == 0/* && s->PointerToRelocations == 0*/) {}
    else s->PointerToRelocations += begin;
#ifdef DEBUG
    printf("%s\n",s->Name);
    printf("%x\n", s->NumberOfRelocations);
    printf("reloc:%x\n", s->PointerToRelocations);
    printf("relocN:%x\n", s->NumberOfRelocations);
    printf("%x\n", s->PointerToRawData);
    printf("%x\n", s->SizeOfRawData);
    printf("---\n");
#endif
  }
  char* name;
  for (;is!=e->sym_end;is++) {
    name = GET_NAME(is, str_begin);    
    // StorageClass == 2 is import/export
    // difference between import and export is if it belongs to section(0) or not.
    // import section should belong to section(0) which means nothing.
    // export section belongs a section from 1.
    if (is->StorageClass == 2 && is->SectionNumber) {
      printf("alloc symbol\n");
      alloc_symbol_chain(name, is);
      // number of export symbols and its string size should be
      // counted for constructing export table on later stage.
      // EAT + ENT + ordinal(WORD)      
      ExportDirectoryLen += 4 + 2 + strlen(name) + 1;
      ExportFuncCount += 1;
    }
#ifdef DEBUG
    printf("val:%x\n",is->Value);
    printf("%s\n",name);
    printf("hash:%p\n", elf_hash(name));
    printf("hash index:%p\n", elf_hash(name) % HashTable.nbucket);    
    printf("value:%x\n", is->Value);
    printf("sectionNum:%x\n", is->SectionNumber);
    printf("type:%x\n", is->Type);
    printf("storageClass:%x\n", is->StorageClass);
    printf("auxNum:%x\n", is->NumberOfAuxSymbols);
    printf("----\n");
#endif
    // jump aux vector if it has
    for (;is!=is+is->NumberOfAuxSymbols;is++);
    // HashTable.bucket = 1;
  }
}

void* read_cmdline(int argc, char** argv) {
  uint8_t file_num = argc;
  uint8_t i = 1;
  size_t* p = __malloc(sizeof(void*) * (argc));
  void* _p = p;
  for (;i<argc;i++) {
    if (!strcmp("-o",argv[i])) {
      OutputFileName = argv[i+1];
      i++;
      continue;
    }
    *p = argv[i];
    p++;
  }
  *p = 0;
  return _p;
}

int main(int argc, char** argv) {
  
  // 1.create_section_chain
  // 2.create_symbol_chain
  // 3.relocation finding(all of import should be resolved except -l)
  // 4.add_image_directory_entry_section(0) export..import..
  // 5.set_address_of_export
  // 6.resolve_import
  // 7.resolve_image_directory_entry_section
  // 8.emit target file
  // 
  // --- incremental linking
  // you can resolve some pairs of symbols without waiting arrival of all object files.
  
  /* void* a = __malloc(3); */
  /* void* b = __malloc(5); */
  /* void* _c = __malloc(1); */
  
  /* __free(b); */
  /* __free(a); */
  /* /\* __free(_c); *\/ */
  
  /* _c = __malloc(0x1000); */
  /* printf("free:%p\n",(size_t)_c % 0x1000 == 0); */
  /* printf("free:%p\n",((size_t)_c & 0xFFF) == 0); */
  
  /* __free(_c); */

  mem_init();  
  init_hashtable();
  size_t* p1 = read_cmdline(argc, argv);
  void* q;
  info_on_coff c;
  for (;*p1;p1++) {
    printf("%s\n",*p1);
    q = alloc_obj((void*)*p1);
    read_coff(q, &c);
  }
  void* edata;
  void* isection = add_section(".idata");
  if (ExportDirectoryLen)
    edata = add_section(".edata");
  set_virtual_address();
  do_reloc(&resolve);  
  add_import(isection);
  if (ExportDirectoryLen)  
    add_export(edata);
  gen();
  return 0;
}

