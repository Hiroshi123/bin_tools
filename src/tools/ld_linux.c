
#include <stdio.h>
#include <stdint.h>

#include "elf.h"
#include "link.h"
#include "alloc.h"

extern void* alloc_file(char*);
extern void  gen_elf();
extern void set_virtual_address(void* arg1);
extern void do_reloc(void* _oc, void* arg1);

// extern void* alloc_section_container_init(void*, void*, void*, void*);
static void* RelocationSectionOffset;

static Config config;

int main(int argc, char** argv) {

  logger_init();
  mem_init();
  static_data_init(&config);
  init_hashtable("import_list.sqlite3");
  config.virtual_address_offset = 0x400000;
  config.output_vaddr_alignment = 0x200000;
  config.entry_address = 0x4000b0;
  config.program_header_num = 2;
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
  // this should be done as concurrent as it could be in the end.
  for (;i < 3;i++) {
    void* scr = alloc_file(argv[i]);
    sh_callback_arg* p =__malloc(sizeof(sh_callback_arg));
    run_through_elf_shdr3(scr, &_on_section_callback_for_link, p);
    run_through_elf_symtable3
      (p->symbol_offset, ((uint8_t*)p->symbol_offset) + p->symbol_size,
       &_on_elf_symtab_callback_for_link, p->str_offset);
  }
  count_program_header();
  iterate_section(set_virtual_address);
  iterate_object_chain(do_reloc, 0);
  gen();  
  return 0;

}

