
#include <stdio.h>
#include <stdint.h>

#include "os.h"
#include "logger.h"
#include "x86_emu.h"
#include "string.h"
#include "alloc.h"

static char* __RM_BUF[0x30] = {};
static char* __REG_BUF[0x30] = {};
static char __DIGIT[0x10] = {'0','x'};
static char* __REGISTER = 0;

static void cons(char* p, char** q) {
  char* s = 0;
  int mem = 0;
  if (*(*q) == '[') mem = 1;
  for (;;q++) {
    if (*q == 0) break;
    for (s = *q;*p,*s;p++,s++) *p = *s;
  }
  if (mem) *p = ']';
}

static char* itoh(uint64_t a, uint16_t digit) {
  char* h = &__DIGIT[0];
  char* h2 = h+2;
  char* r;
  uint16_t shift = digit - 4;
  uint64_t p = 0x0f;
  p <<= shift;
  int i = shift;
  uint8_t b;
  for (;p > 0;i-=4,h2++,p >>= 4) {
    b = (a & p) >> i;
    b += (b < 0x0a) ? 0x30 : 0x61 - 0x0a;
    *h2 = b;
  }
  return h;
}

static char* set_name1(char* q, int flag) {

  switch (flag) {
  case 0x00:
    *(uint8_t*)(q+2) = 'l';
    q++;
    break;
  case 0x04:
    *(uint8_t*)(q+2) = 'h';
    q++;
    break;
  case 0x08:
    *(uint8_t*)(q+2) = 'x';
    q++;
    break;
  case 0x10:
    *(uint8_t*)(q) = 'e';
    *(uint8_t*)(q+2) = 'x';
    break;
  case 0x18:
    *(uint8_t*)(q) = 'r';
    *(uint8_t*)(q+2) = 'x';
    break;
  }
  return q;
}

static char* set_name2(char* q, int flag) {

  switch (flag) {
  case 0x00:
    *(q+3) = 'l';
    q++;
    break;
  case 0x04:
    __os__write(1, "err\n", 4);
    for (;;);
    break;
  case 0x08:
    // put nothing
    q++;
    break;
  case 0x10:
    *q = 'e';
    break;
  case 0x18:
    *q = 'r';
    break;
  }
  return q;
}

static char* set_name3(char* q, int flag) {

  switch (flag) {
  case 0x00:
    *(q+2) = 'b';
    break;
  case 0x04:
    __os__write(1, "err\n", 4);
    for (;;);
    break;
  case 0x08:
    *(q+2) = 'w';
    break;
  case 0x10:
    *(q+2) = 'd';
    break;
  case 0x18:
    break;
  }
  return q;
}

static char* get_register_name(uint64_t* p, uint8_t flag) {

  Reg* reg = _get_reg_head();
  int r = p - (uint64_t*)reg;
  uint8_t* q = &__REGISTER;
  *(uint64_t*)q = 0;
  switch (r) {
  case 0:
    *(uint8_t*)(q+1) = 'a';
    return set_name1(q, flag);
  case 1:
    *(uint8_t*)(q+1) = 'c';
    return set_name1(q, flag);
  case 2:
    *(uint8_t*)(q+1) = 'd';
    return set_name1(q, flag);
  case 3:
    *(uint8_t*)(q+1) = 'b';
    return set_name1(q, flag);
  case 4:
    *(uint8_t*)(q+1) = 's';
    *(uint8_t*)(q+2) = 'p';
    return set_name2(q, flag);
  case 5:
    *(uint8_t*)(q+1) = 'b';
    *(uint8_t*)(q+2) = 'p';
    return set_name2(q, flag);
  case 6:
    *(uint8_t*)(q+1) = 's';
    *(uint8_t*)(q+2) = 'i';
    return set_name2(q, flag);
  case 7:
    *(uint8_t*)(q+1) = 'd';
    *(uint8_t*)(q+2) = 'i';
    return set_name2(q, flag);
  case 8:
    *q = 'r';
    *(q+1) = '8';
    set_name3(q, flag);
    return q;
  case 9:
    *q = 'r';
    *(q+1) = '9';
    set_name3(q, flag);
    return q;
  case 10:
  case 11:
  case 12:
  case 13:
  case 14:
  case 15:
    *q = 'r';
    *(q+1) = '1';
    *(q+2) = r + 0x26;// 0x30 - 0xa
    set_name3(q+1, flag);
    return q;
  default:
    logger_emit_p(r);
    logger_emit("misc.log", "!\n");
    for (;;);
    return 0;
  }
}

static void check_sign(uint64_t imm_val, char** buf, int digit) {

  uint64_t pow = 1 << digit;
  /* logger_emit_p(pow); */
  /* logger_emit_p(1 << 31); */
  /* logger_emit_p(imm_val >= (pow >> 1)); */
  if (imm_val >= (1 << (digit - 1))) {
    buf[0] = "-";
    uint64_t neg = pow - imm_val;// - 1;
    buf[1] = itoh(neg, digit);
  } else {
    buf[0] = "+";
    buf[1] = itoh(imm_val, digit);
  }
}

static void write_scale_index_base(Context* con, char* out, char** buf) {

  char b1[5] = {};
  char** begin = buf;
  for (;*buf;buf++);
  buf[0] = "(";
  if (con->sib_base == 0) {
    buf[1] = " ";
    goto b1;
  }
  char* p = get_register_name(con->sib_base, con->aflag);
  // needs to be copied as next is coming.
  char* q = &b1[0];
  char* q1 = q;
  for (;*p;p++,q++) *q = *p;
  buf[1] = q1;
 b1:
  // if scale == 00, then it means index is neglected.
  // you can omit scale & index.
  if (con->sib_scale == 0) {
    buf[2] = ")";
    cons(out, begin);
    return;
  }
  buf[2] = ",";
  buf[3] = get_register_name(con->sib_index, con->aflag);
  buf[4] = ",";
  if (con->sib_scale == 1)
    buf[5] = "2";
  if (con->sib_scale == 2)
    buf[5] = "4";
  if (con->sib_scale == 3)
    buf[5] = "8";  
  buf[6] = ")";
  cons(out, begin);
}

static void t1(Context* con, uint8_t apply_mod, char* out) {

  uint8_t mod = con->mod;
  uint8_t dflag = con->dflag;
  uint8_t aflag = con->aflag;
  char* buf[0x10] = {};
  char b1[5] = {};
  if (apply_mod) {
    switch (mod) {
    case 0x00:
      // if r/m is set, then r/m does not indicate single register.
      // instead check scale * sib_index and sib_base.
      if (con->sib) {
	write_scale_index_base(con, out, &buf[0]);
	return 0;
      }
      // mod == 00 and r/m == 101(points on rbp)
      // rip + displacement32
      buf[0] = "[";
      if (con->rip_rel) {
	int arch = _get_arch();
	if (arch == 2) {
	  buf[1] = "rip";	  
	  check_sign(con->displacement, &buf[2], 32);
	} else {
	  check_sign(con->displacement, &buf[1], 32);
	}
      } else {
	buf[1] = get_register_name(con->rm, con->aflag);
      }
      cons(out, &buf[0]);
      return 0;
    case 0x08:
      if (con->sib) {
	check_sign(con->displacement, &buf[0], 8);
	write_scale_index_base(con, out, &buf[0]);
	return 0;
      }
      buf[0] = "[";
      buf[1] = get_register_name(con->rm, con->aflag);
      check_sign(con->displacement, &buf[2], 8);
      cons(out, &buf[0]);
      return 0;
    case 0x10:
      if (con->sib) {
	logger_emit_p(con->displacement);
	check_sign(con->displacement, &buf[0], 32);
	write_scale_index_base(con, out, &buf[0]);
	return 0;
      }
      buf[0] = "[";
      buf[1] = get_register_name(con->rm, con->aflag);
      check_sign(con->imm_val, &buf[2], 32);
      cons(out, &buf[0]);
      break;
    case 0x18:
      buf[0] = get_register_name(con->rm, con->dflag);
      cons(out, &buf[0]);
      break;
    }
    return 0;
  } else {
    buf[0] = get_register_name(con->reg, con->dflag);
    logger_emit("misc.log", buf[0]);
    logger_emit_p(con->dflag);
    logger_emit_p(con->reg);
    cons(out, &buf[0]);
    return 0;
  }
}

static char* get_rm_name(Context* con, char* out) {
  t1(con, 1, out);
}

static char* get_reg_name(Context* con, char* out) {
  t1(con, 0, out);
}

static char* _snprintf(char* p, char** q) {

  char* s = 0;
  for (;;q++) {
    if (*q == 0) break;
    for (s = *q;*p,*s;p++,s++) *p = *s;
    *p++ = ' ';
  }
  *p = '\n';
  return p;
}

static void fill_name(Context* con, char** b2) {

  get_rm_name(con, &__RM_BUF[0]);
  get_reg_name(con, &__REG_BUF[0]);
  int idx1 = 1;
  int idx2 = 2;
  if (con->is_rm_src) {
    idx1 = 2;idx2 = 1;
  }
  b2[idx1] = &__RM_BUF[0];
  b2[idx2] = &__REG_BUF[0];
}

static char* get_jmp_name(int index) {
  switch (index & 0xf) {
  case 0x0:
    return "jo";
  case 0x1:
    return "jno";
  case 0x2:
    return "jnae";
  case 0x3:
    return "jnc";
  case 0x4:
    return "je";
  case 0x5:
    return "jne";
  case 0x6:
    return "jna";
  case 0x7:
    return "ja";
  case 0x8:
    return "js";
  case 0x9:
    return "jns";
  case 0xa:
    return "jpe";
  case 0xb:
    return "jpo";
  case 0xc:
    return "jnge";
  case 0xd:
    return "jge";
  case 0xe:
    return "jng";
  case 0xf:
    return "jnle";
  default:
    __os__write(1, "not yet\n", 7);
    break;
  }
  return 0;
}

static char* get_op_shift_name(Context* con) {
  uint64_t* d = _get_op_shift_base();
  size_t diff = (uint64_t*)con->imm_op - d;
  switch (diff) {
  case 0x00:
    return "rol";
  case 0x01:
    return "ror";
  case 0x02:
    return "rcl";
  case 0x03:
    return "rcr";
  case 0x04:
    return "shl";
  case 0x05:
    return "shr";
  case 0x06:
    return "shl1";
  case 0x07:
    return "sar";
  default:
    logger_emit_p(diff);
    logger_emit_p(d);
    __os__write(1, "p\n", 2);
    for (;;);
    break;
  }
  return 0;
}

static char* get_f6_opcode_name(Context* con) {

  logger_emit_p(con->imm_op);
  uint64_t* d = _get_op_f6_f_base();
  logger_emit_p(d);
  size_t diff = (uint64_t*)con->imm_op - d;
  switch (diff) {
  case 0x00:
    return "test";
    break;
  case 0x01:
    for (;;);
    break;
  case 0x02:
    return "not";
  case 0x03:
    return "neg";
  case 0x04:
    return "mul";
  case 0x05:
    return "imul";
  case 0x06:
    return "div";
  case 0x07:
    return "idiv";
  default:
    __os__write(1, "p\n", 2);
    for (;;);
    break;
  }
  return 0;
}

static char* get_fe_opcode_name(Context* con) {
  uint64_t* d = _get_op_fe_f_base();
  size_t diff = (uint64_t*)con->imm_op - d;
  switch (diff) {
  case 0x00:
    return "inc";
  case 0x01:
    return "dec";
  case 0x02:
    return "call";
  case 0x03:
    return "call";
  case 0x04:
    return "jmp";
  case 0x05:
    return "jmp";
  case 0x06:
    return "push";
  case 0x07:
    __os__write(1, "a\n", 2);
    for (;;);
  default:
    __os__write(1, "p\n", 2);
    for (;;);
    break;
  }
  return 0;
}

static char* get_op01_f_base(Context* con) {
  uint64_t* d = _get_op01_f_base();
  size_t diff = (uint64_t*)con->imm_op - d;
  switch (diff) {
  case 0x00:
    return "add";
  case 0x01:
    return "or";
  case 0x02:
    return "adc";
  case 0x03:
    return "sbb";
  case 0x04:
    return "and";
  case 0x05:
    return "sub";
  case 0x06:
    return "xor";
  case 0x07:
    return "cmp";
  default:
    __os__write(1, "p\n", 2);
    for (;;);
    break;
  }
  return 0;
}

static state_clean_internal() {
  // 8 * (0x30 + 0x30 + 0x10)
  int ss = 0x30 + 0x30;// + 0x10;
  uint64_t* p = &__RM_BUF[0];
  uint64_t* pe = p + ss;
  for (;p<pe;p++) {
    *p = 0;
  }
  p = &__DIGIT[0] + 2;
  *p = 0;
}

static void write_addr() {
  char* a1 = __z__emu__x86__get_image_base();
  char* a2 = __z__emu__x86__get_emu_base();
  char* rip = _get_rip();  
  size_t addr = (char*)rip + ((char*)a1 - (char*)a2);  
  uint64_t* p = itoh(addr, 32);
  __os__write(1, p, strlen(p));
  __os__write(1, "\t", 1);
  logger_emit("instruction.log", p);
  logger_emit("instruction.log", "\t");
  // write_addr();
  p = &__DIGIT[0] + 2;
  *p = 0;
}

static char* get_xmm_op_name(uint64_t* reg, uint8_t t) {

  uint64_t* d = _get_xmm_base();
  int diff = (reg - d)/2;
  logger_emit_p(diff);
  static char xmm[5] = "xmm";
  if (t) {
    // no string merge required
    static char xmm2[5] = "xmm";    
    xmm2[3] = diff + 0x30;
    return &xmm2;
  } else {
    xmm[3] = diff + 0x30;
  }
  return &xmm;
}

static int check_fp(Context* con) {

  if (con->data_prefix) {
    return 1;
  }
  if (con->repz) {
    return 2;
  }
  if (con->repnz) {
    return 3;    
  }
  return 0;
}

static char* get_0xf90_name(uint8_t x) {
  static char r[6] = "set";
  r[4] = 0;r[5] = 0;r[6] = 0;
  char* r4 = (char*)&r[0]+3;
  switch (x) {
  case 0x90:
    *r4 = 'o';
    break;
  case 0x91:
    *r4 = 'n';
    *(r4+1) = 'o';
  case 0x92:
    *r4 = 'b';
    break;
  case 0x93:
    *r4 = 'n';
    *(r4+1) = 'b';
    break;
  case 0x94:
    *r4 = 'e';
    break;
  case 0x95:
    *r4 = 'n';
    *(r4+1) = 'e';
    break;
  default:
    break;
  }
  return &r[0];
}

static char* fpu_tmp(size_t* x) {
  Reg* reg = _get_reg_head();
  size_t diff = x - (size_t*)reg;
  logger_emit_p(reg);
  logger_emit_p(x);
  switch (diff) {
  case 0:
    return "fxsave";
  case 1:
    return "fxrstor";
  case 2:
    return "ldmxcsr";
  case 3:
    return "stmxcsr";
  }
  for (;;);
}

static char* get_fpu_d8ace_op_name(Context* con) {

  Reg* reg = _get_reg_head();
  size_t diff = (size_t*)con->reg - (size_t*)reg;
  switch (diff) {
  case 0:
    return "fadd";
  case 1:
    return "fmul";
  case 2:
    return "fcom";
  case 3:
    return "fcomp";
  case 4:
    return "fsub";
  case 5:
    return "fsubr";
  case 6:
    return "fdiv";
  case 7:
    return "fdivr";
  default:
    for (;;);
    return "";
  }
}

static char* get_fpu_d9_op_name(Context* con) {
  // basically should be determined by reg.
  // additionally, mod
  Reg* reg = _get_reg_head();
  size_t diff = (size_t*)con->reg - (size_t*)reg;
  switch (diff) {
  case 0:
    return "fld";
  case 1:
    return "fxch";
  case 2: {
    // fnop
    return "fst";
  }
  case 3:
    return "fstp";
  case 4:
    // mod = e0 => fchs
    return "fldenv";
  case 5:
    return "fldcw";
  case 6:
  case 7:
    return "aa";
  }
  for (;;);
}

// sensitive operation
// verr/verw
// 

static void fetch_extend(Context* con) {

  char* buf[20] = {};
  char* b;
  char** b2[4] = {};
  int idx1 = 1;
  int idx2 = 2;
  switch (con->opcode) {
  case 0x00:
    // sldt (local discriptor table)
    // str (store task register)
    // lldt 
    // ltr 
    // verr verify a segment for reading
    // verw verify a segment for writing
    for (;;);
    break;
  case 0x05:
    b2[0] = "syscall";
    break;
  
  case 0x10:
  case 0x11:
    switch (check_fp(con)) {
    case 0:
      b2[0] = "movups";
      break;
    case 1:
      b2[0] = "movupd";
      break;
    case 2:
      b2[0] = "movsd";
      break;
    case 3:
      b2[0] = "movss";
      break;
    }
    if (con->opcode == 0x11) {
      idx1 = 2;idx2 = 1;
    }
    b2[idx1] = get_xmm_op_name(con->reg, 0);
    get_rm_name(con, &__RM_BUF[0]);
    b2[idx2] = &__RM_BUF[0];
    break;
  case 0x14:
    // low packed 
  case 0x15:
    // high packed
    // if 66(data-prefix) is given treat reg as m128
    b2[0] = "unpck";
    b2[1] = get_xmm_op_name(con->reg, 0);
    b2[2] = get_xmm_op_name(con->rm, 1);    
    break;
    
  case 0x1f:
    b2[0] = "nop";
    break;
  case 0x28:
  case 0x29:
    if (con->data_prefix) {
      b2[0] = "movapd";
    } else {
      b2[0] = "movaps";
    }
    if (con->opcode == 0x11) {
      idx1 = 2;idx2 = 1;
    }
    b2[idx1] = get_xmm_op_name(con->reg, 0);
    get_rm_name(con, &__RM_BUF[0]);
    b2[idx2] = &__RM_BUF[0];
    break;
  case 0x2e:
    b2[0] = "ucomis";
    goto xmm2;
  case 0x2f:
    b2[0] = "comis";
    goto xmm2;
    // 
  case 0x40:
    b2[0] = "cmov";
    goto cmov01;
  case 0x41:
    b2[0] = "cmov";
    goto cmov01;
  case 0x42:
    b2[0] = "cmov";
    goto cmov01;
  case 0x43:
    b2[0] = "cmov";
    goto cmov01;
  case 0x44:
    b2[0] = "cmov";
    goto cmov01;
  case 0x45:
    b2[0] = "cmov";
    goto cmov01;
  case 0x46:
    b2[0] = "cmov";
    goto cmov01;
  case 0x47:
    b2[0] = "cmov";
    goto cmov01;
  case 0x48:
    b2[0] = "cmovs";
    goto cmov01;
  case 0x49:
    b2[0] = "cmov";
    goto cmov01;
  case 0x4a:
    b2[0] = "cmov";
    goto cmov01;
  case 0x4b:
    b2[0] = "cmov";
    goto cmov01;
  case 0x4c:
    b2[0] = "cmov";
    goto cmov01;
  case 0x4d:
    b2[0] = "cmov";
    goto cmov01;
  case 0x4e:
    b2[0] = "cmov";
    goto cmov01;
  case 0x4f:
    b2[0] = "cmov";
    goto cmov01;
  cmov01:
    fill_name(con, &b2);
    break;
  case 0x50:
    b2[0] = "movmskp";
    goto xmm2;
  case 0x51:
    b2[0] = "sqrt";
    goto xmm2;
  case 0x52:
    b2[0] = "rsqrt";
    goto xmm2;
  case 0x53:
    b2[0] = "rcp";
    goto xmm2;
  case 0x54:
    b2[0] = "rcp";
    goto xmm2;
  case 0x55:
    b2[0] = "andnp";
    goto xmm2;
  case 0x56:
    b2[0] = "orp";
    goto xmm2;
  case 0x57:
    b2[0] = "xorp";
    goto xmm2;
  case 0x58:
    b2[0] = "add";
    goto xmm2;
  case 0x59:
    b2[0] = "mul";
    goto xmm2;
  case 0x5a:
  case 0x5b:
    b2[0] = "cvtps2pd";
    goto xmm2;
  case 0x5c:
    b2[0] = "sub";
    goto xmm2;
  case 0x5d:
    b2[0] = "min";
    goto xmm2;
  case 0x5e:
    b2[0] = "div";
    goto xmm2;
  case 0x5f:
    b2[0] = "max";
    goto xmm2;

  xmm2:
    b2[1] = get_xmm_op_name(con->reg, 0);
    b2[2] = get_xmm_op_name(con->rm, 1);
    break;

  case 0x80:
  case 0x81:
  case 0x82:
  case 0x83:
  case 0x84:
  case 0x85:
  case 0x86:
  case 0x87:
  case 0x88:
  case 0x89:
  case 0x8a:
  case 0x8b:
  case 0x8c:
  case 0x8d:
  case 0x8e:
  case 0x8f:
    b2[0] = get_jmp_name(con->opcode);
    b2[1] = itoh(con->imm_val, 32);
    // prepare_control_data(reg->pre_rip, reg->pre_rip + con->imm_val, 0, 0);
    break;
  case 0x90:
  case 0x91:
  case 0x92:
  case 0x93:
  case 0x94:
  case 0x95:
  case 0x96:
  case 0x97:
  case 0x98:
  case 0x99:
  case 0x9a:
  case 0x9b:
  case 0x9c:
  case 0x9d:
  case 0x9e:
  case 0x9f:
    b2[0] = get_0xf90_name(con->opcode);
    get_rm_name(con, &__RM_BUF[0]);
    b2[1] = &__RM_BUF[0];
    break;
  case 0xa2:
    b2[0] = "cpuid";
    break;
  case 0xae:
    b2[0] = fpu_tmp(con->reg);
    get_rm_name(con, &__RM_BUF[0]);
    b2[1] = &__RM_BUF[0];    
    break;
  case 0xb0:
  case 0xb1:
    if (con->lock) {
      b2[0] = "lock";
      b2[1] = "cmpxchg";
    } else {
      b2[0] = "cmpxchg";
    }
    fill_name(con, &b2);
    break;
  case 0xb6:
    b2[0] = "movzbl";
    fill_name(con, &b2);    
    break;
  case 0xb7:
    b2[0] = "movzwl";
    fill_name(con, &b2);
    break;
  case 0xbe:
    b2[0] = "movsbl";
    fill_name(con, &b2);
    break;
  default:
    for (;;);
    break;
  }
  if (b2[0]) {
    b = _snprintf(buf, &b2);
    __os__write(1, buf, __z__std__strlen(buf));
    state_clean_internal();
  }
  write_addr();
}

static uint16_t mtohex(char* x) {

  uint16_t r;
  uint8_t first = 1;
  uint8_t h1 = (0xf0 & *x) >> 4; 
 b1:
  if (0 <= h1 && h1 <= 9) {
    h1 += 0x30;
  } else if (10 <= h1 && h1 <= 15) {
    h1 += 0x61 - 10;
  } else {
    for (;;);
  }
  if (first) {
    r = h1;
    first = 0;
    h1 = 0x0f & *x;
    goto b1;
  } else {
    r |= h1 << 8;
  }
  return r;
  // return &r[0];
}

typedef struct {
  void* addr1;
  void* addr2;
  uint64_t flag;
  void* next;
  // caller / callee / call / jmp;  
} control_data;

static control_data first_control_data = {};

static void insert_control_data(control_data* p) {
  
  // control_data;
  control_data* c = &first_control_data;
  for (;c->next;c=c->next) {
    if (c->next == 0) {
      if (p->addr1 < c->addr1) {	
	p->next = c;
      }
    }
  }
  c->next = p;
}

static control_data* alloc_control_data(void* addr1, void* addr2, int caller_callee, int call_jmp) {
  control_data* c = __malloc(sizeof(control_data));
  c->addr1 = addr1;
  c->addr2 = addr2;
  c->flag = 0;
  return c;
}

static void prepare_control_data(void* addr1, void* addr2, int caller_callee, int call_jmp) {

  control_data* c = alloc_control_data(addr1, addr2, 0, 0);
  insert_control_data(c);
  c = alloc_control_data(addr2, addr1, 0, 0);
  insert_control_data(c);
}

void __z__x86__dis() {

  Context* con = _get_context();
  Reg* reg = _get_reg_head();
  logger_emit("misc.log", "----------\n");
  char* p = reg->pre_rip;
  uint16_t v = 0;
  for (;p<reg->rip;p++) {
    logger_emit_p(*p);
    v = mtohex(p);
    __os__write(1, &v, 2);
    // logger_emit_i();
    int i = __z__logger__get_handle("instruction.log");
    __os__write(i, &v, 2);
    // logger_emit("instruction.log", &v);
  }
  uint8_t diff = (char*)reg->rip - (char*)reg->pre_rip;
  __os__write(1, "\t", 1);
  logger_emit("instruction.log", "\t");
  if (diff < 0x8) {
    __os__write(1, "\t", 1);
    // logger_emit("instruction.log", "\t");
  }
  if (diff < 0x4) {
    __os__write(1, "\t", 1);
    // logger_emit("instruction.log", "\t");
  }
  
  /* logger_emit_p(reg->pre_rip); */
  /* logger_emit_p(reg->rip); */
  logger_emit("misc.log", "----------\n");
  
  if (_is_extend_op()) {
    logger_emit("misc.log", "----------\n");
    logger_emit_p(con->opcode);
    fetch_extend(con);
    return 0;
  }
  char* buf[20] = {};
  char* b;
  char** b2[4] = {};
  int len = 0;
  int arch = _get_arch();
  int size = 0x10;
  switch (con->opcode) {
  case 0x00:
  case 0x01:
  case 0x02:
  case 0x03:
    fill_name(con, &b2);
    b2[0] = "add";
    break;
  case 0x04:
  case 0x05:
    b2[0] = "add";
    get_reg_name(con, &__REG_BUF[0]);
    b2[1] = &__REG_BUF[0];
    if (con->opcode == 0x5) size = 32;
    b2[2] = itoh(con->imm_val, size);
    break;
  case 0x08:
  case 0x09:
  case 0x0a:
  case 0x0b:
    fill_name(con, &b2);
    b2[0] = "or";
    break;
  case 0x0c:
    size = 8;
  case 0x0d:
    b2[0] = "or";
    get_reg_name(con, &__REG_BUF[0]);
    b2[1] = &__REG_BUF[0];
    if (con->opcode == 0xd) size = 32;
    b2[2] = itoh(con->imm_val, size);
    break;
  case 0x10:
  case 0x11:
  case 0x12:
  case 0x13:
    fill_name(con, &b2);
    b2[0] = "adc";
    break;
  case 0x14:
    size = 8;
  case 0x15:
    b2[0] = "or";
    get_reg_name(con, &__REG_BUF[0]);
    b2[1] = &__REG_BUF[0];
    if (con->opcode == 0x15) size = 32;
    b2[2] = itoh(con->imm_val, size);
    break;
  case 0x18:
  case 0x19:
  case 0x1a:
  case 0x1b:
    fill_name(con, &b2);
    b2[0] = "sbb";
    break;
  case 0x1c:
    size = 8;
  case 0x1d:
    b2[0] = "sbb";
    get_reg_name(con, &__REG_BUF[0]);
    b2[1] = &__REG_BUF[0];
    if (con->opcode == 0x25) size = 32;
    b2[2] = itoh(con->imm_val, size);
    break;
  case 0x20:
  case 0x21:
  case 0x22:
  case 0x23:
    fill_name(con, &b2);
    b2[0] = "and";
    break;
  case 0x24:
    size = 8;
  case 0x25:
    b2[0] = "and";
    get_reg_name(con, &__REG_BUF[0]);
    b2[1] = &__REG_BUF[0];
    if (con->opcode == 0x25) size = 32;
    b2[2] = itoh(con->imm_val, size);
    break;
  case 0x28:
  case 0x29:
  case 0x2a:
  case 0x2b:
    fill_name(con, &b2);
    b2[0] = "sub";
    break;
  case 0x2c:
  case 0x2d:
    b2[0] = "sub";
    get_rm_name(con, &__RM_BUF[0]);
    b2[1] = &__RM_BUF[0];
    b2[2] = itoh(con->imm_val, 32);
    break;
  case 0x2e:
    __os__write(1, "k\n", 2);
    b2[0] = "nop";
    break;
  case 0x30:
  case 0x31:
  case 0x32:
  case 0x33:
  case 0x34:
  case 0x35:
    fill_name(con, &b2);
    b2[0] = "xor";
    break;
  case 0x38:
  case 0x39:
  case 0x3a:
  case 0x3b:
    fill_name(con, &b2);
    b2[0] = "cmp";
    break;
  case 0x3c:
  case 0x3d:
    b2[0] = "cmp";
    get_rm_name(con, &__RM_BUF[0]);
    b2[1] = &__RM_BUF[0];
    b2[2] = itoh(con->imm_val, 32);
    break;
  case 0x50:
  case 0x51:
  case 0x52:
  case 0x53:
  case 0x54:
  case 0x55:
  case 0x56:
  case 0x57:
    b2[0] = "push";
    if (arch == 2) size = 0x18;
    b2[1] = get_register_name(con->reg, size);
    break;
  case 0x58:
  case 0x59:
  case 0x5a:
  case 0x5b:
  case 0x5c:
  case 0x5d:
  case 0x5e:
  case 0x5f:
    b2[0] = "pop";
    if (arch == 2) size = 0x18;
    b2[1] = get_register_name(con->reg, size);
    break;
  case 0x60:
    break;
  case 0x63:
    b2[0] = "movslq";
    fill_name(con, &b2);    
    break;
  case 0x66:   
    break;
  case 0x68:
    b2[0] = "push";
    b2[1] = itoh(con->imm_val, 32);
    // logger_emit_p(con->imm_val);
    break;
  case 0x6a:
    b2[0] = "push";
    b2[1] = itoh(con->imm_val, 8);
    logger_emit_p(con->imm_val);
    break;
  case 0x70:
  case 0x71:
  case 0x72:
  case 0x73:
  case 0x74:
  case 0x75:
  case 0x76:
  case 0x77:
  case 0x78:
  case 0x79:
  case 0x7a:
  case 0x7b:
  case 0x7c:
  case 0x7d:
  case 0x7e:
  case 0x7f:
    b2[0] = get_jmp_name(con->opcode);
    b2[1] = itoh(con->imm_val, 8);
    prepare_control_data(reg->pre_rip, reg->pre_rip + con->imm_val, 0, 0);

    break;
  case 0x80:
  case 0x81:
  case 0x82:
  case 0x83:
    b2[0] = get_op01_f_base(con);
    get_rm_name(con, &__RM_BUF[0]);
    b2[1] = &__RM_BUF[0];
    b2[2] = itoh(con->imm_val, 8);    
    break;
  case 0x84:
  case 0x85:
    b2[0] = "test";
    fill_name(con, &b2);
    break;
  case 0x86:
  case 0x87:
    b2[0] = "xchg";
    fill_name(con, &b2);
    break;
  case 0x88:
  case 0x89:
    b2[0] = "mov";
    fill_name(con, &b2);
    // for (;;);
    break;
  case 0x8a:
  case 0x8b:
    b2[0] = "mov";
    fill_name(con, &b2);
    break;
  case 0x8c:
    break;
  case 0x8d:
    b2[0] = "lea";
    fill_name(con, &b2);
    /* get_rm_name(con, &__RM_BUF[0]); */
    /* b2[1] = &__RM_BUF[0];     */
    /* b2[2] = itoh(con->imm_val, 32); */
    break;
  case 0x8e:
    break;
  case 0x8f:
    break;

  case 0x90:
  case 0x91:
  case 0x92:
  case 0x93:
  case 0x94:
  case 0x95:
  case 0x96:
  case 0x97:
    b2[0] = "xchg";
    fill_name(con, &b2);
    break;
  case 0x98:
  case 0x99:
    b2[0] = "cltq";
    break;
  case 0x9c:
    b2[0] = "pushf";
    break;    
  case 0x9d:
    b2[0] = "popf";
    break;
  case 0xa0:
  case 0xa1:
    b2[0] = "mov";
    get_rm_name(con, &__RM_BUF[0]);
    b2[2] = &__RM_BUF[0];
    b2[1] = itoh(con->imm_val, 32);    
    break;
  case 0xa2:
  case 0xa3:
    b2[0] = "mov";
    get_rm_name(con, &__RM_BUF[0]);
    b2[1] = &__RM_BUF[0];
    b2[2] = itoh(con->imm_val, 32);    
    break;
  case 0xa5:
  case 0xa6:
    b2[0] = "cmpsb";
    break;
  case 0xa8:
    break;
  case 0xa9:
    b2[0] = "test";
    logger_emit_p(con->imm_val);
    logger_emit_p(con->dflag);
    logger_emit_p(con->rm);
    get_rm_name(con, &__RM_BUF[0]);
    b2[1] = &__RM_BUF[0];
    b2[2] = itoh(con->imm_val, 32);
    break;
  case 0xaa:
    b2[0] = "stosS";
    break;
  case 0xab:
    if (con->repz) {
      b2[0] = "rep";
    }
    b2[1] = "stosS";
    // get_rm_name(con, &__RM_BUF[0]);
    break;
  case 0xac:
    b2[0] = "lodsS";
    break;
  case 0xad:
    b2[0] = "lodsS";
    break;
  
  case 0xb0:
  case 0xb1:
  case 0xb2:
  case 0xb3:
  case 0xb4:
  case 0xb5:
  case 0xb6:
  case 0xb7:
  case 0xb8:
  case 0xb9:
  case 0xba:
  case 0xbb:
  case 0xbc:
  case 0xbd:
  case 0xbe:
  case 0xbf:
    b2[0] = "mov";
    get_reg_name(con, &__REG_BUF[0]);
    b2[1] = &__REG_BUF[0];
    len = (con->opcode & 0b00001000) ? 32 : 8;
    b2[2] = itoh(con->imm_val, len);
    break;
  case 0xc0:
  case 0xc1:
    b2[0] = get_op_shift_name(con);
    get_rm_name(con, &__RM_BUF[0]);
    b2[1] = &__RM_BUF[0];
    b2[2] = itoh(con->imm_val, 8);
    logger_emit_p(con->imm_val);
    // __os__write(1, "c\n", 2);
    // for (;;);
    break;
  case 0xc2:
    b2[0] = "ret";
    b2[1] = itoh(con->imm_val, 16);    
    /* get_rm_name(con, &__RM_BUF[0]); */
    /* b2[1] = &__RM_BUF[0];     */
    break;
  case 0xc3:
    b2[0] = "ret";
    break;
  case 0xc4:
    break;
  case 0xc5:
    break;
  case 0xc6:
    b2[0] = "mov";
    get_rm_name(con, &__RM_BUF[0]);
    b2[1] = &__RM_BUF[0];
    b2[2] = itoh(con->imm_val, 8);
    break;
  case 0xc7:
    b2[0] = "mov";
    get_rm_name(con, &__RM_BUF[0]);
    b2[1] = &__RM_BUF[0];
    b2[2] = itoh(con->imm_val, 32);
    break;
  case 0xc9:
    b2[0] = "leave";
    break;
  case 0xd0:
  case 0xd1:
    b2[0] = get_op_shift_name(con);
    get_rm_name(con, &__RM_BUF[0]);
    b2[1] = &__RM_BUF[0];
    // this actually should be alaways 1
    b2[2] = itoh(con->imm_val, 8);
    break;
  case 0xd8:
    // float32
  case 0xda:
    // int32
  case 0xdc:
    // double(64)
  case 0xde:
    // int16
    b2[0] = get_fpu_d8ace_op_name(con);
    get_rm_name(con, &__RM_BUF[0]);
    b2[1] = &__RM_BUF[0];
    break;
  case 0xd9:
    // float
  case 0xdd:
    // double
    b2[0] = get_fpu_d9_op_name(con);
    get_rm_name(con, &__RM_BUF[0]);
    b2[1] = &__RM_BUF[0];
    break;
  case 0xdb: {
    char* r = reg->rip;
    if (*(r-1) | 0xe3 == 0xe3) {
      b2[0] = "finit";
    }
    break;
  }
  case 0xe8:
    b2[0] = "call";
    b2[1] = itoh(con->imm_val, 32);
    prepare_control_data(reg->pre_rip, reg->pre_rip + con->imm_val, 0, 0);
    break;
  case 0xe9:
    b2[0] = "jmp";
    b2[1] = itoh(con->imm_val, 32);
    prepare_control_data(reg->pre_rip, reg->pre_rip + con->imm_val, 0, 0);
    break;
  case 0xea:
  case 0xeb:
    b2[0] = "jmp";
    b2[1] = itoh(con->imm_val, 8);
    prepare_control_data(reg->pre_rip, reg->pre_rip + con->imm_val, 0, 0);
    break;
  case 0xf6:
    b2[0] = get_f6_opcode_name(con);
    get_rm_name(con, &__RM_BUF[0]);
    b2[1] = &__RM_BUF[0];
    // for (;;);
    break;
  case 0xf7:
    logger_emit_p(con->rm);
    logger_emit_p(con->mod);
    b2[0] = get_f6_opcode_name(con);
    get_rm_name(con, &__RM_BUF[0]);
    b2[1] = &__RM_BUF[0];
    break;
  case 0xfe:
  case 0xff:
    b2[0] = get_fe_opcode_name(con);
    get_rm_name(con, &__RM_BUF[0]);
    b2[1] = &__RM_BUF[0];
    break;
  default:
    logger_emit_p(con->opcode);
    for (;;);
    break;
  }
  if (b2[0]) {
    b = _snprintf(buf, &b2);
    __os__write(1, buf, __z__std__strlen(buf));
    logger_emit("instruction.log", buf);
    
    // logger_emit("instruction.log", "\t");  
    state_clean_internal();
  }
  write_addr();
}


