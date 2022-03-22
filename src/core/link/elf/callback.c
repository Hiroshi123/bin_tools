
#include <stdint.h>
#include <stdio.h>

#include "alloc.h"
#include "elf.h"
#include "link.h"

extern Config* Confp;

// this callback is not only retrieving attributes on each sections,
// but put section offset address
// on this linker's process on each section offset.

// this translation works for 64bit ELF as sh_offset is 8byte.
// but has problem for 32bit ELF.

void __z__link__on_section_callback_for_link(uint8_t* p, uint8_t* strtable,
                                             /*Elf64_Shdr*/ void* shdr,
                                             /*sh_callback_arg*/ size_t* ret) {
  ObjectChain* oc = *ret;

  if (oc == 0) {
    // if callback args does not have pointer to callback
    // ( meaning it is first time for this iteration.),
    // then you need to allocate object chain for it.
    // NOTE :: This behavior assumes first section on input is always null.
    // if the rule is broken, things here does not work out.
    *ret = __z__link__alloc_obj_chain(0, 0, 0);
    oc = *ret;
    oc->section_head = shdr;
    oc->sh_str_table_p = strtable;
    oc->map_base = p;
    printf("%p,%p,%p\n", oc->map_base, p, shdr);
    // for (;;);
    return;
  }
  printf("p:%p\n", p);

  Elf64_Shdr* shdr64 = shdr;
  Elf32_Shdr* shdr32 = shdr;
  char* sh_name = 0;
  // 64bit is 8byte on this field
  // if it requires full of its bytes, need to be correct.
  uint32_t sh_type = 0;
  uint32_t sh_size = 0;

  if (Confp == NULL) {
    printf(
        "[callback.c]\tplease specify format on confp when iterating this "
        "function\n");
    for (;;)
      ;
  }
  if (Confp->file_format == 1 /*ELF32*/) {
    sh_name = (uint8_t*)strtable + shdr32->sh_name;
    sh_type = shdr32->sh_type;
    // cannot do this for 32bit as field is 4byte
    // shdr32->sh_offset += p;
    sh_size = shdr32->sh_size;

  } else if (Confp->file_format == 2 /*ELF64*/) {
    sh_name = (uint8_t*)strtable + shdr64->sh_name;
    sh_type = shdr64->sh_type;
    shdr64->sh_offset += p;
    sh_size = shdr64->sh_size;

  } else {
    printf(
        "[callback.c]\tplease specify format on confp when iterating this "
        "function\n");
    for (;;)
      ;
  }

  switch (sh_type) {
    case SHT_PROGBITS:
      goto insert;
    case SHT_SYMTAB: {
      if (Confp->file_format == 1 /*ELF32*/) {
        oc->symbol_num = sh_size / sizeof(Elf32_Sym);
        printf("symp:%p,%p\n", shdr32->sh_offset, p);
        oc->symbol_table_p = shdr32->sh_offset + oc->map_base;
        oc->str_table_p =
            p + (((Elf32_Shdr*)oc->section_head) + shdr32->sh_link)->sh_offset;
      } else if (Confp->file_format == 2 /*ELF64*/) {
        oc->symbol_num = sh_size / sizeof(Elf64_Sym);
        printf("symp:%p\n", shdr64->sh_offset);
        oc->symbol_table_p = shdr64->sh_offset;
        oc->str_table_p =
            p + (((Elf64_Shdr*)oc->section_head) + shdr64->sh_link)->sh_offset;
      }
      goto insert;
    }
    case SHT_RELA: {
      RelocationChain* relocation_chain =
          __z__link__alloc_reloc_chain(sh_name, shdr);
      if (relocation_chain) {
        RelocationChain* rel = oc->reloc_section_head;
        // if this is not the first entry
        if (rel) {
          rel->next = relocation_chain;
        } else {
          // if this is first entry
          oc->reloc_section_head = relocation_chain;
        }
        oc->reloc_section_tail = relocation_chain;
      }
      goto skip;
    }
    case SHT_NOBITS: {
      // .bss section does not contain any data at this stage,
      // but needs to be taken into account for computation of virtual address.

      // Confp->bss_size += sh_size;
      /* printf("bss:%p\n", Confp->bss_size); */
      /* for (;;); */
      // goto skip;

      if (Confp->file_format == 1 /*ELF32*/) {
        shdr32->sh_offset = __malloc(sh_size);
      } else if (Confp->file_format == 2 /*ELF64*/) {
        shdr64->sh_offset = __malloc(sh_size);
      }
      goto insert;
    }
    case SHT_NOTE: {
      if (Confp->has_gnu_property) goto skip;
      //
      if (!strcmp(sh_name, ".note.gnu.property") &&
          Confp->has_gnu_property == 0) {
        Confp->has_gnu_property = 1;
        Confp->program_header_num++;
        Confp->program_header_tail += sizeof(Elf64_Phdr);
        if (Confp->file_format == 1 /*ELF32*/) {
          Confp->program_header_tail += sizeof(Elf32_Phdr);
        } else if (Confp->file_format == 2 /*ELF64*/) {
          Confp->program_header_tail += sizeof(Elf64_Phdr);
        }
      }
      goto insert;
    }
    case SHT_INIT_ARRAY:
    case SHT_FINI_ARRAY:
      goto skip;
    default:
      goto skip;
  }

insert:

  SectionContainer* sc = __z__link__match_section(sh_name);
  if (sc == 0) {
    sc = __z__link__alloc_section_container(0, sh_name, 0, 0);
  }
  void* schain = __z__link__alloc_section_chain(shdr, 0, sc, 0);

skip:
  // update_object_chain(oc, schain);
}

// what is put on symbol table is as follows.
// st-info bind == GOLBAL/LOCAL/WEAK
// type == FUNCTION/OBJECT/NOTYPE
// visibility == DEFAULT
// STT_SECTION is not included on symbol table but could be the subject of
// relocation without using it.

void __z__link__on_elf_symtab_callback_for_link(void* /*Elf64_Sym*/ sym,
                                                void* _oc) {
  Elf32_Sym* sym32 = sym;
  Elf64_Sym* sym64 = sym;
  unsigned char st_info;
  unsigned char st_bind;
  unsigned char st_other;
  unsigned char st_visibility;
  unsigned char st_type;
  size_t st_name;
  uint16_t st_shndx;
  ObjectChain* oc = _oc;
  uint8_t* p = (ObjectChain*)oc->str_table_p;
  char max_name[100] = {};

  if (Confp == NULL) {
    printf(
        "[callback.c]\tplease specify format on confp when iterating this "
        "function\n");
    for (;;)
      ;
  }

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

  if (Confp->file_format == 1 /*ELF32*/) {
    printf("%p\n", sym32);
    printf("%p\n", sym32->st_info);

    st_info = sym32->st_info;
    st_other = sym32->st_other;
    st_name = sym32->st_name;
    st_shndx = sym32->st_shndx;
    st_bind = ELF32_ST_BIND(sym32->st_info);
    st_type = ELF32_ST_TYPE(sym32->st_info);
    st_visibility = ELF32_ST_VISIBILITY(sym32->st_other);

    // for (;;);

  } else if (Confp->file_format == 2 /*ELF64*/) {
    st_info = sym64->st_info;
    st_other = sym64->st_other;
    st_name = sym64->st_name;
    st_shndx = sym64->st_shndx;
    st_bind = ELF64_ST_BIND(sym64->st_info);
    st_type = ELF64_ST_TYPE(sym64->st_info);
    st_visibility = ELF64_ST_VISIBILITY(sym64->st_other);
    // ELF64_ST_BIND(arg1->st_info);

  } else {
    printf(
        "[callback.c]\tplease specify format on confp when iterating this "
        "function\n");
    for (;;)
      ;
  }

  // only globally binded symbol will be registered on hash table.

  // STB_WEAK is used for c++ resolution.
  /* if ((ELF64_ST_BIND(arg1>st_info) == STB_GLOBAL || */
  /*      ELF64_ST_BIND(arg1->st_info) == STB_WEAK || */
  /*      ELF64_ST_BIND(arg1->st_info) == STB_LOCAL */
  /*      ) && */
  /*     ELF64_ST_VISIBILITY(arg1->st_other) == STV_DEFAULT && */
  /*     arg1->st_shndx != SHN_UNDEF) { */

  if (st_visibility != STV_DEFAULT) {
    goto skip;
  }

  if (st_shndx == SHN_UNDEF) {
    goto skip;
  }

  // printf("aaa,%s,%p\n", p + arg1->st_name, arg1->st_shndx);

  if (st_shndx == SHN_ABS) {
    // if this is file name, skip this
    goto skip;
  }

  // TODO :: SHN_COMMON(communal variables)
  // Merge them if they are duplicated,
  // if not, simply put on .bss section
  if (st_shndx == SHN_COMMON) {
    printf("SHN_COMMON is not yet supported:%s\n", p + st_name);
    return;
  }
  if (st_shndx & 0xff00 == 0xff) {
    printf("%p is not yet supported:%s\n", st_shndx, p + st_name);
    return;
  }

  Elf64_Shdr* shdr = ((Elf64_Shdr*)(oc->section_head)) + st_shndx;
  Elf32_Shdr* shdr32 = shdr;
  Elf32_Shdr* shdr64 = shdr;

  // STT_SECTION is used for static data allocation except .bss which is
  // initialized as 0. section name such as .text or .data does not need to be
  // fed on symbol table as relocation can be done without it. relocation entry
  // => symbol entry => section entry sym->st_shndx is used through the
  // transition which cannot be overrided.
  if (st_type == STT_SECTION) {
    // arg1->st_name must be 0
    // sym_name = oc->sh_str_table_p + shdr->sh_name;
    // printf("aaa!,%p,%s\n", arg1->st_name, sec_name);
    // for (;;);
    goto skip;
  }
  char* sym_name = p + st_name;
  __z__link__alloc_export_symbol_chain(sym, sym_name, st_shndx);
  char* sec_name = 0;
  uint32_t index = 0;
  if (Confp->file_format == 1 /*ELF32*/) {
    sec_name = oc->sh_str_table_p + shdr32->sh_name;
    index = __z__link__get_section_index_by_name(sec_name, shdr);
    // section index is also updated
    sym32->st_shndx = index;
  } else if (Confp->file_format == 2 /*ELF64*/) {
    sec_name = oc->sh_str_table_p + shdr64->sh_name;
    index = __z__link__get_section_index_by_name(sec_name, shdr);
    if (index == 0) {
      printf("a,%s\n", sec_name);
      for (;;)
        ;
    }
    // section index is also updated
    sym64->st_shndx = index;
  }
  sprintf(max_name, "[link/elf/callback.c]\t symbol table callback : %s,%p\n",
          sym_name /*p + arg1->st_name*/, index);
  __z__logger_emit("misc.log", max_name);

skip:
}
