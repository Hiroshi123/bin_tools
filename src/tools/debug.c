
#include "macro.h"
#include "memory.h"
#include "elf.h"
#include "pe.h"
#include "macho.h"
#include "objformat.h"

#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <stdint.h>

#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/syscall.h>

#define SIGPT_SET \
  ((sigset_t *)(const unsigned long [_NSIG/8/sizeof(long)]){ \
    [sizeof(long)==4] = 3UL<<(32*(sizeof(long)>4)) })

#define STACK_SIZE 4096

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

extern void* __clone(void*,void*,void*,void*,void*,void*);
extern uintptr_t __mmap(void*,void*,void*,void*,void*,void*);

extern void* _tls1;

extern uint64_t* CHILD_THREAD_NUM_ADDR;

char A[] = "A";

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

static int C = 0;

void th1(void* arg,uint64_t* arg2,uint64_t* arg3,uint64_t* arg4) {

  C+=1;
  printf("th started!%p,%p,%x\n",&th1,arg,syscall(SYS_gettid));
  printf("%p,%p,%p\n",arg2,arg3,arg4);
  printf("%p,%p,%p\n",*arg2,*arg3,*arg4);
  printf("ppid!:%x,%x\n",syscall(SYS_getppid),syscall(SYS_getpid));
  union sigval value;
  value.sival_int = 1;
  value.sival_ptr = 0;
  
  // if a thread forget letting a futex memory free, and another thread cant be started.
  // you should just register all of futexes that the thread had allocated ever...
  // as of futexes, you cannot know how many number of threads are waiting on it until you wake them.
  // if it is more than one, and keep sleeping 
  
  /* if (sigqueue(syscall(SYS_getppid), SIGUSR2, value) == 0) { */
  /*   printf("signal sent successfully!!\n"); */
  /* } else { */
  /*   printf("SIGSENT-ERROR:%d\n",errno); */
  /* }   */
  sleep(5);
  printf("th1 done\n");
  fflush(stdout);
  sleep(1);
  exit(0);
}

// static int CHILD_THREAD_NUM = 0;

static void thread_term (int sig) {
  printf("got signal\n");
  C += 1;
  pid_t pid;
  pid = wait(NULL);
  printf("Pid %x exited.\n", pid);
  if (*CHILD_THREAD_NUM_ADDR != 0) {
    *CHILD_THREAD_NUM_ADDR-=1;
  }
  printf("child thread num : %d\n", *CHILD_THREAD_NUM_ADDR);
  // got_signal = 1;
}

void init_signal(int sig) {
  sigset_t mask;
  sigset_t orig_mask;
  struct sigaction act;
  memset(&act, 0, sizeof(act));
  act.sa_handler = thread_term;
  if (sigaction(SIGCHLD, &act, 0)) {
    perror ("sigaction");
    return 1;
  }
  sigset_t sigset,unblock_set;
  sigfillset(&sigset);
  sigprocmask(SIG_SETMASK, &sigset, NULL);
  sigemptyset(&unblock_set);
  sigaddset(&unblock_set, SIGHUP); // 1
  sigaddset(&unblock_set, SIGINT); // 2
  sigaddset(&unblock_set, SIGQUIT);// 3
  sigaddset(&unblock_set, SIGTERM);// 15
  sigaddset(&unblock_set, SIGCHLD);// 17
  sigprocmask(SIG_UNBLOCK, &unblock_set, NULL);
  
  printf( "Old set was %8.8lx\n", sigset);
  /* sigpending( &pset ); */
  /* printf( "Pending set is %8.8ld.\n", pset ); */  
}

int main(int argc,char** argv) {
  // file descriptor is substituted for stdin/stderr/stdout

  init_signal();

  if (fork()) {
    printf("Parent pid is %x,%x,%x\n", getppid(),getpid(),syscall(SYS_gettid));
    for (;;) {
      sleep(-1);
      if (*CHILD_THREAD_NUM_ADDR == 0) break;
      printf("child thread num:%d\n",*CHILD_THREAD_NUM_ADDR);
    }
    printf("done\n");
    exit(0);
  }
  *CHILD_THREAD_NUM_ADDR += 1;
  printf("child thread num :: %d\n", *CHILD_THREAD_NUM_ADDR);
  uint64_t* r1 = __mmap
    (0,STACK_SIZE,PROT_WRITE|PROT_READ|PROT_EXEC,MAP_ANONYMOUS|MAP_PRIVATE,-1,0);  
  uint64_t* t = malloc(8);
  uint64_t* t2 = malloc(8);
  *t = 0;
  printf("Child pid is %x,%x,%x\n", getppid(),getpid(),syscall(SYS_gettid));
  r1 = (uint64_t*)(((uint8_t*)r1) + STACK_SIZE - 8);
  *r1 = &th1;
  void* rr = __clone(0,r1,t,t2,0,0);  
  *CHILD_THREAD_NUM_ADDR += 1;
  printf("child thread num ::: %d\n", *CHILD_THREAD_NUM_ADDR);
  if (rr) {
    sleep(3);
    //
    ret = sys_futex(&mem, FUTEX_WAIT_PRIVATE, old);    
    syscall(SYS_futex, uaddr, op, val, NULL, NULL, 0);
    //uint64_t* tt = *(uint64_t*)t;
    printf("ok\n");
    printf("hei:%p,%p,%p,%d,%p,%p\n",
	   rr,errno,r1,C,t,*(uint64_t*)t);
    printf("my tid:%x\n",syscall(SYS_gettid));
    printf("ppid:%x,%x\n",syscall(SYS_getppid),syscall(SYS_getpid));
    return 0;
  } else {
    printf("0 :%p,%p\n",rr,r1);
  }
  return 1;
  
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
  printf("%s,\n",DOT_FNAME);

  /* const int out_fd1 = open(LOG_DIR"/"DOT_FNAME, O_RDWR | O_CREAT | O_TRUNC); */
  /* const int out_fd2 = open(LOG_DIR"/"MEM_FNAME, O_RDWR | O_CREAT | O_TRUNC); */
  /* EXPORT(fd_num) += 2; */
  /* if (out_fd1 == -1 || out_fd2 == -1) { */
  /*   printf("error code:%d on fd:%d,%d\n", errno, out_fd1,out_fd2); */
  /*   return 0; */
  /* } */
  /* heap* out1 = out_map_file(out_fd1); */
  /* heap* out2 = out_map_file(out_fd2); */

  // heap* out2 = out_map_file(out_fd3);  

  /* EXPORT(out_page_head) = out1->begin; */
  /* EXPORT(draw_memory_table_page_head) = out2->begin; */
  /* EXPORT(draw_memory_table_page_ptr) = out2->begin; */
  
  // EXPORT(inst_page_head) = out2->begin;
  // EXPORT(out_page_head) = out1->begin;
  /* init_graph(out1->begin); */
  /* init_draw_memory_table(out2->begin); */
  
  if (o == ELF32) {
    start_addr = load_elf32(h->begin, meta->begin);
    read_elf32(h->begin, meta->begin);
  } else if (o == ELF64) {
    /* start_addr = load_elf32(h->begin, meta->begin); */
    read_elf64(h->begin, meta->begin);
    run_through_elf_phdr3(h->begin, &_on_elf_phdr_callback, meta->begin);    
    run_through_elf_shdr3(h->begin, &_on_elf_section_callback, meta->begin);
    printf("meta begin:%x\n",meta->begin);
    
    info_on_elf* tmp = meta->begin;
    void* sym_end = (size_t)tmp->symbol_p + (size_t)tmp->symbol_size;
    int i = run_through_elf_symtable3(tmp->symbol_p, sym_end, &_on_elf_symtab_callback,tmp);
    printf("ret:%x,%x,%x\n",sizeof(Elf64_Sym),tmp,sym_end);
    // beginning of elf files, heads of function table, extra data which will be accumulated..
    // 
    return;
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

