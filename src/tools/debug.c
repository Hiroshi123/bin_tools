
#include "macro.h"
#include "memory.h"
#include "elf.h"
#include "pe.h"
#include "macho.h"
#include "objformat.h"

#include <fcntl.h>
#include <stdio.h>
#include <stdint.h>

#include <sys/mman.h>
#include <sys/stat.h>

// for making function call graph.
// this represents pinter to current function.
p_host CURRENT_FNAME = 0;

extern uint8_t EXPORT(processor);
extern uint8_t EXPORT(objformat);

extern uint64_t EXPORT(rax);
extern uint64_t EXPORT(rcx);
extern uint64_t EXPORT(rdx);
extern uint64_t EXPORT(rbx);
extern uint64_t EXPORT(rsp);
extern uint64_t EXPORT(rbp);
extern uint64_t EXPORT(rsi);
extern uint64_t EXPORT(rdi);
extern uint64_t EXPORT(r8);
extern uint64_t EXPORT(r9);
extern uint64_t EXPORT(r10);
extern uint64_t EXPORT(r11);
extern uint64_t EXPORT(r12);
extern uint64_t EXPORT(r13);
extern uint64_t EXPORT(r14);
extern uint64_t EXPORT(r15);
extern uint64_t EXPORT(eflags);
extern uint64_t EXPORT(rip);

extern void _hello_world();
extern void EXPORT(_initialize_v_regs());
extern void EXPORT(_set_rsp(void*));
extern void EXPORT(_set_rip(void*));

extern void EXPORT(exec_one());

extern uint64_t* _opcode_table;
extern uint8_t EXPORT(debug);

void print_memory(void* guest_addr) {

  uint64_t diff = EXPORT(get_diff_host_guest_addr(guest_addr));
  uint8_t* host_addr = (uint8_t*)((uint64_t)guest_addr + diff);  
  printf("%x(%x),0x",guest_addr,host_addr);
  uint8_t* end = host_addr + 8;
  for (;host_addr!=end;host_addr++) {
    printf("%02x",*host_addr);
  }
  printf("\n");
}

void print_inst() {
  printf("[instruction] : ");
  uint8_t* p = &EXPORT(debug);
  uint8_t* s = (uint8_t*)(&EXPORT(debug) + 0x10);
  uint8_t* e = p + *s - 1;
  for (;p<e;p++) printf("%x,",*p);  
  printf("%x\n",*p);
  *s = 0;
}

void print_regs() {

  printf("[rax]:%x,",EXPORT(rax));
  printf("[rcx]:%x,",EXPORT(rcx));
  printf("[rdx]:%x,",EXPORT(rdx));
  printf("[rbx]:%x,",EXPORT(rbx));
  printf("[rdi]:%x,",EXPORT(rdi));
  printf("[rsi]:%x\n",EXPORT(rsi));

  printf("[r8]:%x,",EXPORT(r8));
  printf("[r9]:%x,",EXPORT(r9));
  printf("[r10]:%x,",EXPORT(r10));
  printf("[r11]:%x,",EXPORT(r11));
  printf("[r12]:%x,",EXPORT(r12));
  printf("[r13]:%x,",EXPORT(r13));
  printf("[r14]:%x,",EXPORT(r14));
  printf("[r15]:%x\n",EXPORT(r15));
  
  printf("[rbp]:%x\n",EXPORT(rbp));    
  printf("[rsp]:%lx\n",EXPORT(rsp));
  printf("[rip]:%x\n",EXPORT(rip));
  
}

heap* stack_map(void* stack_addr) {

  // stack Reserved
  void* stack_head = (uint64_t)stack_addr & 0xfffff000;
  uint32_t map_size = ((0 + 0x1000) & 0xfffff000);  
  heap* h = guest_mmap(stack_head, map_size, 0, 0); 
  return h;
}

p_host check_fname(void* meta, p_guest f_addr) {
  if (EXPORT(objformat) == ELF32) {
    return get_name_of_f_on_elf32(f_addr, meta);
  } 
}

int main(int argc,char** argv) {
  
  int fd = open(argv[1], O_RDONLY);
  
  /* uint8_t* p = (uint8_t*)h->begin; */
  uint32_t header_size = 0;
  enum OBJECT_FORMAT o = detect_format(fd, &header_size);
  EXPORT(objformat) = o;
  struct stat stbuf;
  if (fstat(fd, &stbuf) == -1) {
    close(fd);
    return 0;
  }
  heap * h = map_file(fd, stbuf.st_size);
  heap* meta = get_page(1);
  p_guest start_addr;
  if (o == ELF32) {
    start_addr = load_elf32(h->begin, meta->begin);
    printf("start:%x\n",start_addr);
    read_elf32(h->begin, meta->begin);
  } else if (o == ELF64) {
    // start_addr = load_elf32(h->begin, meta->begin);
    // read_elf32(h->begin, meta->begin);
  } else if (o == MACHO32) {
    info_on_macho i;
    load_macho32(h->begin,&i);
    EXPORT(processor) = 0x32;
    start_addr = i.entry;
  } else if (o == MACHO64) {
    info_on_macho i;
    load_macho64(h->begin,&i);
    EXPORT(processor) = 0x64;
    start_addr = i.entry;
  } else if(o == PE32) {
    EXPORT(processor) = 0x32;
    start_addr = load_pe(h->begin);
  } else if(o == PE64) {
    EXPORT(processor) = 0x64;
    start_addr = load_pe(h->begin);
  } else {
    fprintf(stderr,"format error\n");
  }
  // 0x7ffffff8;
  void* stack_addr = 0x8010a5c0;
  stack_map(stack_addr);
  EXPORT(hello_world());
  EXPORT(initialize_v_regs());
  // guest address is set.
  EXPORT(set_rsp(stack_addr));
  EXPORT(set_rip(start_addr));
  
  printf("intial--------------\n");
  print_regs();
  CURRENT_FNAME = check_fname(meta->begin,start_addr);
  printf("%s\n",CURRENT_FNAME);
  int count = 0;
  for (;count < 70;count++) {
    EXPORT(exec_one());
    printf("--------------%d.--------------\n",count);
    print_regs();
    print_inst();
    printf("-------------stack-------------:\n");
    uint64_t* p = EXPORT(rsp);
    for (;p<=stack_addr;p++) {
      print_memory(p);
    }
    printf("----------------------------\n");    
  }
}

