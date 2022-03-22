
#include <stdio.h>
#include <stdint.h>

typedef struct {
  uint64_t* low;
  uint64_t* high;
} t_xmm;

typedef struct {

  uint64_t* rax;
  uint64_t* rcx;
  uint64_t* rdx;
  uint64_t* rbx;

  uint64_t* rsp;
  uint64_t* rbp;
  uint64_t* rsi;
  uint64_t* rdi;

  uint64_t* r8;
  uint64_t* r9;
  uint64_t* r10;
  uint64_t* r11;
  uint64_t* r12;
  uint64_t* r13;
  uint64_t* r14;
  uint64_t* r15;

  uint64_t* eflags;
  uint64_t* rip;
  uint64_t* pre_rip;
  t_xmm xmm[16];
} Reg;

typedef struct __attribute__((packed)) {
  
  uint64_t** opcode_table;
  uint64_t gen_code_base;
  // uint64_t load;
  uint64_t displacement;
  uint64_t arg1;
  uint64_t arg2;
  uint64_t res;
  uint64_t imm_op;
  uint64_t internal_arg1;
  uint64_t internal_arg2;
  uint64_t override_reg;
  uint64_t* reg;
  uint64_t* rm;
  // can be 32?
  uint64_t imm_val;
  uint64_t image_base;
  uint64_t emu_base;
  uint64_t sib_index;
  uint64_t sib_base;
  uint64_t sib_displacement;
  
  uint8_t opcode;
  uint8_t rex;
  uint8_t data_prefix;
  uint8_t addr_prefix;
  uint8_t repz;
  uint8_t repnz;
  uint8_t lock;
  uint8_t vex;
  uint8_t override;
  uint8_t mod;
  uint8_t mod_type;
  uint8_t sib;
  uint8_t sib_scale;  
  uint8_t rip_rel;
  uint8_t dflag;
  uint8_t aflag;
  uint8_t is_rm_src;
} Context;

void __z__emu__x86__set_image_base(void*);
void* _get_context();
void* _get_reg_head();
void* _get_op_f6_f_base();
void* _get_op_fe_f_base();
void* _get_op_shift_base();
void* _get_op01_f_base();
void* _get_xmm_base();
int _get_objformat();
void _set_objformat(int);
int _get_arch();
void _set_arch(int);

void* _get_rip();
void* _set_pre_rip(void*);
void* _get_cpu_id(int, void*);
void _clear_state();
int _is_extend_op();

void* __z__emu__x86__get_image_base();
void* __z__emu__x86__get_emu_base();
