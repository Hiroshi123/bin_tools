
#include <stdint.h>
#include <stdio.h>
#include <windows.h>

#include "coff.h"
#include "link.h"
#include "win_memory.h"

/* extern int run_through_coff_shdr3(void*,void*); */
/* static uint32_t EXPORT_SYMBOL_NUM = 0; */
/* static uint32_t IMPORT_SYMBOL_NUM = 0; */

extern void resolve(CallbackArgIn* _in, uint32_t* addr);
extern void resolve_only_in_a_section(CallbackArgIn* _in, uint32_t* addr);

extern uint32_t ExportDirectoryLen;
extern uint32_t ExportFuncCount;
extern uint32_t ImportDirectoryLen;
extern struct SymbolHashTable HashTable;
extern ObjectChain* CurrentObject;
extern uint64_t ImageBase;
extern uint8_t EmitType;

uint32_t PltBegin = 0;
uint32_t PltOffset = 0;
SectionChain* PltSection = 0;

void* OutputFileName = 0;
void* EntryPointFuncName = 0;
SectionChain* EntrySectionChain = 0;
uint32_t EntrySectionOffset = 0;
uint8_t _Win32 = 0;
uint8_t WithTlsDirectory = 0;

__attribute__((constructor)) void set_heap_header() {}

__attribute__((destructor)) void dealloc() { printf("finish \n"); }

/*static inline */ const char check_coff(const uint16_t* p) {
  if (*p == 0x8664) {
    return 2;
  }
  if (*p == 0x14c) {
    _Win32 = 1;
    return 1;
  }
  return 0;
}

char read_coff(const char* const begin) {
  IMAGE_FILE_HEADER* img_file_header = (IMAGE_FILE_HEADER*)begin;
  int sec_num = img_file_header->NumberOfSections;
  IMAGE_SYMBOL* sym_begin =
      (IMAGE_SYMBOL*)(begin + img_file_header->PointerToSymbolTable);
  IMAGE_SYMBOL* sym_end = sym_begin + img_file_header->NumberOfSymbols;
  IMAGE_SYMBOL* is = sym_begin;
  const char* str_begin = sym_end;
  alloc_obj_chain(sym_begin, str_begin, img_file_header->NumberOfSymbols);
  IMAGE_SECTION_HEADER* sec_begin =
      (IMAGE_SECTION_HEADER*)(img_file_header + 1);
  IMAGE_SECTION_HEADER* sec_end = sec_begin + sec_num;
  IMAGE_SECTION_HEADER* s = sec_begin;
  IMAGE_SECTION_HEADER* _sec;
  char log[30] = {};
  for (; s != sec_end; s++) {
    if (!strcmp(s->Name, "/4")) {
      continue;
    }
    // if you find an existing section which has the same section name as
    // the coming section name, then you need to merge them.
    // To do this, you need to check the list of existing section,
    _sec = check_section(s);
    alloc_section_chain(begin, s, _sec);
    if (s->NumberOfRelocations == 0 /* && s->PointerToRelocations == 0*/) {
    } else
      s->PointerToRelocations += begin;
#ifdef DEBUG
    logger_emit("-------------------\n");
    logger_emit("section\n");
    sprintf(log, "section name:%s\n", s->Name);
    logger_emit(log);
    /* printf("section\n"); */
    /* printf("%s\n",s->Name); */
    /* printf("%x\n", s->NumberOfRelocations); */
    /* printf("reloc:%x\n", s->PointerToRelocations); */
    /* printf("relocN:%x\n", s->NumberOfRelocations); */
    /* printf("%x\n", s->PointerToRawData); */
    /* printf("%x\n", s->SizeOfRawData); */
    /* printf("---\n"); */
#endif
  }
  char* name;
  for (; is != sym_end; is++) {
    name = GET_NAME(is, str_begin);
    // StorageClass == 2 is import/export
    // difference between import and export is if it belongs to section(0) or
    // not. import section should belong to section(0) which means nothing.
    // export section belongs a section from 1.
    if (EntryPointFuncName && !strcmp(EntryPointFuncName, name)) {
      EntrySectionChain = get_sc_from_obj(is->SectionNumber);
      EntrySectionOffset = is->Value;
    }
    if (is->StorageClass == 2 /*External*/ && is->SectionNumber) {
      alloc_symbol_chain(name, is);
      // number of export symbols and its string size should be
      // counted for constructing export table on later stage.
      // EAT + ENT + ordinal(WORD)
      if (ExportDirectoryLen == 0)
        ExportDirectoryLen =
            sizeof(IMAGE_EXPORT_DIRECTORY) + strlen(OutputFileName) + 1;
      ExportDirectoryLen += 4 + 4 + 2 + strlen(name) + 1;
      ExportFuncCount += 1;
    }
    // even if the storage class is static(which means refers from only same
    // object file, can be refered by them.REALLY???)
    if (is->StorageClass == 3 /*Static*/) {
      // SectionChain* sc = get_sc_from_obj(is->SectionNumber);
      // alloc_static_symbol(sc, is);
    }
#ifdef DEBUG
    logger_emit("-------------------\n");
    logger_emit("symbol\n");
    sprintf(log, "symbol name:%s\n", name);
    logger_emit(log);
    /* printf("val:%x\n",is->Value); */
    /* printf("%s\n",name); */
    /* printf("hash:%p\n", elf_hash(name)); */
    /* printf("hash index:%p\n", elf_hash(name) % HashTable.nbucket);     */
    /* printf("value:%x\n", is->Value); */
    /* printf("sectionNum:%x\n", is->SectionNumber); */
    /* printf("type:%x\n", is->Type); */
    /* printf("storageClass:%x\n", is->StorageClass); */
    /* printf("auxNum:%x\n", is->NumberOfAuxSymbols); */
    /* printf("----\n"); */
#endif
    // jump aux vector if it has
    for (; is != is + is->NumberOfAuxSymbols; is++)
      ;
    // HashTable.bucket = 1;
  }
}

void* read_cmdline(int argc, char** argv) {
  uint8_t file_num = argc;
  if (argc == 1) {
    return 0;
  }
  uint8_t i = 1;
  size_t* p = __malloc(sizeof(void*) * (argc));
  void* _p = p;
  for (; i < argc; i++) {
    if (!strcmp(argv[i], "-o")) {
      OutputFileName = argv[i + 1];
      char* s = OutputFileName;
      for (; *s != '.'; s++)
        ;
      if (!strcmp(s, ".dll")) {
        EmitType = EMIT_DLL;
        // printf("DLL\n");
      } else if (!strcmp(s, ".o")) {
        EmitType = EMIT_OBJ;
      } else if (!strcmp(s, ".exe")) {
        EmitType = EMIT_EXE;
      }
      i++;
      continue;
    }
    // ef is the abbrebiation of a function that the entry point points to.
    if (!strcmp(argv[i], "-ef")) {
      EntryPointFuncName = argv[i + 1];
      i++;
      continue;
    }
    if (!strcmp(argv[i], "-ib")) {
      ImageBase = argv[i + 1];
      continue;
    }
    if (!strcmp(argv[i], "-tls")) {
      WithTlsDirectory = 1;
      continue;
    }
    *p = argv[i];
    p++;
  }
  if (ImageBase == 0) {
    if (EmitType == EMIT_DLL) {
      ImageBase = 0x67B80000;
    } else {
      ImageBase = 0x400000;
    }
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
  // you can resolve some pairs of symbols without waiting arrival of all object
  // files.

  logger_init();
  mem_init();
  init_hashtable("import_list.sqlite3");
  size_t* p1 = read_cmdline(argc, argv);
  if (p1 == 0) {
    printf("usage\n");
    printf("-ef : specify an entry point function name\n");
    printf("-ib : specify image base\n");
    printf("-o : specify an outputfile. candidate suffix .exe/.dll/.o\n");
    return 0;
  }
  void* q;
  for (; *p1; p1++) {
    q = alloc_obj((void*)*p1);
    if (check_coff(q) == 0) {
      fprintf(stderr, "a file does not have coff header\n");
      return 0;
    }
    read_coff(q);
  }
  if (EmitType != EMIT_OBJ) {
    SectionChain* tlsdata;
    if (WithTlsDirectory) {
      tlsdata = add_section(".tls", 200);
      add_tls(tlsdata);
    }
    void* esection;
    if (ExportDirectoryLen) esection = dd_section(".edata", ExportDirectoryLen);
    // size should be adjusted by number of import symbol which needs to be
    // resolved.
    void* isection;
    SectionChain* pltsection;
    /* if (ImportDirectoryLen) { */
    pltsection = add_section(".plt", 200);
    isection = add_section(".idata", 200);
    /* } */
    set_virtual_address();
    /* if (ImportDirectoryLen) { */
    PltSection = pltsection;
    PltBegin = pltsection->p->VirtualAddress;
    PltOffset = PltBegin;
    /* } */
    do_reloc(&resolve);
    if (ImportDirectoryLen) add_import(isection);
    if (ExportDirectoryLen) add_export(esection);
  } else {
    do_reloc(&resolve_only_in_a_section);
  }
  gen();
  return 0;
}
