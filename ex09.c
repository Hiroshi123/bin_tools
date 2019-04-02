
#include <stdio.h>
#include <stdint.h>
#include <sys/mman.h>
#include <sys/stat.h>

#define MAKE_GPR(x) \
    union {\
    uint64_t _hl;\
    struct {\
      uint32_t _h;\
      uint32_t _l;\
    } x ## 32;\
    struct {\
      uint16_t _h1;\
      uint16_t _h2;\
      uint16_t _l1;\
      uint16_t _l2;\
    } x ## 16;\
    struct {\
      uint8_t _h1;\
      uint8_t _h2;\
      uint8_t _h3;\
      uint8_t _h4;\     
      uint8_t _l1;\
      uint8_t _l2;\
      uint8_t _l3;\
      uint8_t _l4;\     
    } x ## 8;\
    } x ## 64;\

typedef struct {

  MAKE_GPR(ax)
  MAKE_GPR(cx)
  MAKE_GPR(dx)
  MAKE_GPR(bx)
  MAKE_GPR(sp)
  MAKE_GPR(bp)
  MAKE_GPR(si)
  MAKE_GPR(di)

  MAKE_GPR(r8_)
  MAKE_GPR(r9_)
  MAKE_GPR(r10_)
  MAKE_GPR(r11_)
  MAKE_GPR(r12_)
  MAKE_GPR(r13_)
  MAKE_GPR(r14_)
  MAKE_GPR(r15_)
  
  MAKE_GPR(rip)

  uint32_t eflags;

  // segment register
  uint64_t ss;
  uint64_t cs;
  uint64_t ds;
  uint64_t es;
  uint64_t fs;
  uint64_t gs;  
} _REG;

_REG REG;

// RAX
#define RAX REG.ax64._hl
#define EAX REG.ax64.ax32._l
#define AX  REG.ax64.ax16._l2
#define AH  REG.ax64.ax8._l3
#define AL  REG.ax64.ax8._l4
// RCX
#define RCX REG.cx64._hl
#define ECX REG.cx64.cx32._l
#define CX  REG.cx64.cx16._l2
#define CH  REG.cx64.cx8._l3
#define CL  REG.cx64.cx8._l4
// RDX
#define RDX REG.dx64._hl
#define EDX REG.dx64.dx32._l
#define DX  REG.dx64.dx16._l2
#define DH  REG.dx64.dx8._l3
#define DL  REG.dx64.dx8._l4
// RBX
#define RBX REG.bx64._hl
#define EBX REG.bx64.bx32._l
#define BX  REG.bx64.bx16._l2
#define BH  REG.bx64.bx8._l3
#define BL  REG.bx64.bx8._l4
// RSP
#define RSP REG.sp64._hl
#define ESP REG.sp64.sp32._l
#define SP  REG.sp64.sp16._l2
#define SPH REG.sp64.sp8._l3
#define SPL REG.sp64.sp8._l4
// RBP
#define RBP REG.bp64._hl
#define EBP REG.bp64.bp32._l
#define BP  REG.bp64.bp16._l2
#define BPH REG.bp64.bp8._l3
#define BPL REG.bp64.bp8._l4
// RSI
#define RSI REG.si64._hl
#define ESI REG.si64.si32._l
#define SI  REG.si64.si16._l2
#define SIH REG.si64.si8._l3
#define SIL REG.si64.si8._l4
// RDI
#define RDI REG.di64._hl
#define EDI REG.di64.di32._l
#define DI  REG.di64.di16._l2
#define DIH REG.di64.di8._l3
#define DIL REG.di64.di8._l4


// R8
#define R8 REG.r8_64._hl
#define E8 REG.r8_64.r8_32._l
#define _8X REG.r8_64.r8_16._l2
#define _8H REG.r8_64.r8_8._l3
#define _8L REG.r8_64.r8_8._l4

#define RIP REG.rip64._hl

#define REG_OFFSET8 REG.ax64.ax8._h1
#define REG_OFFSET64 RAX
#define AUX_REG_OFFSET REG.ax64.r8_64.r8_8._h1
#define AUX_REG_OFFSET64 R8

// #define ax REG.ax64.ax16._l2

static uint64_t OPCODE[256] =
  {
    // 0x00 [OP_ADDL] :: reg is source. r/m is destination.
    // 0x01
    // 0x02 [OP_ADDR] :: reg is destination. r/m is source.
    // 0x03 
    // 0x04
    // 0x05

    // 0x08 [OP_ORL] :: reg is source. r/m is destination.
    // 0x09
    
    // 0x10 [OP_ADCL]

    // 0x18 [OP_SBBL]

    // 0x20 [OP_ANDL]

    // 0x28 [OP_SUBL]

    // 0x30 [OP_XORL]

    // 0x38 [OP_CMPL]

    // 0x40,
    // 0x41,
    
  };

static uint64_t* IP;

typedef struct {  
  uint8_t prefix;
  uint8_t rex;
} context;

context CTX;

// prefix
#define PREFIX_REPZ   0x01
#define PREFIX_REPNZ  0x02
#define PREFIX_LOCK   0x04
#define PREFIX_DATA   0x08
#define PREFIX_ADR    0x10
#define PREFIX_VEX    0x20

// rex prefix
#define REX_W 0b00001000
#define REX_R 0b00000100
#define REX_I 0b00000010
#define REX_B 0b00000001
#define PREFIX_DATA 0b00010000
#define PREFIX_ADDR 0b00100000

// eflags values
#define CF   0b00000000000000000000000000000001
#define PF   0b00000000000000000000000000000100
#define AF   0b00000000000000000000000000010000
#define ZF   0b00000000000000000000000001000000
#define SF   0b00000000000000000000000010000000
#define TF   0b00000000000000000000000100000000
#define IF   0b00000000000000000000001000000000
#define DF   0b00000000000000000000010000000000
#define OF   0b00000000000000000000100000000000
#define IOPL 0b00000000000000000011000000000000
#define NT   0b00000000000000000100000000000000
#define WF   0b00000000000000010000000000000000
#define VM   0b00000000000000100000000000000000
#define AC   0b00000000000001000000000000000000
#define VIF  0b00000000000010000000000000000000
#define VIP  0b00000000000100000000000000000000
#define ID   0b00000000001000000000000000000000

#define OP static void

enum {
  MO_8,
  MO_16,
  MO_32,
  MO_64,
};

enum {
    OP_ADDL,
    OP_ORL,
    OP_ADCL,
    OP_SBBL,
    OP_ANDL,
    OP_SUBL,
    OP_XORL,
    OP_CMPL,
};

enum {
    OP_ROL,
    OP_ROR,
    OP_RCL,
    OP_RCR,
    OP_SHL,
    OP_SHR,
    OP_SHL1, /* undocumented */
    OP_SAR = 7,
};

// 0xf5/0xf6
enum {
    OP_TEST = 0,
    OP_NOT = 2,
    OP_NEG,
    OP_MUL,
    OP_IMUL,
    OP_DIV,
    OP_IDIV,
};

// 0xfe/0xff
enum {
    OP_INC,
    OP_DEC,
    OP_CAL,
    OP_LCAL,
    OP_JMP,
    OP_LJMP,
    OP_PUSH,
};

enum {
    JCC_O,
    JCC_B,
    JCC_Z,
    JCC_BE,
    JCC_S,
    JCC_P,
    JCC_L,
    JCC_LE,
};

static uint8_t mod_reg_rm(uint8_t* _reg, uint8_t* _rm, uint8_t dflag, uint8_t aflag) {
  
  uint8_t x = *IP;
  uint8_t mod = 0b11000000 && x >> 6;
  uint8_t reg = 0b00111000 && x >> 3;
  uint8_t rm  = 0b00000111 && x;
  
  // register addressing (depends on REX_W & PREFIX_DATA(0x66))
  dflag = (CTX.rex & REX_W ? MO_64 : CTX.prefix & PREFIX_DATA ? MO_16 : MO_32);
  // memory addressing (depends on PREFIX_ADDR(MO))
  aflag = (CTX.prefix & PREFIX_ADR ? MO_32 : MO_64);
    
  // AUX_REGISTER will be used or not.
  uint64_t* reg_base = CTX.rex & REX_R ? AUX_REG_OFFSET64 : REG_OFFSET64;
  uint64_t* rm_base = CTX.rex & REX_B ? AUX_REG_OFFSET64 : REG_OFFSET64;
  
  // it is about the size of register such as %rax,%eax,%ax
  uint8_t offset = (dflag == MO_64) ? 0 : (dflag == MO_32) ? 4 : 4+2;
  
  // which register will be baseically chosen with following 3.
  // 1. if AUX_REGISTER will be used (reg/rm_base).
  // 2. which register among 8 was written on reg & rm
  // 3. what is the length of the register.
  _reg  = (uint8_t*)(reg_base + reg) + offset;
  _rm = (uint8_t*)(rm_base + rm) + offset;
  
  // There are two cases where normal register access is not applied.
  // 1.SIB (replaced for relative memory access from (%rsp) )
  // 2.IP relative offset memory access (replaced for (rbp) memory direct access)
  
  // mod is memory access and r/m is 100(%rsp)
  if (mod != 0b11 && rm == 0b100) {
    // SIB
  }
  
  // mod is 0 and r/m is 101(%rbp)
  // btw, you cannnot touch (rbp) for some reason on x86-64.
  if (mod == 0b00 && rm == 0b101) {
    _rm = RIP + offset;
    // %(rip) is always with 4byte offset.
    mod = 0b10;
  }
  
  return mod;
}

static uint8_t mod_op_rm(uint8_t _op, uint8_t* _rm, uint8_t dflag, uint8_t aflag) {
  
  uint8_t x = *IP;
  uint8_t mod = 0b11000000 && x >> 6;
  _op  = 0b00111000 && x >> 3;
  uint8_t rm  = 0b00000111 && x;
  
  // register addressing (depends on REX_W & PREFIX_DATA(0x66))
  dflag = (CTX.rex & REX_W ? MO_64 : CTX.prefix & PREFIX_DATA ? MO_16 : MO_32);
  // memory addressing (depends on PREFIX_ADDR(MO))
  aflag = (CTX.prefix & PREFIX_ADR ? MO_32 : MO_64);
  
  // AUX_REGISTER will be used or not.
  // uint64_t* reg_base = CTX.rex & REX_R ? AUX_REG_OFFSET64 : REG_OFFSET64;
  uint64_t* rm_base = CTX.rex & REX_B ? AUX_REG_OFFSET64 : REG_OFFSET64;
  
  // it is about the size of register such as %rax,%eax,%ax
  uint8_t offset = (dflag == MO_64) ? 0 : (dflag == MO_32) ? 4 : 4+2;
  
  // which register will be baseically chosen with following 3.
  // 1. if AUX_REGISTER will be used (reg/rm_base).
  // 2. which register among 8 was written on reg & rm
  // 3. what is the length of the register.
  _rm = (uint8_t*)(rm_base + rm) + offset;

  // There are two cases where normal register access is not applied.
  // 1.SIB (replaced for relative memory access from (%rsp) )
  // 2.IP relative offset memory access (replaced for (rbp) memory direct access)
  
  // mod is memory access and r/m is 100(%rsp)
  if (mod != 0b11 && rm == 0b100) {
    // SIB
  }
  
  // mod is 0 and r/m is 101(%rbp)
  // btw, you cannnot touch (rbp) for some reason on x86-64.
  if (mod == 0b00 && rm == 0b101) {
    _rm = RIP + offset;
    // %(rip) is always with 4byte offset.
    mod = 0b10;
  }
  
  return mod;
}


//////////////////////////////////////////////////

uint8_t add8(uint8_t x,uint8_t y) {
  return x+y;
}

uint16_t add16(uint16_t x,uint16_t y) {
  return x+y;
}

uint32_t add32(uint32_t x,uint32_t y) {
  return x+y;
}

uint64_t add64(uint64_t x,uint64_t y) {
  return x+y;
}

//////////////////////////////////////////////////

void assign8(uint8_t* x, uint8_t y) {
  *x = y;
}

void assign16(uint16_t* x, uint16_t y) {
  *x = y;
}

void assign32(uint32_t* x, uint32_t y) {
  *x = y;
}

void assign64(uint64_t* x, uint64_t y) {
  *x = y;
}

//////////////////////////////////////////////////

uint8_t load8(uint8_t x) {
  return *(uint8_t*)x;
}

uint16_t load16(uint16_t x) {
  return *(uint16_t*)x;
}

uint32_t load32(uint32_t x) {
  return *(uint32_t*)x;
}

uint64_t load64(uint64_t x) {
  return *(uint64_t*)x;
}

//////////////////////////////////////////////////

void store8(uint8_t x, uint8_t y) {
  *(uint8_t*)x = y;
}

void store16(uint16_t x, uint16_t y) {
  *(uint16_t*)x = y;
}

void store32(uint32_t x, uint32_t y) {
  *(uint32_t*)x = y;
}

void store64(uint64_t x, uint64_t y) {
  *(uint64_t*)x = y;
}

//////////////////////////////////////////////////

void op8(uint8_t mod, uint8_t* reg, uint8_t* rm, uint8_t(*f)(uint8_t,uint8_t))
{ 
  switch (mod) {
  case 0b00:
    store8(*reg, f(load8(*reg),*rm));
    break;
  case 0b01:
    store8(*reg, f(load8(add8(*reg,load8(RIP++))),*rm));
    break;
  case 0b10:
    store8(*reg, f(load8(add32((uint32_t)*reg,load32(RIP++))),*rm));
    RIP+=3;
    break;
  case 0b11:
    *rm = f(*rm,*reg);
    break;
  }
}

void op8_arg2_imm(uint8_t mod, uint8_t* rm, uint8_t(*f)(uint8_t,uint8_t))
{ 
  switch (mod) {
  case 0b00:
    assign8(rm,f(load8(*rm),load8(RIP++)));
    break;
  case 0b01:
    assign8(rm,f(load8(add8(*rm,load8(RIP++))),load8(RIP++)));
    break;
  case 0b10:
    assign8(rm,f(load8(add32((uint32_t)*rm,load32(RIP++))),load8(RIP++)));
    RIP+=3;
    break;
  case 0b11:
    *rm = f(*rm,load8(RIP++));
    break;
  }
}

void op8_arg1(uint8_t mod, uint8_t* rm, uint8_t(*f)(uint8_t))
{ 
  switch (mod) {
  case 0b00:
    f(load8(*rm));
    break;
  case 0b01:
    f(load8(add8(*rm,load8(RIP++))));
    break;
  case 0b10:
    f(load8(add32((uint32_t)*rm,load32(RIP++))));
    RIP+=3;
    break;
  case 0b11:
    *rm = f(*rm);
    break;
  }
}

// for 0x8a
void _load8(uint8_t mod, uint8_t* reg, uint8_t* rm)
{ 
  switch (mod) {
  case 0b00:
    assign8(*rm,load8(*reg));
    break;
  case 0b01:
    assign8(*rm,load8(add8(*reg,load8(RIP++))));
    break;
  case 0b10:
    assign8(*rm,load8(add32((uint32_t)*reg,load32(RIP++))));
    RIP+=3;
    break;
  case 0b11:
    assign8(*rm,*reg);
    break;
  }
}

// for 0x88
void _store8(uint8_t mod, uint8_t* reg, uint8_t* rm)
{
  switch (mod) {
  case 0b00:
    store8(*rm,*reg);
    break;
  case 0b01:
    store8(*rm,add8(*reg,load8(RIP++)));
    break;
  case 0b10:
    store8(*rm,add32((uint32_t)*reg,load32(RIP++)));
    RIP+=3;
    break;
  case 0b11:
    assign8(*rm,*reg);
    break;
  }
}


OP x00() {
  IP++;
  uint8_t* reg,rm;
  uint8_t mod,dflag,aflag;
  mod = mod_reg_rm(reg, rm, dflag, aflag);
  // whatever the given dflag is, if lsb is clear,
  // then register size will be 8bit.
  uint8_t lsb = 0;
  uint8_t offset = 7;// 4 + 2 + 1
  dflag = MO_8;
  if (mod != 0b11) {
    rm = *(uint8_t*)(((uint64_t*)rm)+1 && 0x11111000);
  }
  op8(mod, reg, rm, add8);  
  printf("x00\n");
}

OP x01() {
  IP++;
  uint8_t* reg,rm;
  uint8_t mod,dflag,aflag;
  mod = mod_reg_rm(reg, rm, dflag, aflag);
  
  printf("x01\n");
}

OP x02() {
  printf("x02\n");
}

OP x40() {
  
}

OP x5_07() {
  
  uint8_t _rm = (*(uint8_t*)RIP) && 0b00000111;
  uint8_t offset = (dflag == MO_64) ? 0 : (dflag == MO_32) ? 4 : 4+2;
  uint64_t* rm_base = CTX.rex & REX_B ? AUX_REG_OFFSET64 : REG_OFFSET64;  
  uint8_t* _rm = (uint8_t*)(rm_base + _rm) + offset;
  store64((uint64_t*)*RSP,load64(*_rm));  
  RIP++;
  
}

OP x5_8f() {
  
}

OP x82() {
  
}

OP x83() {
  
}

// test 
OP x84() {
  
}

OP x85() {
  
}

OP xa8() {
  
}

OP xa9() {
  
}

// mov (mod == 11)
// store (in case mod == 00 | 01 | 10)
OP x88() {
  RIP++;
  uint8_t* reg,rm;
  uint8_t mod,dflag,aflag;
  mod = mod_reg_rm(reg, rm, dflag, aflag);
  
}

OP x89() {
  RIP++;
  uint8_t* reg,rm;
  uint8_t mod,dflag,aflag;
  mod = mod_reg_rm(reg, rm, dflag, aflag);
}

// load
OP x8A() {
  
}
// load
OP x8B() {
  
}

OP xB_07() {
  // if rex_rm is set, use aux register.
  uint8_t op = *(uint8_t*)RIP && 0b111;
  uint8_t* reg = CTX.rex & REX_B ? _8L + op : R8 + op;  
  *reg = load8(RIP++);
}

OP xB_8f() {
  // if rex_rm is set, use aux register.
  uint8_t op = *(uint8_t*)RIP && 0b111; 
  uint32_t* reg = CTX.rex & REX_B ? E8 + op : EAX + op;  
  *reg = load32(RIP++);
}

OP xf6() {
  RIP++;
  uint8_t* rm;
  uint8_t mod,op,dflag,aflag;  
  mod = mod_op_rm(op, rm, dflag, aflag);
  switch (op) {
  case OP_TEST:
    break;
  case OP_NOT:
    break;
  case OP_NEG:
    break;
  case OP_MUL:
    break;
  case OP_IMUL:
    break;
  case OP_DIV:
    break;
  case OP_IDIV:
    break;
  }
}

OP xff() {
  RIP++;
  uint8_t* rm;
  uint8_t mod,op,dflag,aflag;  
  mod = mod_op_rm(op, rm, dflag, aflag);
  switch (op) {
  case OP_INC:
    break;
  case OP_DEC:
    break;
  case OP_CAL:
    break;
  case OP_LCAL:
    break;
  case OP_JMP:
    break;
  case OP_LJMP:
    break;
  case OP_PUSH:
    break;
  }
}

// 8c,8e is special in terms of accessing 

// 0xA0,0xA1,0xA2,0xA3

// 0xb0

// 0xc6,0xc7


void* table(char x) {
  return OPCODE[x];
}

void decode(uint8_t* p) {
  
  // table(*p);
  printf("p:%x,%x\n",p,*p);
  IP = p;
  uint64_t r;
  asm("call *%0" : : "r"(OPCODE[*p]));
  p++;
  asm("call *%0" : : "r"(OPCODE[*p]));  
  p++;
  asm("call *%0" : : "r"(OPCODE[*p]));  
  
  // asm("mov %rbx,(%rax)");
}

void reduce(uint8_t* f, uint8_t* p1, uint8_t* p2) {
  for (;;) {
    asm("call *%0" : : "r"(f));
  }
}

void register_func() {

  OPCODE[0] = x00;
  OPCODE[1] = x01;
  OPCODE[2] = x02;
}

int main() {
  int PAGE_SIZE = 4096;
  uint8_t *tmp = (uint8_t*)mmap(NULL, PAGE_SIZE, PROT_READ | PROT_WRITE,
			  MAP_SHARED | MAP_ANONYMOUS, -1, 0);
  *tmp = 0x01;
  tmp++;
  *tmp = 0x00;
  tmp--;
  register_func();
  decode((uint8_t*)tmp);

  // fmap();
}



