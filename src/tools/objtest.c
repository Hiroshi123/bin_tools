
#include <stdio.h>
#include <stdlib.h>

#include "elf.h"
#include "macho.h"
#include "memory.h"
#include "pe.h"
#include "objformat.h"

// what needs to be resoled is
// 1. calls to functions of test subject on test function.
// 2. calls from test subject functions to mock function which are functions
// defined on external object format files.

void show_usage() {
  const char *usage = "1st argument:test & mock file\n"
                      "2nd argument:test subject file\n";
  printf("%s", usage);
}

char *process_elf(heap *a, heap *b) {
  info_on_elf e1;
  info_on_elf e2;
  const char *test_ = "test__";
  read_elf(a->begin, &e1);
  read_elf(b->begin, &e2);
  char *r = do_relocs(test_, &e1, &e2);
  return r;
}

char *process_macho(heap *a, heap *b) {
  info_on_macho macho1;
  info_on_macho macho2;
  read_macho(a->begin, &macho1);
  read_macho(b->begin, &macho2);
  // for macho every function on it will be subject of relocation
  // as you are not able to guess caller-callee dependency from
  // its symbol table.
  do_reloc_all(&macho1, &macho2);
  do_reloc_all(&macho2, &macho1);
  // on macho, every function puts prefix _ ahead of it,
  const char* test_ = "_test__";
  return find_test(test_, &macho1);
}

char *process_coff(heap *a, heap *b) {
  info_on_coff e1;
  info_on_coff e2;
  read_coff(a->begin, &e1);
  read_coff(b->begin, &e2);
  do_reloc_coff(&e1, &e2);
  do_reloc_coff(&e2, &e1);
  const char* test_ = "test__";
  return find_test_(test_, &e1);
}

// testing will be done by writing inline assembly.
// Note that inline assembly does not seem to support register-variable &
// variable-register assignment. Assignment your value to a specific register is
// done from a memory address where you know you allocated surely on running
// time. You can use "mov 0x10,%rax" but you cannot do with "mov %0,%rax" at
// least in AT&T syntax. Instead, you can set & get in a following way. Setter
// 1. set %rax by calling a dummy function and return your desired value on it.
// 2. mov %rax to your desired register by assignment in betweeen pair of
// registers. Getter
// 1. set %rax by calling a dummpy function again which returns address.
// 2. You will assign the value on your desired regiser on the address that
// you've got previously.

// that is the trick where you can assign %rdi %rsi as arguments, and get
// the returned value after calling a function on %rax on x86.

void do_test(char *r) {
  
  heap* h = get_page(1);
  // call once for later another call.
  if ( h->begin != get_page_head()) {
    printf("error\n");
  }
  char *_a = (char *)(h->begin);
  // this is going to be the arugment to the test-subject function.
  printf("argument:rdi is %d\n", *_a);
  printf("argument:+4(rdi) is %d\n", *(_a + 4));
  
  asm("call *%0" : : "r"(get_page_head));
  asm("mov %rax,%rdi");
  asm("call *%0" : : "r"(r));
  asm("mov %rax,%rbx");
  asm("call *%0" : : "r"(get_page_head));
  asm("mov %rbx,(%rax)");
  printf("returned value on rax is %d\n", *_a);
  
}

do_instrument(uint8_t* r) {

  printf("r:%x,%x\n",r,*r);
  decode(r);
  /* for (;;c++) { */    
  /* } */
  
}

int main(int argc, char **argv) {
  if (argc < 3) {
    show_usage();
    exit(0);
  }
  heap *a = init_map_file(argv[1]);
  heap *b = init_map_file(argv[2]);
  char ao = detect_format(a->begin);
  char bo = detect_format(b->begin);
  if (ao != bo) {
    printf("currently support only a pair of same format file.\n");
  }
  char *r;
  switch (ao) {
  case ELF:
    r = process_elf(a, b);
    break;
  case MACHO:
    r = process_macho(a, b);
    if (r == 0) {
      printf("not found\n");
    }
    break;
  case PE:
    // pe might be difficult as its definition
    // means executable but not relocatable..
    // Ummm, needs to be considered...
    // r = process_pe(a, b);
    printf("not yet..\n");
    return 0;
    // break;
  case COFF:
    r = process_coff(a, b);
    return 0;
    break;
  case NONE:
    printf("error\n");
    return 0;
    // break;
  }
  do_instrument(r);
  // do_test(r);
  return 0;
}
