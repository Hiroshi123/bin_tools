
// in default, gcc will attach these functions with an executable.

// .interp

// .interp,
// .note.ABI-tag,
// .note.gnu.build-id,
// .gnu.hash,
// .dynsym,
// .dynstr,
// .gnu.version,
// .rela.dyn,
// .rela.plt,
// .init
// .plt
// .plt.got
// .text
// .fini
// .rodata
// .eh_frame_hdr
// .eh_frame
// 

// what needs to be done on .text section

// 1. remove pre-fixed codes.

// _start
// register_tm_clones
// deregister_tm_clones
// __do_global_dtors_aux
// frame_dummy

// __libc_csu_init
// __libc_csu_fini

// 2. examine callee address which calls .plt.

// you must know if the call is for .plt or internal.
// 0xe8 0x00 0x00 0x00 0x00
// if the call is internal and if you do not change the gap in-between, you can let it as it is.
// if the call is external, it is pretty sure that you need to reassign the destination address of the call as
// .plt is often put before .text and beggining of .plt, some codes which are supposed to be removed are put.
// 

#include <stdio.h>
#include "elf.h"
#include "link.h"

// static uint8_t __libc_csu_fini[] = {0x4c, 0x8d, 0x05};
// static uint8_t __libc_csu_init[] = {0x48, 0x8d, 0x0d};

void f1() {
  
}

// most of the section must start from _start.
void extract_main(uint8_t* p, uint8_t* ip) {
  
  // plt + got should be combined.  
  int off = 0xf;
  uint32_t* q = p + off + 0x3;
  off += 0x7;
  uint8_t* __libc_csu_fini = ip + off + *q;
  q = p + off + 0x3;
  off += 0x7;
  uint8_t* __libc_csu_init = ip + off + *q;
  q = p + off + 0x3;
  off += 0x7;
  uint8_t* main = ip + off + *q;
  
  printf("!!!!!!!!!!!%p,%p,%p\n",
	 __libc_csu_fini,
	 __libc_csu_init,
	 main
	 );

  // find the address which you need to fill in
  // 
  
  // construct .rela.text
  // .rela.plt
  
  
  uint8_t* p1 = (uint8_t*)q + 0x4 + *q;
  uint8_t i = 0;
  for (;i < 0x20;p1++,i++) {
    printf("%d,%p,%p\n", i,p1,*p1);
  }  
  
  // you need to point on current address.
  // 0x4c 0x8d 0x05
  // 0x48 0x8d 0x0d
  // 0x48 0x8d 0x3d
  // 0xff 0x15 
  
}

void __p1(void* arg1) {

  SectionContainer* sc = arg1;
  SectionChain* schain = sc->init;
  if (sc->init == 0) return;
  if (sc->name) {
    printf("%s\n", sc->name);
  }
  Elf64_Shdr* shdr = schain->p;
  if (!strcmp(sc->name, ".text")) {
    extract_main(shdr->sh_offset, shdr->sh_addr);
  }  
  printf("sh_size:%d\n", shdr->sh_size);
  uint64_t* p = shdr->sh_offset;
  printf("sh_size:%p,%p\n", p, *p);  
}

