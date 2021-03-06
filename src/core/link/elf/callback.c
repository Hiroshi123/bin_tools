
#include <stdio.h>
#include <stdint.h>

#include "alloc.h"
#include "elf.h"
#include "link.h"

extern Config* Confp;

void _on_elf_symtab_callback_for_link(Elf64_Sym* arg1, void* _oc) {

  ObjectChain* oc = _oc;
  uint8_t* p = (ObjectChain*)oc->str_table_p;
  char max_name[100] = {};
  sprintf(max_name, "[link/elf/callback.c]\t symbol table callback : %s\n", p + arg1->st_name);
  logger_emit("misc.log", max_name);
  /* printf("sym tab callback!,%s,%d,%d,%d,%p,%p,%p,%p\n", */
  /* 	 p + arg1->st_name, */
  /* 	 arg1->st_size, */
  /* 	 arg1->st_shndx, */
  /* 	 arg1->st_value, */
  /* 	 arg1->st_info, */
  /* 	 ELF64_ST_BIND(arg1->st_info), */
  /* 	 arg1->st_other, */
  /* 	 ELF64_ST_VISIBILITY(arg1->st_other) */
  /* 	 );   */
  // only globally binded symbol will be registered on hash table.
  if (ELF64_ST_BIND(arg1->st_info) == STB_GLOBAL &&
      ELF64_ST_VISIBILITY(arg1->st_other) == STV_DEFAULT &&
      arg1->st_shndx != SHN_UNDEF) {

    // TODO :: SHN_COMMON(communal variables)
    // Merge them if they are duplicated,
    // if not, simply put on .bss section
    if (arg1->st_shndx == SHN_COMMON) {
      printf("SHN_COMMON is not yet supported:%s\n", p + arg1->st_name);
      return;
    }
    if (arg1->st_shndx & 0xff00 == 0xff) {
      printf("%p is not yet supported:%s\n", arg1->st_shndx, p + arg1->st_name);
      return;
    }
    alloc_export_symbol_chain(arg1, p + arg1->st_name, arg1->st_shndx);
    Elf64_Shdr* shdr = ((Elf64_Shdr*)(oc->section_head)) + arg1->st_shndx;
    char* sec_name = oc->sh_str_table_p + shdr->sh_name;
    uint16_t index = get_section_index_by_name(sec_name, shdr);
    arg1->st_shndx = index;
  }
}

void _on_section_callback_for_link
(
 uint8_t* p, uint8_t* strtable, Elf64_Shdr* shdr, /*sh_callback_arg*/ size_t* ret) {
  char* sh_name = (uint8_t*)strtable + shdr->sh_name;
  ObjectChain* oc = *ret;
  if (oc == 0) {
    // if callback args does not have pointer to callback
    // ( meaning it is first time for this iteration.),
    // then you need to allocate object chain for it.
    // NOTE :: This behavior assumes first section on input is always null.
    // if the rule is broken, things here does not work out.
    *ret = _alloc_obj_chain(0, 0, 0);
    oc = *ret;
    oc->section_head = shdr;
    oc->sh_str_table_p = strtable;
    return;
  }
  shdr->sh_offset += p;

  // TODO :: all of section prefixed as .rela should be resolved correctly.
  // how about .rela.eh_frame?
  if (shdr->sh_type == SHT_RELA) {
    if (!strcmp(sh_name, ".rela.text") || !strcmp(sh_name, ".rela.data")) {
      // if (!strcmp(sh_name, ".rela.text")) {
      oc->reloc_section_head = shdr;
      oc->reloc_section_tail = shdr;
      // sh_link should point to section index of symbol table.
      // Since symbol table is recorded in another way, and it is not often
      // that objectfile contains multiple symbol tables, it is omitted.
      // }
    }
    return;
  }
  if (shdr->sh_type == SHT_SYMTAB) {
    // ret->symbol_table_p
    // oc = ret->object_chain;
    oc->symbol_num = shdr->sh_size / sizeof(Elf64_Sym);
    oc->symbol_table_p = shdr->sh_offset;
    oc->str_table_p = p + (((Elf64_Shdr*)oc->section_head) + shdr->sh_link)->sh_offset;
    
    // string table is also fed here.
    /* ret->str_offset = p + (((Elf64_Shdr*)ret->section_head) + shdr->sh_link)->sh_offset; */
    /* ret->symbol_offset = shdr->sh_offset; */
    /* ret->symbol_size = shdr->sh_size; */

    // return;
  }
  if (shdr->sh_type == SHT_STRTAB) {
    // if this is section header string table, do not add it, as this will be resolved
    /* if (shdr->sh_offset == strtable) { */
    /*   return; */
    /* } */
    // ret->str_offset = shdr->sh_offset;
    // return;
  }
  if (shdr->sh_type == SHT_STRTAB) {
    // return;
  }
  // if you find previous section that the name is matched with other section,
  // you do not need to reallocate new SectionContainer.

  if (Confp->outfile_type == ET_DYN || Confp->outfile_type == ET_EXEC) {
    // TODO :: .bss section will be truned into a part of .data section which is initialized as 0.
    // if you set a distinct one, rewrite this.
    if (shdr->sh_type == SHT_NOBITS) {
      Confp->bss_size += shdr->sh_size;
      shdr->sh_offset = __malloc(shdr->sh_size);
    } else if (shdr->sh_type != SHT_PROGBITS) {
      return;
    }
  }
  SectionContainer* sc = match_section(sh_name);
  if (sc == 0) {
    sc = alloc_section_container(0, sh_name, 0, 0);
  }
  void* schain = alloc_section_chain(shdr, 0, sc, 0);
  // update_object_chain(oc, schain);
}

