
#include <stdint.h>
#include <stdio.h>

#include "elf.h"

void _on_elf_phdr_callback(void* arg1, Elf64_Phdr* arg2, void* arg3) {
  printf("phdr callback,%p,type:%d,%p,%p,%p\n", arg1, arg2->p_type,
         arg2->p_vaddr, arg2->p_paddr, arg3);
}

void _on_elf_symtab_callback(Elf64_Sym* arg1, info_on_elf* e1) {
  //
  uint8_t* p = (uint8_t*)e1;
  printf("sym tab callback!,%s,%d,%d,%d\n", p + arg1->st_name, arg1->st_size,
         arg1->st_shndx, arg1->st_value);
  alloc_symbol_chain();
}

void _on_elf_section_callback(uint8_t* arg1, uint8_t* strtable,
                              Elf64_Shdr* arg2, void* p) {
  char* sh_name = (uint8_t*)strtable + arg2->sh_name;
  // Elf64_Shdr* ss = strtable;
  for (;;)
    ;
  // printf("ppp!!!!%p,%p,%s\n",arg2,strtable,sh_name);

  // read_debug_info();

  // if the arugments are .debug_info, you just need to retrieve the value...
  /* switch (arg2->sh_type) { */
  /* case SHT_PROGBITS : { */
  /*   if (!strcmp(".debug_info",(uint8_t*)strtable + arg2->sh_name)) { */
  /*     printf("this is debug info!!\n"); */
  /*     printf("offset:%x\n",arg1 + arg2->sh_offset); */
  /*     uint64_t* u = arg1 + arg2->sh_offset; */
  /*     uint64_t* e = u + 0x10; */
  /*     for (;u<e;u++) { */
  /* 	printf("%x,%16p\n",u,*u); */
  /*     } */
  /*     printf("size:0x%x\n",arg2->sh_size); */
  /*   } */
  /*   else if (!strcmp(".debug_line",(uint8_t*)strtable + arg2->sh_name)) { */
  /*     printf("debug line!\n"); */
  /*     uint64_t* u = arg1 + arg2->sh_offset; */
  /*     uint64_t* e = u + 0x10; */
  /*     for (;u<e;u++) { */
  /* 	printf("%x,%16p\n",u,*u); */
  /*     } */
  /*     printf("size:0x%x\n",arg2->sh_size); */
  /*   } */
  /* } */
  /* case SHT_SYMTAB:{ */
  /*   printf("symtab\n"); */
  /* } */

  /* } */
  /* printf("%d\n",1); */
  /* printf("on elf section:%x,%x,%x\n",arg1,arg0,arg2); */
  /* printf("%x,%s\n",arg2->sh_type,(uint8_t*)arg0 + arg2->sh_name); */

  /* printf("on elf section:%d,%s,%p\n",arg2->sh_type, */
  /* 	 (uint8_t*)arg0 + arg2->sh_name,arg1); */
}
