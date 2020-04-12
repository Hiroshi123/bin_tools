
#include <stdio.h>
#include <stdint.h>

#include "elf.h"
#include "link.h"
#include "alloc.h"

extern void* alloc_file(char*);
extern void  gen_elf();
extern void set_virtual_address(void* arg1);
extern void set_program_header(void* arg1);
extern void do_reloc(void* _oc, void* arg1);

// extern void* alloc_section_container_init(void*, void*, void*, void*);
static Config config;

void* read_cmdline(int argc, char** argv) {
  uint8_t file_num = argc;
  if (argc == 1) {
    return 0;
  }
  uint8_t i = 1;
  size_t* p = __malloc(sizeof(void*) * (argc));
  void* _p = p;
  for (;i<argc;i++) {
    if (!strcmp(argv[i], "-o")) {
      config.outfile_name = argv[i+1];
    }
    printf("%s\n", argv[i]);
  }
}

int main(int argc, char** argv) {
  if (argc == 1) {
    printf("need at least 1 argc\n");
    return 0;
  }
  logger_init();
  mem_init();
  size_t* p1 = read_cmdline(argc, argv);
  static_data_init(&config);
  init_hashtable("import_list.sqlite3");
  config.base_address = 0x400000;
  config.dynamic_entry_num = 6;
  config.output_vaddr_alignment = 0;//0x200000;
  config.entry_address = 0x4000b0;
  config.program_header_num = 2;
  config.virtual_address_offset = config.base_address + 
    config.program_header_num * sizeof(Elf64_Phdr) + sizeof(Elf64_Ehdr);
  config.shdr_num = 0;

  SectionContainer* init = alloc_section_container_init(0, 0, 0, 0);
  config.initial_section = init;
  config.current_section = init;
  ObjectChain* oc = alloc_obj_chain_init(0, 0, 0);
  config.initial_object = oc;
  config.current_object = oc;
  // used for future.
  config.mem = __thalloc();
  
  int i = 1;
  ObjectChain* ocp;
  // this should be done as concurrent as it could be in the end.
  for (;i < argc;i++) {
    void* scr = alloc_file(argv[i]);
    ocp = 0;
    run_through_elf_shdr3(scr, &_on_section_callback_for_link, &ocp);
    run_through_elf_symtable3
      (ocp->symbol_table_p, ((Elf64_Sym*)ocp->symbol_table_p) + ocp->symbol_num,
       &_on_elf_symtab_callback_for_link, ocp->str_table_p);
  }
  // iterate_section_container(set_program_header);
  // sort_section_container_complete();
  add_pltgot_sc();
  iterate_section_container(set_virtual_address);
  add_dynamic();
  iterate_object_chain(do_reloc, 0);
  set_dynanmic();
  char* fname;
  if (config.outputfile) {
    fname = config.outfile_name;
  } else {
    fname = "_out01.o";
  }
  gen(fname);
  printf("done\n");
  return 0;  
}

