
#include <stdio.h>
#include <stdint.h>

#include "os.h"
#include "logger.h"
#include "x86_emu.h"
#include "string.h"

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
    logger_emit("misc.log", "!\n");
    for (;;);
    return 0;
  }
}

static void check_sign(uint64_t imm_val, char** buf, int digit) {

  uint64_t pow = 1 << digit;
  if (imm_val >= (pow >> 1)) {
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
	buf[1] = "rip";
	check_sign(con->displacement, &buf[2], 8);
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
  b2[1] = &__RM_BUF[0];
  b2[2] = &__REG_BUF[0];
}

static char* get_jmp_name(int index) {
  switch (index) {
  case 0x70:
    return "jo";
  case 0x71:
    return "jno";
  case 0x72:
    return "jnae";
  case 0x73:
    return "jnc";
  case 0x74:
    return "je";
  case 0x75:
    return "jne";
  case 0x76:
    return "jna";
  case 0x77:
    return "ja";
  case 0x78:
    return "js";
  case 0x79:
    return "jns";
  case 0x7a:
    return "jpe";
  case 0x7b:
    return "jpo";
  case 0x7c:
    return "jnge";
  case 0x7d:
    return "jge";
  case 0x7e:
    return "jng";
  case 0x7f:
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
  p = &__DIGIT[0] + 2;
  *p = 0;
}

static void fetch_extend(Context* con) {

  char* buf[20] = {};
  char* b;
  char** b2[4] = {};
  switch (con->opcode) {
  case 0x00:
    break;
  case 0x1f:
    b2[0] = "nop";
    break;    
  default:
    break;    
  }
  if (b2[0]) {
    b = _snprintf(buf, &b2);
    __os__write(1, buf, __z__std__strlen(buf));
    state_clean_internal();
  }
  write_addr();
}

void __z__x86__dis() {

  Context* con = _get_context();
  Reg* reg = _get_reg_head();

  logger_emit_p(&con->opcode);
  logger_emit("misc.log", "----------\n");  
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
  switch (con->opcode) {
  case 0x00:
  case 0x01:
  case 0x02:
  case 0x03:
  case 0x04:
  case 0x05:
    fill_name(con, &b2);
    b2[0] = "add";
    break;
  case 0x08:
  case 0x09:
  case 0x0a:
  case 0x0b:
  case 0x0c:
  case 0x0d:
    fill_name(con, &b2);
    b2[0] = "or";
    break;
  case 0x10:
  case 0x11:
  case 0x12:
  case 0x13:
  case 0x14:
  case 0x15:
    fill_name(con, &b2);
    b2[0] = "adc";
    break;
  case 0x18:
  case 0x19:
  case 0x1a:
  case 0x1b:
  case 0x1c:
  case 0x1d:
    fill_name(con, &b2);
    b2[0] = "sbb";
    break;
  case 0x20:
  case 0x21:
  case 0x22:
  case 0x23:
  case 0x24:
  case 0x25:
    fill_name(con, &b2);
    b2[0] = "and";
    break;
  case 0x28:
  case 0x29:
  case 0x2a:
  case 0x2b:
  case 0x2c:
  case 0x2d:
    fill_name(con, &b2);
    b2[0] = "sub";
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
  case 0x3c:
  case 0x3d:
    fill_name(con, &b2);
    b2[0] = "cmp";
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
    b2[1] = get_register_name(con->reg, 0x18);
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
    b2[1] = get_register_name(con->reg, 0x18);
    break;
  case 0x60:
    
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
    break;
  case 0x9c:
    b2[0] = "pushf";
    break;    
  case 0x9d:
    b2[0] = "popf";
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
    b2[0] = "stosS";
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
    get_rm_name(con, &__RM_BUF[0]);
    b2[1] = &__RM_BUF[0];    
    break;
  case 0xc3:
    b2[0] = "ret";
    break;
  case 0xc4:
    break;
  case 0xc5:
    break;
  case 0xc6:
  case 0xc7:
    b2[0] = "mov";
    fill_name(con, &b2);
    /* for (;;); */
    /* get_rm_name(con, &__RM_BUF[0]); */
    /* b2[1] = &__RM_BUF[0]; */
    /* if (con->dflag == 0) len = 8; */
    /* else if (con->dflag == 0x8) len = 16; */
    /* else if (con->dflag == 0x10) len = 32; */
    /* else if (con->dflag == 0x18) len = 32; */
    /* b2[2] = itoh(con->imm_val, len); */
    break;
  case 0xd0:
  case 0xd1:
    b2[0] = get_op_shift_name(con);
    get_rm_name(con, &__RM_BUF[0]);
    b2[1] = &__RM_BUF[0];
    // this actually should be alaways 1
    b2[2] = itoh(con->imm_val, 8);
    break;
  case 0xe8:
    b2[0] = "call";
    b2[1] = itoh(con->imm_val, 32);
    break;
  case 0xe9:
    b2[0] = "jmp";
    b2[1] = itoh(con->imm_val, 32);
    break;
  case 0xea:
  case 0xeb:
    b2[0] = "jmp";
    b2[1] = itoh(con->imm_val, 8);
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
    state_clean_internal();
  }
  write_addr();
}


