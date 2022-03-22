

#include <stdint.h>
#include <stdio.h>

#include "alloc.h"
#include "link.h"
#include "pe.h"

extern Config* Confp;

static int char_to_digit(char* p) {
  uint8_t i = 7;
  char* q = p + 7;
  int digit = 0;
  for (; i; i--, q--) {
    digit += *q - 0x30;
  }
  return digit;
}

void __z__link__coff_section_callback(void* arg1, void* base, size_t* _oc) {
  ObjectChain* oc = *_oc;
  if (oc == 0) {
    *_oc = __z__link__alloc_obj_chain(0, 0, 0);
    oc = *_oc;
    IMAGE_FILE_HEADER* ifh = base;
    char* str = (char*)base + ifh->PointerToSymbolTable;
    oc->symbol_table_p = str;
    str += ifh->NumberOfSymbols * sizeof(IMAGE_SYMBOL);
    oc->section_head = arg1;
    oc->str_table_p = str;
    oc->map_base = base;
  }
  char* str_table_p = oc->str_table_p;
  IMAGE_SECTION_HEADER* isec = arg1;
  char* sh_name = isec->Name;

  /* printf("ok:%s,%c,%p\n", sh_name,*sh_name, _oc); */
  if (*sh_name == '/') {
    int offset = char_to_digit(sh_name);
    /* printf("%s\n", str_table_p + offset); */
    sh_name = str_table_p + offset;
  }
  SectionContainer* sc = match_section(sh_name);
  if (sc == 0) {
    sc = alloc_section_container(0, sh_name, 0, 0);
  }
  void* data_offset = oc->map_base + isec->PointerToRawData;
  void* schain = alloc_section_chain(isec, data_offset, sc, oc);
  /* if (schain) */
  /*   update_object_chain(oc, schain); */
}

static int aux_symbols = 0;

void __z__link__coff_symtab_callback(void* arg1, void* str_p, size_t* _oc) {
  ObjectChain* oc = *_oc;
  if (aux_symbols) {
    aux_symbols--;
    return;
  }
  // char* str_table_p = str_p;
  IMAGE_SYMBOL* is = arg1;
  // char* str_table_p = _oc->str_table_p;
  uint32_t* aa = (uint32_t*)is;
  char* str_t_p = oc->str_table_p;
  char* name = GET_NAME(is, str_t_p);

  if (Confp->loglevel > 0) {
    char* str[100] = {};
    logger_emit("misc.log", "[link/coff/callback.c]\t sym table\n");
    logger_emit("misc.log", "-------------------------\n");
    sprintf(str,
            "\tname:\t%s\n"
            "\tvalue\t:%p\n"
            "\tsection number\t:%p\n"
            "\ttype\t:%p\n"
            "\tstorage class\t:%p\n"
            "\tnum of aux sym\t:%p\n",
            name, is->Value, is->SectionNumber, is->Type, is->StorageClass,
            is->NumberOfAuxSymbols);
    logger_emit("misc.log", str);
  }
  if (is->StorageClass == 2 /*External*/ && is->SectionNumber) {
    int sym_index = 1;
    // add_export_entry(name);
    alloc_export_symbol_chain(is, name, is->SectionNumber);
    // number of export symbols and its string size should be
    // counted for constructing export table on later stage.
    // EAT + ENT + ordinal(WORD)
    if (Confp->export_directory_len == 0)
      Confp->export_directory_len =
          sizeof(IMAGE_EXPORT_DIRECTORY) + strlen(Confp->outfile_name) + 1;
    Confp->export_directory_len += 4 + 4 + 2 + strlen(name) + 1;
    Confp->export_func_count += 1;
    // add_dt_hash_entry(Confp->hash_table_p, name, sym_index);
  }
  if (is->NumberOfAuxSymbols) {
    aux_symbols = is->NumberOfAuxSymbols;
  }
}

void __z__link__coff_set_virtual_address(void* arg1) {
  SectionContainer* sc = arg1;
  SectionChain* schain = sc->init;
  if (sc->init == 0) return;
  int size = 0;
  IMAGE_SECTION_HEADER* shdr;
  sc->size = 0;
  for (; schain; schain = schain->this) {
    shdr = schain->p;
    // printf("sc:%p,%p\n", shdr->SizeOfRawData, schain->obj);
    schain->virtual_address = Confp->virtual_address_offset + sc->size;
    // shdr->SizeOfRawData;
    shdr->VirtualAddress = schain->virtual_address;
    sc->size += shdr->SizeOfRawData;
  }
  sc->virtual_address = Confp->virtual_address_offset;
  Confp->virtual_address_offset += sc->size;
  Confp->plt_offset = Confp->virtual_address_offset;
}

void __z__link__coff_do_reloc(void* arg1) {
  SectionContainer* sc = arg1;
  if (sc->init == 0) return;
  SectionChain* schain = sc->init;
  IMAGE_SECTION_HEADER* ish;
  IMAGE_SYMBOL* is1;
  CoffReloc* reloc;
  char* name;
  int i = 0;
  ObjectChain* oc;
  char* str_table_p;
  CallbackArgIn arg = {};
  uint32_t addr = 0;
  for (; schain; schain = schain->this) {
    ish = schain->p;
    oc = schain->obj;
    str_table_p = oc->str_table_p;
    reloc = oc->map_base + ish->PointerToRelocations;
    // printf("a:%p,%p\n", oc->map_base, ish->PointerToRelocations);
    for (i = 0; i < ish->NumberOfRelocations; i++, reloc++) {
      is1 = (IMAGE_SYMBOL*)oc->symbol_table_p + reloc->SymbolTableIndex;
      // current section + reloc_offset
      logger_emit_p(((IMAGE_SECTION_HEADER*)schain->p)->PointerToRawData);
      arg.filled_addr = oc->map_base +
                        ((IMAGE_SECTION_HEADER*)schain->p)->PointerToRawData +
                        reloc->VirtualAddress;
      logger_emit_p(arg.filled_addr);
      arg.src_vaddr = schain->virtual_address + reloc->VirtualAddress;
      arg.dst_vaddr = is1->Value;
      arg.name = GET_NAME(is1, str_table_p);
      // virtual address points to the head of address not the tail.
      /* if (EmitType != EMIT_OBJ) { */
      /* 	arg.virtual_address = ish->VirtualAddress +
       * reloc->VirtualAddress; */
      /* } else { */
      /* 	arg.section_name = ((IMAGE_SECTION_HEADER*)(sec2->p))->Name; */
      /* } */
      arg.src_section_name = ((IMAGE_SECTION_HEADER*)schain->p)->Name;
      arg.type = reloc->Type;
      arg.storage_class = is1->StorageClass;
      arg.src_oc = oc;
      arg.shndx = is1->SectionNumber;
      if (Confp->loglevel > 0) {
        char log[50] = {};
        sprintf(log, "%s,%p,%p,%p\n", arg.name, reloc->Type,
                reloc->VirtualAddress, is1->SectionNumber);
        logger_emit("misc.log", log);
      }
      __coff_resolve(&arg);
    }
  }
}
