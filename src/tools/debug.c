
#include "macro.h"
#include "memory.h"
#include "pe.h"
#include "macho.h"

#include <stdio.h>
#include <stdint.h>

#include <sys/mman.h>
#include <sys/stat.h>

/* extern void* get_pure_mmap(void*); */

/*static*/ heap *HEAP_HEADER_ADDR_HEAD;
heap *HEAP_HEADER_ADDR_P;
heap *HEAP_HEADER_ADDR_TAIL;
size_t PAGE_SIZE;

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

/* extern void* _get_rip(); */
/* extern void* _get_rsp(); */

extern void EXPORT(exec_one());

extern uint64_t* _opcode_table;


void __f2() {


}

void print_memory(void* guest_addr) {

  uint64_t diff = get_diff_host_guest_addr(guest_addr);
  uint8_t* host_addr = (uint8_t*)((uint64_t)guest_addr + diff);  
  printf("%x(%x),0x",guest_addr,host_addr);
  uint8_t* end = host_addr + 8;
  for (;host_addr!=end;host_addr++) {
    printf("%02x",*host_addr);
  }
  printf("\n");
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
  printf("[rsp]:%x\n",EXPORT(rsp));
  printf("[rip]:%x\n",EXPORT(rip));
  
}

heap* stack_map(void* stack_addr) {

  // stack Reserved
  void* stack_head = (uint64_t)stack_addr & 0xfffff000;
  uint32_t map_size = ((0 + 0x1000) & 0xfffff000);  
  heap* h = guest_mmap(stack_head, map_size, 0, 0); 
  return h;
}

int main(int argc,char** argv) {
  
  char* t1 = argv[1];
  for (;*t1!=0;t1++) {
    printf("%x,%c\n",*t1,tolower(*t1));
    *t1 = tolower(*t1); 
  }
  
  printf("%s,%c\n",argv[1],tolower(argv[1]));
  
  heap * h = init_map_file(argv[1]);
  uint8_t* p = (uint8_t*)h->begin;
  uint8_t a = detect_format(p);
  printf("%x,%x,%d\n",p, *p,a);
  printf("%d,%x\n",h->page_num,h->begin);
  uint32_t start_addr;
  if (a == 2) {
    info_on_macho i;
    read_macho(h->begin,&i, 1);
    start_addr = i.entry;
  }
  
  void* stack_addr = 0x7ffffff8;
  stack_map(stack_addr);
  EXPORT(hello_world());
  EXPORT(initialize_v_regs());
  // guest address is set.
  EXPORT(set_rsp(stack_addr));
  EXPORT(set_rip(start_addr));
  
  printf("intial--------------\n");  
  print_regs();
  int count = 0;
  for (;count < 1;count++) {
    
    EXPORT(exec_one());
    printf("--------------%d.--------------\n",count);
    print_regs();
    printf("-------------stack-------------:\n");
    uint64_t* p = EXPORT(rsp);
    for (;p<=stack_addr;p++) {
      print_memory(p);
    }
    printf("----------------------------\n");    
  }
  
}

