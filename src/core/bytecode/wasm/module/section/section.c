
#include <stdio.h>
#include <stdint.h>

extern uint8_t* inp;
extern uint64_t* outp;
extern uint64_t* opcode_table;

/* extern uint64_t 0x00_unreachable; */

void _0x00_none() {}

void _0x01_type() {

  printf("type:%x,%x,%x,%x\n",inp, *(uint8_t*)inp,*((uint8_t*)inp+1),*((uint8_t*)inp+2));
  
  uint8_t type;
  uint8_t num_params;
  uint8_t num_results;
  uint8_t* end;
  uint8_t type_end = *((uint8_t*)inp+1);
  uint8_t* t = (uint8_t*)inp+2;
  int i=0;
  
  for (;i<type_end;t++,i++) {
    type = t;
    printf("t:%x\n",type);
    end = t + *(t+1);
    for (;t<end;t++) {}
    end = t + *(t+1);
    for (;t<end;t++) {}
  }
}

uint8_t* read_string(uint8_t* t) {
  uint8_t* e;
  e = t + 1 + *t; //string_len;
  for (t++;t<e;t++) {
    printf("%c",*t);
  }
  printf("\n");
  return t;
}

void _0x02_import() {
  printf("imports\n");
  uint8_t* t = (uint8_t*)inp+2;  
  uint8_t n = *((uint8_t*)inp+1);
  uint8_t i = 0;
  uint8_t string_len;
  for (;i<n;i++,t++) {
    // import module
    /* string_len = *t; */
    t = read_string(t);
    // import fields
    t = read_string(t);
    // if import kind is 0
    if (*t == 0) {
      // add import signature
      t++;
    } else if (*t == 2) {
      t++;// limit flags
      t++;// limit initial
    }
  }
  printf("import kind:\n");
  printf("import sig:\n");
}

void _0x03_function() {

  printf("function\n");
  
  uint8_t* t = (uint8_t*)inp+2;  
  uint8_t* e = t + *((uint8_t*)inp+1);
  uint8_t body_size;
  uint8_t local_decl_count;
  uint8_t* e2;
  for (;t<e;t++) {
    printf("num functions:%d\n",*t);
    // printf("%x\n",*t);    
  }
  /*   e2 = t + *t; */
  /*   printf("cdcd,%x\n",*t); */
  /*   local_decl_count = *(t+1); */
  /*   for (;t<e2;t++) { */
  /*     printf("%x,%x\n",t,*t);       */
  /*   } */
  /* } */
}

void _0x04_table() {}
void _0x05_memory() {}
void _0x06_global() {}
void _0x08_start() {}
void _0x09_element() {}

void _0x07_export() {
  printf("exports\n");
  uint8_t* t = (uint8_t*)inp+2;
  uint8_t* e = t + *((uint8_t*)inp+1);
  uint8_t i=0;
  for (;t<e;i++,t++) {
    printf("%x\n",t);
    /* t++; */
    // t = read_string(t);
  }  
}

// code section will tell you

// import section -> 
// export section ->
// code section

// symbol table
// 1. export table
// 2. import table

// 1.type area()
// 2.import
// 3.function area
// 4.table area
// 5.memory section
// 6.global area(static/bss section)
// 7.export
// 8.start
// 9.element
// 10.code area
// 11.

// struct of load_command
// if it is segment command(data/global/code)
// you need to add another extra_info

// 1.static area generation
// 2.code generation
// object-format file generation

// function area

// how much byte do you need for 1 function?

// 1.static area reference
// 2.other function call ()
// 3.stack simulation
// 4.jmp to imports

// symbol table(dynamic)
// you can expect how many functions are needed...

// code area(3,4,3,2)

// lea,mov (memory access)
// function call

// you cannot determine what values are fed on them when you first generate, but instead, you have to
// know how much size the funciton needs to consume on memory.

// symbol table(name,value,section,type,function)
// dynamic symbol table
// value of string

// data section
// do_link
// you should add temp index to the begging of code.
// the code will be 

// calling convention

// local.get means you need to allocate the value which was given on stack.
//

// where is the

// mov rsp rbp
// mov rbp rap

// before doing local get, you can assume the value has already been allocated if
// function has param,

// type (param) assumes the values had aleady been allocated on rbp(+4).
// all of arguments can be assumed to be set on memory at first.

// you need to remember which address holds the value that you wanna access.
// local.get 1 means (+8?)

// current register of stack needs to be kept track.

// mov (rbp+4) rax
// mov (rbp+8) rcx
// mov (rbp+12) rdx

// always keep the register which is lower.
// add rcx rdx
// add rax rcx

// after computation, you always keep the one which is on bottom.
// when you add 

// do not use immmidiate one as it is the figure after a bit of optimization.

// memory access()

// you should add everything else.

// 0x55
// 0x48 0x89 0xc0 (2,3,3)

// 0x48,0x89,(2,3,3)

// (mod >> 6) | (enum >> 3)
// mod | reg | rm
// or operation is what needs to be done.

// set_mod_reg_rm

// bit,bit,bit
// al,cl,dl ????

// type()

// mod/reg/rm , displacement, immidiate, 

// take into account of calling convention
// mov rdx, 0x48,(rbp+4)
// mov rdx, 0x48,(rbp+8)
// 0x40

// mod,reg,rm
// 0x40,0x89,0xc0
// 0x48,0x83,0x10(+16)

// op,reg,rm
// 0x40,0x89,0xc0

// gen_op_reg_rm

// 0x5d

// you should need to gain the power to believe things that you thing valuable.
// if external message had been arrived, just ignore it whatever is comes from.

// 

void _0x0a_code() {

  uint8_t* t = (uint8_t*)inp+2;  
  uint8_t n = *((uint8_t*)inp+1);
  uint8_t body_size;
  uint8_t local_decl_count;
  uint8_t* e2;
  void* f;
  printf("section size:%d\n",*((uint8_t*)inp+1));
  uint8_t i = 0;
  
  for (;i<n;i++,t++) {
    // current address + num body size 
    e2 = t + *t;
    printf("cdcd,%x,%x\n",*t,e2);
    local_decl_count = *(t+1);
    for (t+=2;t<e2;t = inp) {
      f = &opcode_table + *t;
      printf("op:%x\n",*t);
      inp = t + 1;
      // assumes inp is incremented on it.
      asm("call *%0" : : "r"(*(uint64_t*)f));      
    }
  }
}

void _0x0b_data() {
  printf("data\n");
  uint8_t* t = (uint8_t*)inp+2;  
  uint8_t* e = t + *((uint8_t*)inp+1);
  for (;t<e;t++) {
    printf("%x\n",t);
  }
}

void opt(void* t) {

  // const == 41
  // const == 41
  printf("%x\n",t);
  printf("%x\n",t);

  // (3 + 5)
  // you should add iconst

  // iconst 3(0x41)
  // iconst 4(0x41)
  // add(0x6a)  
  // iconst 7
  
  // iconst 3
  // iconst a
  // iconst 
  
  // if == 
  
  // iconst 7

  // if you find such a pattern,
  // (x+3) == 5

  // things that you can do is inserting operation on top level on the same equation...
  
  // 
  
  // (x+3) - 3 == 5 -3
  
  // iconst
  // get.local

  // iconst 
  
}

