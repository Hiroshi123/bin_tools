
#include "macro.h"
#include "memory.h"
#include "elf.h"
#include "pe.h"
#include "macho.h"
#include "objformat.h"

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdint.h>

#include <sys/mman.h>
#include <sys/stat.h>

extern uint8_t EXPORT(processor);
extern uint8_t EXPORT(objformat);
extern uint64_t EXPORT(meta_page_head);
extern uint64_t EXPORT(meta_page_ptr);
extern uint64_t EXPORT(out_page_head);
extern uint64_t EXPORT(out_page_ptr);
extern uint64_t EXPORT(draw_memory_table_page_head);
extern uint64_t EXPORT(draw_memory_table_page_ptr);

extern uint64_t EXPORT(current_fname);
extern uint64_t EXPORT(fd_num);

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

void init_graph(uint8_t* out) {
  char* header = "digraph g {\n";
  p_host offset = out + strlen(header);
  memcpy(out, header, strlen(header));
  EXPORT(out_page_ptr) = offset;
}

void init_draw_memory_table(uint8_t* out) {
  char* h1 = " | page | host | guest | page_num | flags |\n";
  char* h2 = " |---|---|---|---|---|\n";
  p_host offset = out;
  memcpy(offset, h1, strlen(h1));
  offset += strlen(h1);
  memcpy(offset, h2, strlen(h2));
  offset += strlen(h2);
  EXPORT(draw_memory_table_page_ptr) = offset;
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
  set_page_path1(get_page_path1() + 1);
  set_page_path2('0');
  int fd = open_page_map();
  uint64_t name_or_parent_addr = "stack";
  heap* h = guest_mmap(stack_head, map_size, 0, name_or_parent_addr, fd); 
  return h;
}

void start_emu(void* stack_addr) {
  printf("intial--------------\n");
  print_regs();
  int count = 0;
  for (;count < 28;count++) {
    EXPORT(exec_one());
    printf("--------------%d.--------------\n",count);
    print_regs();
    print_inst();
    stack_addr = 0;
    if (stack_addr) {
      printf("-------------stack-------------:\n");
      uint64_t* p = EXPORT(rsp);      
      for (;p<=stack_addr;p++) {
	print_memory(p);
      }
      printf("----------------------------\n");
    }
  }
}

// should bios code be mapped in a way that other program will is mapped???
// it is already on mother board..
void map_bios() {
  
}

void do_bios() { 
  
}

void do_drive(char* name) {
  int fd = open(name, O_RDONLY);
  EXPORT(fd_num) += 1;
  uint8_t block[512];
  if (!read(fd, &block, 512)) goto error;
  p_guest start_addr = 0x7c00;
  p_guest offset = start_addr & 0x00000fff;
  uint32_t map_size = ((0x1000 + 0x512) & 0xfffff000);
  heap* h = guest_mmap(start_addr & 0xfffff000, map_size , 0, 0, -1);
  // memset();
  // put 1block(=512byte) not on the beginning of a page, but
  // on the end of it.
  memcpy
    (h->begin + offset,
     &block,
     512
     );
  EXPORT(set_rip(start_addr));
  // you need to map only first 512byte.
  start_emu(NULL);
 error:
  printf("error\n");
}

int main(int argc,char** argv) {
  // file descriptor is substituted for stdin/stderr/stdout
  EXPORT(fd_num) = 2;
  int fd;
  // if it is drive, read first 512 byte.
  if (argc < 2) {
    printf("not enough argument.\n");
  }
  if (!strcmp("-drive", argv[1])) {
    // bios is actually not going to be mapped but is already embedded on the last part of memory(ROM).
    // the first address which is jumped is called as reset vector.
    map_bios();
    //
    do_bios();
    
    do_drive(argv[2]);
    exit(1);
  }
  fd = open(argv[1], O_RDONLY);
  EXPORT(fd_num) += 1;
  uint32_t header_size = 0;
  enum OBJECT_FORMAT o = detect_format(fd, &header_size);
  EXPORT(objformat) = o;
  p_guest start_addr;
  struct stat stbuf;
  if (fstat(fd, &stbuf) == -1) {
    close(fd);
    return 0;
  }
  heap * h = map_file(fd, stbuf.st_size, -1);
  heap* meta = get_page(1);
  EXPORT(meta_page_head) = meta->begin;
  EXPORT(meta_page_ptr) = meta->begin;
  const int out_fd1 = open(LOG_DIR"/"DOT_FNAME, O_RDWR | O_CREAT | O_TRUNC);
  const int out_fd2 = open(LOG_DIR"/"MEM_FNAME, O_RDWR | O_CREAT | O_TRUNC);
  EXPORT(fd_num) += 2;
  if (out_fd1 == -1 || out_fd2 == -1) {
    printf("error code:%d on fd:%d,%d\n", errno, out_fd1,out_fd2);
    return 0;
  }
  heap* out1 = out_map_file(out_fd1);
  heap* out2 = out_map_file(out_fd2);
  // heap* out2 = out_map_file(out_fd3);  
  EXPORT(out_page_head) = out1->begin;
  EXPORT(draw_memory_table_page_head) = out2->begin;
  EXPORT(draw_memory_table_page_ptr) = out2->begin;
  
  // EXPORT(inst_page_head) = out2->begin;
  // EXPORT(out_page_head) = out1->begin;
  init_graph(out1->begin);
  init_draw_memory_table(out2->begin);
  
  if (o == ELF32) {
    start_addr = load_elf32(h->begin, meta->begin);
    read_elf32(h->begin, meta->begin);
  } else if (o == ELF64) {
    // start_addr = load_elf32(h->begin, meta->begin);
    // read_elf32(h->begin, meta->begin);
  } else if (o == MACHO32) {
    load_macho32(h->begin,meta->begin);
    EXPORT(processor) = 0x32;
    start_addr = ((info_on_macho*)(meta->begin))->entry;
  } else if (o == MACHO64) {
    load_macho64(h->begin,meta->begin);        
    EXPORT(processor) = 0x64;
    start_addr = ((info_on_macho*)(meta->begin))->entry;
  } else if (o == PE32) {
    EXPORT(processor) = 0x32;
    start_addr = load_pe(h->begin);
  } else if (o == PE64) {
    EXPORT(processor) = 0x64;
    start_addr = load_pe(h->begin);
  } else {
    fprintf(stderr,"format error\n");
  }
  // 0x7ffffff8;
  void* stack_addr = 0x8010a5c0;
  stack_map(stack_addr);
  EXPORT(initialize_v_regs());
  // guest address is set.
  EXPORT(set_rsp(stack_addr));
  EXPORT(set_rip(start_addr));
  EXPORT(current_fname) = check_fname(meta->begin, start_addr, o);
  
  do_reloc("_printf", meta->begin);
  sleep(-1);
  start_emu(stack_addr);
  
}

