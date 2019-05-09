

#include "memory.h"
#include "pe.h"

#include <stdio.h>
#include <stdint.h>

#include <sys/mman.h>
#include <sys/stat.h>

/* extern void* get_pure_mmap(void*); */

/*static*/ heap *HEAP_HEADER_ADDR_HEAD;
heap *HEAP_HEADER_ADDR_P;
heap *HEAP_HEADER_ADDR_TAIL;
size_t PAGE_SIZE;

extern uint64_t _rax;
extern uint64_t _rcx;
extern uint64_t _rdx;
extern uint64_t _rbx;
extern uint64_t _rsp;
extern uint64_t _rbp;
extern uint64_t _rsi;
extern uint64_t _rdi;
extern uint64_t _r8;
extern uint64_t _r9;
extern uint64_t _r10;
extern uint64_t _r11;
extern uint64_t _r12;
extern uint64_t _r13;
extern uint64_t _r14;
extern uint64_t _r15;
extern uint64_t _eflags;
extern uint64_t _rip;

extern void* _0x00_add;
extern void* _0x01_add;
extern void* _0x02_add;
extern void* _0x03_add;
extern void* _0x04_add;

extern void* _0x48_set_rex;
extern void* _0x55_push;
extern void* _0x89_mov;

extern void* _0xc3_ret;

extern void* _0xe6_port_io;
extern void* _0xe7_port_io;
extern void* _0xe8_call;
extern void* _0xff_op;

extern void _hello_world();
extern void _initialize_v_regs();
extern void _set_rsp(void*);
extern void _set_rip(void*);

extern void* _get_rip();
extern void* _get_rsp();

extern void _exec_one();
extern void* _get_host_rsp();
extern void* _get_host_rax();

extern uint64_t* _opcode_table;

void __f1() {
  void* a = _get_host_rsp();
  printf("%x\n",a);
}

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

  printf("[rax]:%x,",_rax);
  printf("[rcx]:%x,",_rcx);
  printf("[rdx]:%x,",_rdx);
  printf("[rbx]:%x,",_rbx);
  printf("[rdi]:%x,",_rdi);
  printf("[rsi]:%x\n",_rsi);

  printf("[r8]:%x,",_r8);
  printf("[r9]:%x,",_r9);
  printf("[r10]:%x,",_r10);
  printf("[r11]:%x,",_r11);
  printf("[r12]:%x,",_r12);
  printf("[r13]:%x,",_r13);
  printf("[r14]:%x,",_r14);
  printf("[r15]:%x\n",_r15);
  
  
  printf("[rbp]:%x\n",_rbp);    
  printf("[rsp]:%x\n",_rsp);
  printf("[rip]:%x\n",_rip);
  
}

heap* stack_map(void* stack_addr) {

  // stack Reserved
  void* stack_head = (uint64_t)stack_addr & 0xfffff000;
  uint32_t map_size = ((0 + 0x1000) & 0xfffff000);  
  heap* h = guest_mmap(stack_head, map_size); 
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

  start_addr = load_pe(p);
  void* stack_addr = 0x7ffffff8;
  stack_map(stack_addr);
  _hello_world();
  _initialize_v_regs();
  // guest address is set.
  _set_rsp(stack_addr);
  _set_rip(start_addr);
  
  printf("intial--------------\n");  
  print_regs();
  int count = 0;
  for (;count < 6;count++) {
    
    _exec_one();
    printf("--------------%d.--------------\n",count);
    print_regs();
    printf("-------------stack-------------:\n");
    uint64_t* p = _rsp;
    for (;p<=stack_addr;p++) {
      print_memory(p);
    }
    printf("----------------------------\n");    
  }
  
}
