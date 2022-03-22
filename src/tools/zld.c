
#include <stdint.h>
#include <stdio.h>

#include "alloc.h"
#include "elf.h"
#include "link.h"
#include "objformat.h"
#include "pe.h"

static FILE* MiscLog = 0;

extern void __z__link__elf_set_virtual_address(void* arg1);
extern void __z__link__coff_set_virtual_address();
extern void __z__link__coff_do_reloc();
extern void __z__link__elf_do_reloc();

extern void set_program_header(void* arg1);
extern void __z__link__add_export_symbol(void* _oc, void* arg1);
extern void __p1(void* arg1);

extern void __z__link__elf_f1(void* arg1);
// extern void* alloc_section_container_init(void*, void*, void*, void*);
static Config config;

#define CMDLINE_SO 1

static void* read_cmdline(int argc, char** argv) {
  uint8_t file_num = argc;
  if (argc == 1) {
    return 0;
  }
  uint8_t i = 1;
  size_t* p = __malloc(sizeof(void*) * (argc));
  // p++;
  void* _p = p;
  int get_obj = 0;
  int get_obj_done = 0;
  int context = 0;
  for (; i < argc; i++) {
    // printf("%s\n", argv[i]);
    if (!strcmp(argv[i], "-v")) {
      config.verbose = 1;
      // continue;
    } else if (!strcmp(argv[i], "-o")) {
      config.outfile_name = argv[i + 1];
      char* s = config.outfile_name;
      for (; *s != '.'; s++)
        ;
      if (!strcmp(s, ".so")) {
        config.outfile_type = ET_DYN;
      } else if (!strcmp(s, ".o")) {
        // most of the exe is actually position independent
        config.outfile_type = ET_DYN;
        // config.outfile_type = ET_EXEC;
      } else if (!strcmp(s, ".dll")) {
        config.outfile_type = PE_DLL;
      } else if (!strcmp(s, ".exe")) {
        config.outfile_type = PE_EXEC;
      } else {
        config.outfile_type = ET_DYN;
        // printf("%s:not acceptable\n", s);
      }
      i++;
      // continue;
    } else if (!strcmp(argv[i], "-p")) {
      config.pack = 1;
      // p++;
      // continue;
    } else if (!strcmp(argv[i], "-nodynamic")) {
      config.nodynamic = 1;
      // continue;
    } else if (!strcmp(argv[i], "-l")) {
      // __malloc()
      config.dynlib = &argv[i + 1];
      context = CMDLINE_SO;
      // ho matter how many dynamic header is set
      // config.dynlib = i - 1;// argv[i+1];
      i++;
      // continue;
    } else if (!strcmp(argv[i], "-use-dt-hash")) {
      config.use_dt_hash = 1;
      // continue;
    } else if (!strcmp(argv[i], "-no-pie")) {
      config.nopie = 1;
      // continue;
    } else if (!strcmp(argv[i], "-f")) {
      if (!strcmp(argv[i + 1], "win32")) {
        config.file_format = COFF_I386;
      } else if (!strcmp(argv[i + 1], "win64")) {
        config.file_format = COFF_AMD64;
      } else if (!strcmp(argv[i + 1], "elf32")) {
        config.file_format = ELF32;
      } else if (!strcmp(argv[i + 1], "elf64")) {
        config.file_format = ELF64;
      } else {
        printf("unknown format:%s\n", argv[i + 1]);
      }
      i++;
      // continue;
    } else if (!strcmp(argv[i], "-ib")) {
      // config.base_address = &argv[i+1];
      printf("-ib is not yet implemented\n");
      i++;
      // continue;
    } else if (!strcmp(argv[i], "-nointerp")) {
      config.no_interp = 1;
      // continue;
    } else if (!strcmp(argv[i], "-interp-name")) {
      config.interp_name = argv[i + 1];
      i++;
      // continue;
    } else if (!strcmp(argv[i], "-ef")) {
      config.entry_function_name = argv[i + 1];
      i++;
      // continue;
    } else if (!strcmp(argv[i], "-keep-gnu-property")) {
      config.keep_gnu_property = 1;
      i++;
      // continue;
    } else if (get_obj_done == 0) {
      get_obj = 1;
      *p = argv[i];
      p++;
      continue;
    } else {
      // printf("unkown param,%s\n", argv[i]);
      // for (;;);
    }
    if (get_obj == 1) {
      get_obj_done = 1;
    }
  }
  *p = 0;
  if (config.file_format == 0) {
    config.file_format = ELF64;
  }
  return _p;
}

static void set_coff_default_config() {
  if (!config.outfile_type) {
    config.outfile_type = PE_EXEC;
  }
  if (!config.outfile_name) {
    config.outfile_name = "a.exe";
  }
  if (config.outfile_type == PE_DLL) {
    config.base_address = 0x00000;
  } else {
    // it seems Windows does not allow me to
    // set 0x0 as base address unlike elf even with dynamic base_address.
    // when starting exe.
    config.base_address = 0x40000;
  }
  config.hash_table_param.nbucket = 1;
  config.hash_table_param.bloom_size = 2;
  config.hash_table_param.bloom_shift = 5;
  if (config.use_dt_hash == 0) {
    config.use_gnu_hash = 1;
  }
  config.virtual_address_offset = 0x1000;
  config.shdr_num = 1;

  if (!config.entry_address) {
    config.entry_address = config.virtual_address_offset;
  }
  config.use_dt_hash = 1;
}

static void set_elf_default_config() {
  if (!config.outfile_type) {
    config.outfile_type = ET_EXEC;
  }
  if (!config.outfile_name) {
    config.outfile_name = "a.out";
  }
  if (config.outfile_type == ET_DYN) {
    config.base_address = 0x00000;
  } else {
    if (config.base_address == 0) config.base_address = 0x000000;
    // 0x400000;
  }

  // PHDR,LOAD,DYNAMIC
  config.program_header_num = 3;
  config.program_header_tail =
      sizeof(Elf64_Ehdr) + config.program_header_num * sizeof(Elf64_Phdr);

  if (config.no_interp) {
  } else {
    config.program_header_num++;
    // 0x1c is temporary ld name length
    // for interp program header
    config.program_header_tail += sizeof(Elf64_Phdr);
    if (config.interp_name == NULL) {
      // /lib64/ld-linux-x86-64.so.2
      static char default_ld_name[] = "/lib64/ld-linux-x86-64.so.3";
      config.interp_name = &default_ld_name[0];
    }
    config.program_header_tail += strlen(config.interp_name) + 1;
  }
  if (config.nodynamic) {
    config.dynamic_entry_num = 1;
  } else {
    config.dynamic_entry_num = 13;
  }
  config.output_vaddr_alignment = 0;  // 0x200000;

  config.shdr_num = 0;

  config.hash_table_param.nbucket = 1;
  config.hash_table_param.bloom_size = 2;
  config.hash_table_param.bloom_shift = 5;
  if (config.use_dt_hash == 0) {
    config.use_gnu_hash = 1;
  }
}

static void* strstr(char* s1, char* s2) {
  char* _s1;
  char* _s2;
  for (; *s1; s1++) {
    if (*s1 == *s2) {
      _s1 = s1;
      _s2 = s2;
      for (; *_s1 || *_s2; _s1++, _s2++) {
        if (*_s1 != *_s2) {
          goto b1;
        }
      }
      return _s1;
    b1 : {}
    }
  }
  return 0;
}

static void do_elf_task() {
  config.virtual_address_offset =
      config.base_address + config.program_header_tail;

  if (!config.entry_address) {
    config.entry_address = config.virtual_address_offset;
  }

  // iterate_section_container(set_program_header);
  // sort_section_container_complete();

  __z__link__add_pltgot_sc(7, ".plt.got");

  __z__link__iterate_section_container(__z__link__elf_set_virtual_address);

  // __z__link__iterate_section_container(__z__link__elf_f1);
  // for (;;);

  __z__link__add_dynamic();

  __z__link__add_dt_needed();

  // [dynamic.c]
  // add symbols to hash entry so every symbols on all given object files are
  // subject of relocation.
  __z__link__iterate_object_chain(__z__link__add_export_symbol, 0);
  // for (;;);
  // for (;;);

  // [reloc.c]
  // .rela section entry is going to be processed here.
  __z__link__iterate_object_chain(&__z__link__elf_do_reloc, 0);
  // [dynamic.c]
  // previous relocation may add another entry such as DE_NEEDED,
  // and the data which are referred from.

  __z__link__set_dynanmic();
  // generate an elf file
  __z__link__gen_elf(config.outfile_name);

  if (config.verbose) {
    printf("emit:%s\n", config.outfile_name);
  }
}

static void do_coff_task() {
  add_coff_section(".edata", 0, config.export_directory_len, 0);
  iterate_section_container(&__z__link__coff_set_virtual_address);
  add_export();
  iterate_section_container(&__z__link__coff_do_reloc);
  add_import();
  __z__link__gen_pe(config.outfile_name);
  if (config.verbose) {
    printf("emit:%s\n", config.outfile_name);
  }
}

int main(int argc, char** argv) {
  __z__logger_init("misc.log");
  __z__mem_init();
  size_t* p1 = read_cmdline(argc, argv);

  if (p1 == 0) {
    char* str =
        "usage\n"
        "-ef:\t specify an entry point function name\n"
        "-ib :\t specify image base\n"
        "-l :\t dynamic library\n"
        "-p :\t pack binary\n"
        "-v :\t vervose stdout\n"
        "-nodynamic :\t nodynamic entry\n"
        "-use-gnu-hash :\t use gnu hash(not dt-hash)\n"
        "-nointerp :\t no interp program header\n"
        "-interp-name :\t dynamic linker location\n"
        "-keep-gnu-property :\t krrp-gnu ptoprty ptogram header given its "
        "existance"
        "-o :\t specify an outputfile. candidate suffix .exe/.dll/.o/.so\n";

    printf("%s\n", str);
    return 0;
  }

  static_data_init(&config);
  if (config.file_format == COFF_I386 || config.file_format == COFF_AMD64) {
    set_coff_default_config();
    init_hashtable("import_list.sqlite3");
  } else {
    // treat as elf
    set_elf_default_config();
    init_hashtable("import_list.sqlite3");
  }
  SectionContainer* init = __z__link__alloc_section_container_init(0, 0, 0, 0);
  config.initial_section = init;
  config.current_section = init;
  ObjectChain* oc = __z__link__alloc_obj_chain_init(0, 0, 0);
  config.initial_object = oc;
  config.current_object = oc;
  // used for future.
  // config.mem = __thalloc();
  ObjectChain* ocp = 0;
  // this should be done as concurrent as it could be in the end.
  enum OBJECT_FORMAT format = -1;

  printf("%p,%p\n", p1, *p1);
  /* for (;;); */

  for (; *p1; p1++) {
    printf("u:%p, %s\n", p1, *p1);

    format = __z__obj__detect_format_fname((void*)*p1, 0);
    if (format == -1) {
      printf("%s does not exist\n", *p1);
      return;
    }
    if (config.file_format == -1) {
      config.file_format = format;
    } else {
      if (config.file_format != format) {
        printf("input format needs to be same:%s\n", *p1);
      }
    }
    void* scr = __z__mem__alloc_file((void*)*p1);
    if (scr == 0) {
      printf("%s does not exist\n", *p1);
      return;
    }
    switch (format) {
      case ELF32:
        printf("%p,%p,%p\n", scr, sizeof(Elf32_Sym), sizeof(Elf64_Sym));
        // for (;;);
      case ELF64: {
        ocp = 0;
        // go through section table and symbol table so that it can construct
        // object chain which refers symbol chain and section chain.
        int tmp = __z__obj__run_through_elf_shdr3(
            scr, &__z__link__on_section_callback_for_link, &ocp);

        if (format == ELF32) {
          printf("sym:%p,%p\n", ocp->symbol_table_p, ocp->symbol_num);
          __z__obj__run_through_elf_symtable4_32(
              ocp->symbol_table_p,
              ((Elf32_Sym*)ocp->symbol_table_p) + ocp->symbol_num,
              &__z__link__on_elf_symtab_callback_for_link, ocp);
        } else if (format == ELF64) {
          __z__obj__run_through_elf_symtable4_64(
              ocp->symbol_table_p,
              ((Elf64_Sym*)ocp->symbol_table_p) + ocp->symbol_num,
              &__z__link__on_elf_symtab_callback_for_link, ocp);
        }

        break;
      }
      case COFF_I386:
      case COFF_AMD64: {
        ocp = 0;
        __z__obj__run_through_coff_shdr3(scr, &__z__link__coff_section_callback,
                                         &ocp);
        __z__obj__run_through_coff_symtable3(
            scr, &__z__link__coff_symtab_callback, &ocp);
        break;
      }
      default:
        printf("non supported format;%s\n", *p1);
        return;
    }
  }

  /* if (config.pack) */
  /*   iterate_section_container(__p1); */

  if (config.file_format == ELF64) do_elf_task();
  if (config.file_format == COFF_AMD64) do_coff_task();
  return 0;
}
