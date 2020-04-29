
#include <stdio.h>
#include <stdint.h>

#include "elf.h"
#include "link.h"
#include "alloc.h"

static FILE* MiscLog = 0;

extern void* alloc_file(char*);
extern void  gen_elf();
extern void set_virtual_address(void* arg1);
extern void set_program_header(void* arg1);
extern void do_reloc(void* _oc, void* arg1);
extern void __p1(void* arg1);

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
      char* s = config.outfile_name;
      for (;*s != '.';s++);
      if (!strcmp(s, ".so")) {
	config.outfile_type = ET_DYN;
      } else if (!strcmp(s, ".o")) {
	config.outfile_type = ET_EXEC;
      } else {
	printf("not acceptable\n");
      }
      i++;
      continue;
    } else if (!strcmp(argv[i], "-p")) {
      config.pack = 1;
      continue;
    } else if (!strcmp(argv[i], "-nodynamic")) {
      config.nodynamic = 1;
      continue;
    }
    *p = argv[i];
    p++;
    printf("%s\n", argv[i]);
  }
  *p = 0;
  if (!config.outfile_type) {
    config.outfile_type = ET_EXEC;
  }
  return _p;
}

int main(int argc, char** argv) {

  /* if (argc == 1) { */
  /*   printf("need at least 1 argc\n"); */
  /*   return 0; */
  /* } */
  logger_init("misc.log");
  mem_init();
  size_t* p1 = read_cmdline(argc, argv);
  
  if (p1 == 0) {
    printf("usage\n");
    printf("-ef : specify an entry point function name\n");
    printf("-ib : specify image base\n");
    printf("-o : specify an outputfile. candidate suffix .exe/.dll/.o/.so\n");
    return 0;
  }
  static_data_init(&config);
  init_hashtable("import_list.sqlite3");
  if (config.outfile_type == ET_DYN) {
    config.base_address = 0x00000;
  } else {
    config.base_address = 0x400000;
  }
  if (config.nodynamic) {
    config.dynamic_entry_num = 1;
  } else {
    config.dynamic_entry_num = 9;
  }
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
  
  for (;*p1;p1++) {
    void* scr = alloc_file((void*)*p1);
    ocp = 0;
    run_through_elf_shdr3(scr, &_on_section_callback_for_link, &ocp);
    run_through_elf_symtable3
      (ocp->symbol_table_p, ((Elf64_Sym*)ocp->symbol_table_p) + ocp->symbol_num,
       &_on_elf_symtab_callback_for_link, ocp->str_table_p);
  }
  if (config.pack)
    iterate_section_container(__p1);
  
  // return;
  // iterate_section_container(set_program_header);
  // sort_section_container_complete();
  if (config.nodynamic == 0)
    add_pltgot_sc();
  iterate_section_container(set_virtual_address);
  if (config.nodynamic == 0) {
    add_dynamic();    
  } else {    
    add_dynamic_sc();
  }
  if (config.outfile_type == ET_DYN) {
    add_export_symbol();
  } else {
    add_dt_needed("/mnt/c/Users/hiroshi.sawada/myrepo/bin_tools/pika.so");
    printf("dt needed added\n");
  }
  printf("v\n");
  iterate_object_chain(do_reloc, 0);
  printf("nn\n");
  set_dynanmic();
  
  char* fname;
  if (config.outfile_name) {
    fname = config.outfile_name;
  } else {
    fname = "a.out";
  }
  gen(fname);
  printf("done\n");
  return 0;
}

