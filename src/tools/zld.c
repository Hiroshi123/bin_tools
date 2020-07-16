
#include <stdio.h>
#include <stdint.h>

#include "elf.h"
#include "link.h"
#include "alloc.h"
#include "objformat.h"
#include "pe.h"

static FILE* MiscLog = 0;

extern void __z__link__elf_set_virtual_address(void* arg1);
extern void __z__link__coff_set_virtual_address();
extern void __z__link__coff_do_reloc();
extern void __z__link__elf_do_reloc();

extern void set_program_header(void* arg1);
extern void add_export_symbol(void* _oc, void* arg1);
extern void __p1(void* arg1);

// extern void* alloc_section_container_init(void*, void*, void*, void*);
static Config config;

static void* read_cmdline(int argc, char** argv) {
  uint8_t file_num = argc;
  if (argc == 1) {
    return 0;
  }
  uint8_t i = 1;
  size_t* p = __malloc(sizeof(void*) * (argc));
  p++;
  void* _p = p;
  for (;i<argc;i++) {
    // printf("%s\n", argv[i]);
    if (!strcmp(argv[i], "-v")) {
      config.verbose = 1;
      continue;
    } else if (!strcmp(argv[i], "-o")) {
      config.outfile_name = argv[i+1];
      char* s = config.outfile_name;
      for (;*s != '.';s++);
      if (!strcmp(s, ".so")) {
	config.outfile_type = ET_DYN;
      } else if (!strcmp(s, ".o")) {
	config.outfile_type = ET_EXEC;
      } else if (!strcmp(s, ".dll")) {
	config.outfile_type = PE_DLL;
      } else if (!strcmp(s, ".exe")) {
	config.outfile_type = PE_EXEC;
      } else {
	config.outfile_type = ET_EXEC;
	// printf("%s:not acceptable\n", s);
      }
      i++;
      continue;
    } else if (!strcmp(argv[i], "-p")) {
      config.pack = 1;
      p++;
      continue;
    } else if (!strcmp(argv[i], "-nodynamic")) {
      config.nodynamic = 1;
      continue;
    } else if (!strcmp(argv[i], "-l")) {
      // __malloc()
      config.dynlib = &argv[i+1];
      // config.dynlib = i - 1;// argv[i+1];
      p++;
      continue;
    } else if (!strcmp(argv[i], "-use-dt-hash")) {
      config.use_dt_hash = 1;
      continue;
    } else if (!strcmp(argv[i], "-no-pie")) {
      config.nopie = 1;
      continue;
    } else if (!strcmp(argv[i], "-f")) {
      
      if (!strcmp(argv[i+1], "win32")) {
	config.file_format = COFF_I386;
      } else if (!strcmp(argv[i+1], "win64")) {
	config.file_format = COFF_AMD64;
      } else if (!strcmp(argv[i+1], "elf32")) {
	config.file_format = ELF32;	
      } else if (!strcmp(argv[i+1], "elf64")) {
	config.file_format = ELF64;
      } else {
	printf("unknown format:%s\n", argv[i+1]);
      }
      i++;
      continue;
    }
    *p = argv[i];
    p++;
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
    config.base_address = 0x00000;
      // 0x400000;
  }
  config.program_header_num = 2;
  if (config.nodynamic) {
    config.dynamic_entry_num = 1;
  } else {
    config.dynamic_entry_num = 13;
  }
  config.output_vaddr_alignment = 0;//0x200000;  
  config.virtual_address_offset = config.base_address + 
    config.program_header_num * sizeof(Elf64_Phdr) + sizeof(Elf64_Ehdr);
  config.shdr_num = 0;
  if (!config.entry_address) {
    config.entry_address = config.virtual_address_offset;
  }
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
  for (;*s1;s1++) {
    if (*s1 == *s2) {
      _s1 = s1;
      _s2 = s2;
      for (;*_s1 || *_s2;_s1++,_s2++) {
	if (*_s1 != *_s2) {
	  goto b1;
	}
      }
      return _s1;
    b1:{}
    }
  }
  return 0;
}

static void do_elf_task() {

  // iterate_section_container(set_program_header);
  // sort_section_container_complete();
  if (config.nodynamic == 0)
    add_pltgot_sc(7, ".plt.got");
  iterate_section_container(__z__link__elf_set_virtual_address);
  if (config.nodynamic == 0) {
    add_dynamic();
  } else {
    add_dynamic_sc();
  }
  if (config.dynlib) {
    char** p2 = config.dynlib;    
    for (;*p2;p2++) {
      if (strstr(*p2, ".so")) {
	/* printf("dt needed:%s\n", *p2); */
	add_dt_needed(*p2);
      } else {
	break;
      }
    }
  }
  iterate_object_chain(add_export_symbol, 0);
  iterate_object_chain(&__z__link__elf_do_reloc, 0);
  set_dynanmic();
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
  
  logger_init("misc.log");
  mem_init();
  size_t* p1 = read_cmdline(argc, argv);
  if (p1 == 0) {
    char* str ="usage\n"\
      "-ef:\t specify an entry point function name\n"\    
      "-ib :\t specify image base\n"\
      "-l :\t dynamic library\n"\
      "-p :\t pack binary\n"\
      "-v :\t vervose stdout\n"\
      "-nodynamic :\t nodynamic entry\n"\
      "-use-gnu-hash ;\t use gnu hash(not dt-hash)\n"\
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
  SectionContainer* init = alloc_section_container_init(0, 0, 0, 0);
  config.initial_section = init;
  config.current_section = init;
  ObjectChain* oc = alloc_obj_chain_init(0, 0, 0);
  config.initial_object = oc;
  config.current_object = oc;
  // used for future.
  // config.mem = __thalloc();
  ObjectChain* ocp = 0;
  // this should be done as concurrent as it could be in the end.
  enum OBJECT_FORMAT format = -1;
  for (;*p1;p1++) {
    // printf("u:%s\n", *p1);
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
    case ELF64: {
      ocp = 0;
      run_through_elf_shdr3(scr, &_on_section_callback_for_link, &ocp);
      run_through_elf_symtable3
	(ocp->symbol_table_p, ((Elf64_Sym*)ocp->symbol_table_p) + ocp->symbol_num,
	 &_on_elf_symtab_callback_for_link, ocp);
      break;
    }
    case COFF_I386:
    case COFF_AMD64: {
      ocp = 0;
      __z__obj__run_through_coff_shdr3(scr, &__z__link__coff_section_callback, &ocp);
      __z__obj__run_through_coff_symtable3(scr, &__z__link__coff_symtab_callback, &ocp);
      break;
    }
    default:
      printf("non supported format;%s\n", *p1);
      return;
    }
  }
  /* if (config.pack) */
  /*   iterate_section_container(__p1); */

  if (config.file_format == ELF64)
    do_elf_task();
  if (config.file_format == COFF_AMD64)
    do_coff_task();
  return 0;
}

