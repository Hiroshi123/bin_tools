

	default rel
	
	section .data

	global _reg_names

	global _tls1

	global _reg_head
	global _rax
	global _rcx
	global _rdx
	global _rbx
	global _rsp
	global _rbp	
	global _rsi
	global _rdi
	global _r8
	global _r9
	global _r10
	global _r11
	global _r12
	global _r13
	global _r14
	global _r15
	global _eflags	
	global _rip
	global _pre_rip

	global _cs
	global _ds
	global _es
	global _fs
	global _gs
	global _ss

	global _cr0
	
	global __rax
	global __rdx
	global __rcx
	global __rbx
	global __rsp
	global __rbp	
	global __rsi
	global __rdi
	global __r8
	global __r9
	global __r10
	global __r11
	global __r12
	global __r13
	global __r14
	global __r15
	global __eflags	
	global __rip

	global _opcode_table
	global _extend_opcode_table
	
	global _debug
	global _debug._inst
	global _debug._offset

	global _processor
	global _architecture
	global _objformat
	global _meta_page_head
	global _meta_page_ptr

	global _out_page_head
	global _out_page_ptr
	global _draw_memory_table_page_head
	global _draw_memory_table_page_ptr

	global _current_fname
	global _fd_num
	global _current_page_tail
	
	global _context
	global _context._opcode
	global _context._opcode_table
	global _context._override
	global _context._override_reg
	global _context._imm_val
	global _context._displacement
	global _context._image_base
	global _context._emu_base
	global _context._is_rm_src
	
	global _context._rex	
	global _context._mod
	global _context._mod_type
	global _context._reg
	global _context._rm
	global _context._sib
	global _context._sib_scale
	global _context._sib_index
	global _context._sib_base
	global _context._sib_displacement
	global _context._dflag
	global _context._aflag	
	global _context._data_prefix
	global _context._addr_prefix
	global _context._repz
	global _context._repnz
	global _context._lock
	global _context._vex
	
	global _context._arg1
	global _context._arg2
	global _context._res
	global _context._imm_op
	global _context._internal_arg1
	global _context._internal_arg2
	global _context._rip_rel
	global _context._extend_opcode

	global _dflag_len

	;; immidiate operation
	global _op01_f_base
	global _op_f6_f_base
	global _op_fe_f_base	
	global _op_shift_base
	
	global _op_shl_base
	
	global _mod_load_base
	global _mod_fetch_base
	global _mod_store_base

	global _reg_size8

	global _load_base
	global _store_base
	global _fetch_base
	global _add_base
	global _sub_base
	global _assign_base
	global _and_base
	global _or_base
	global _xor_base

	global _seg_base
	global _xmm_base
	global _xmm_ex_base
	
align 0x10
_reg_head:
_rax:	dq 0
_rcx:	dq 0
_rdx:	dq 0
_rbx:	dq 0

_rsp:	dq 0
_rbp:	dq 0
_rsi:	dq 0
_rdi:	dq 0

_r8:	dq 0
_r9:	dq 0
_r10:	dq 0
_r11:	dq 0
_r12:   dq 0
_r13:	dq 0
_r14:	dq 0
_r15:	dq 0
_eflags:dq 0
_rip:	dq 0
_pre_rip:dq 0

_xmm_base:
_xmm0:  dq 0
	dq 0
_xmm1:	dq 0
	dq 0
_xmm2:	dq 0
	dq 0
_xmm3:	dq 0
	dq 0
_xmm4:	dq 0
	dq 0
_xmm5:	dq 0
	dq 0
_xmm6:	dq 0
	dq 0
_xmm7:	dq 0
	dq 0
;;; this is only used for x86-64
_xmm_ex_base:
_xmm8:	dq 0
	dq 0
_xmm9:	dq 0
	dq 0
_xmm10:	dq 0
	dq 0
_xmm11:	dq 0
	dq 0
_xmm12:	dq 0
	dq 0
_xmm13:	dq 0
	dq 0
_xmm14:	dq 0
	dq 0
_xmm15:	dq 0
	dq 0

_mxcsr:	dq 0
	dq 0
;;; segment register
_seg_base:
_es:	dq 0
_cs:	dq 0
_ss:	dq 0
_ds:	dq 0
_fs:	dq 0
_gs:	dq 0

;;; control register
_cr0:	dq 0
_cr1:	dq 0
_cr2:	dq 0
_cr3:	dq 0
_cr4:	dq 0
_cr5:	dq 0
_cr6:	dq 0
_cr7:	dq 0

_cr8:	dq 0

_xcr0:	dq 0

;;; Extended Feature Enable Register if AMD K6
_efer:	dq 0

;;;  this is auxietrary set of registers
__rax:	dq 0
__rcx:	dq 0
__rdx:	dq 0
__rbx:	dq 0
__rsp:	dq 0
__rbp:	dq 8
__rsi:	dq 0
__rdi:	dq 0
__r8:	dq 0
__r9:	dq 0
__r10:	dq 0
__r11:	dq 0
__r12:	dq 0
__r13:	dq 0
__r14:	dq 0
__r15:	dq 0
__eflags:dq 0
__rip:	dq 0

_dflag_len:
	dq 1
	dq 2
	dq 4
	dq 8

_tls1:
	dq 0
	
;; _rsp:	dq 0
	
_opcode_table:
	;; 0x00
	dq _0x00_add
	dq _0x01_add
	dq _0x02_add
	dq _0x03_add
	dq _0x04_add
	dq _0x05_add
	dq _0x00_add
	dq _0x00_add

	;; 0x08 or
	dq _0x08_or
	dq _0x09_or
	dq _0x0a_or
	dq _0x0b_or
	dq _0x0c_or
	dq _0x0d_or
	dq _0x00_add
	dq _0x0f

	;; 0x10 adc
	dq _0x10_adc
	dq _0x11_adc
	dq _0x12_adc
	dq _0x13_adc
	dq _0x14_adc
	dq _0x15_adc
	dq _0x00_add
	dq _0x00_add

	;; 0x18 sbb
	dq _0x18_sbb
	dq _0x19_sbb
	dq _0x1a_sbb
	dq _0x1b_sbb
	dq _0x1c_sbb
	dq _0x1d_sbb
	dq _0x00_add
	dq _0x1f
	
	;; 0x20 and
	dq _0x20_and
	dq _0x21_and
	dq _0x22_and
	dq _0x23_and
	dq _0x24_and
	dq _0x25_and
	dq _0x00_add
	dq _0x00_add

	;; 0x28 sub
	dq _0x28_sub
	dq _0x29_sub
	dq _0x2a_sub
	dq _0x2b_sub
	dq _0x2c_sub
	dq _0x2d_sub
	dq _0x2e_prefix_seg_cs
	dq _0x2f_bcd_das

	;; 0x30 xor
	dq _0x30_xor
	dq _0x31_xor
	dq _0x32_xor
	dq _0x33_xor
	dq _0x34_xor
	dq _0x35_xor
	dq _0x36_prefix_seg_ss
	dq _0x00_add

	;; 0x38 cmp
	dq _0x38_cmp
	dq _0x39_cmp
	dq _0x3a_cmp
	dq _0x3b_cmp
	dq _0x3c_cmp
	dq _0x3d_cmp
	dq _0x00_add
	dq _0x00_add

	;; 0x40
	dq _0x40_set_rex
	dq _0x41_set_rex
	dq _0x42_set_rex
	dq _0x43_set_rex
	dq _0x44_set_rex
	dq _0x45_set_rex
	dq _0x46_set_rex
	dq _0x47_set_rex
	
	;; 0x48
	dq _0x48_set_rex
	dq _0x49_set_rex
	dq _0x4a_set_rex
	dq _0x4b_set_rex
	dq _0x4c_set_rex
	dq _0x4d_set_rex
	dq _0x4e_set_rex
	dq _0x4f_set_rex

	;; 0x50
	dq _0x50_push
	dq _0x51_push
	dq _0x52_push
	dq _0x53_push
	dq _0x54_push
	dq _0x55_push
	dq _0x56_push
	dq _0x57_push

	;; 0x58
	dq _0x58_pop
	dq _0x59_pop
	dq _0x5a_pop
	dq _0x5b_pop
	dq _0x5c_pop
	dq _0x5d_pop
	dq _0x5e_pop
	dq _0x5f_pop

	dq _0x60_pusha
	dq _0x61_popa

	dq _0x62_bound
	dq _0x63_movslS
	dq _0x64_prefix_seg_fs
	dq _0x65_prefix_seg_gs
	dq _0x66_prefix_data
	dq _0x67_prefix_addr
	
	dq _0x68_push_iv
	dq _0x69_imul
	dq _0x6a_push_iv
	dq _0x6b_imul
	dq _0x6c_insS
	dq _0x6d_insS
	dq _0x6e_outsS
	dq _0x6f_outsS
	
	dq _0x70_jo
	dq _0x71_jno
	dq _0x72_jnae
	dq _0x73_jnc
	dq _0x74_je
	dq _0x75_jne
	dq _0x76_jna
	dq _0x77_ja
	dq _0x78_js
	dq _0x79_jns
	dq _0x7a_jpe
	dq _0x7b_jpo
	dq _0x7c_jnge
	dq _0x7d_jge
	dq _0x7e_jng
	dq _0x7f_jnle	

	dq _0x80_arith_imm
	dq _0x81_arith_imm
	dq _0x82_arith_imm
	dq _0x83_arith_imm
	dq _0x84_test
	dq _0x85_test
	dq _0x86_xchg
	dq _0x87_xchg
	dq _0x88_mov
	dq _0x89_mov
	dq _0x8a_mov
	dq _0x8b_mov
	dq _0x8c_mov_seg
	dq _0x8d_lea
	dq _0x8e_mov_seg	
	dq _0x8f_pop_ev

	dq _0x90_nop
	dq _0x91_xchg_eax
	dq _0x92_xchg_eax
	dq _0x93_xchg_eax
	dq _0x94_xchg_eax
	dq _0x95_xchg_eax
	dq _0x96_xchg_eax
	dq _0x97_xchg_eax
	dq _0x98_convert
	dq _0x99_convert
	dq _0x9a_lcall
	dq _0x9b_fwait
	dq _0x9c_pushf
	dq _0x9d_popf
	dq _0x9e_sahf
	dq _0x9f_lahf

	dq _0xa0_mov
	dq _0xa1_mov
	dq _0xa2_mov
	dq _0xa3_mov
	dq _0xa4_movsS	
	dq _0xa5_movsS
	dq _0xa6_cmpsS	
	dq _0xa7_cmpsS
	dq _0xa8_test	
	dq _0xa9_test
	dq _0xaa_stosS
	dq _0xab_stosS
	dq _0xac_lodsS
	dq _0xad_lodsS
	dq _0xae_scasS
	dq _0xaf_scasS
	
	dq _0xb0_mov	
	dq _0xb1_mov	
	dq _0xb2_mov	
	dq _0xb3_mov	
	dq _0xb4_mov	
	dq _0xb5_mov	
	dq _0xb6_mov	
	dq _0xb7_mov
	dq _0xb8_mov	
	dq _0xb9_mov	
	dq _0xba_mov	
	dq _0xbb_mov	
	dq _0xbc_mov	
	dq _0xbd_mov	
	dq _0xbe_mov	
	dq _0xbf_mov

	dq _0xc0_shift
	dq _0xc1_shift	
	dq _0xc2_ret
	dq _0xc3_ret
	dq _0xc4_les
	dq _0xc5_lds
	dq _0xc6_mov
	dq _0xc7_mov
	dq _0xc8_enter
	dq _0xc9_leave	
	dq _0xca_lret
	dq _0xcb_lret
	dq _0xcc_int3
	dq _0xcd_int	
	dq _0xce_into	
	dq _0xcf_iret
	
	dq _0xd0_shift
	dq _0xd1_shift
	dq _0xd2_shift
	dq _0xd3_shift
	dq _0xd4_aam
	dq _0xd5_aad
	dq _0xd6_salc	
	dq _0xd7_xlat
	
	dq _0xd8_float
	dq _0xd9_float
	dq _0xda_float
	dq _0xdb_float
	dq _0xdc_float
	dq _0xdd_float
	dq _0xde_float
	dq _0xdf_float

	dq _0xe0_loopnz
	dq _0xe1_loopz
	dq _0xe2_loop
	dq _0xe3_jecxz
	dq _0xe4_port_io
	dq _0xe5_port_io
	dq _0xe6_port_io
	dq _0xe7_port_io

	dq _0xe8_call
	dq _0xe9_jmp
	dq _0xea_jmp
	dq _0xeb_jmp	
	
	dq _0xec_port_io
	dq _0xed_port_io	
	dq _0xee_port_io
	dq _0xef_port_io
	
	dq _0xf0_prefix_lock
	dq _0xf1_icebp
	dq _0xf2_prefix_repnz
	dq _0xf3_prefix_repz
	dq _0xf4_hlt
	dq _0xf5_cmc
	dq _0xf6_op
	dq _0xf7_op
	dq _0xf8_clc
	dq _0xf9_stc
	dq _0xfa_cli
	dq _0xfb_sti	
	dq _0xfc_cld
	dq _0xfd_std
	dq _0xfe_op
	dq _0xff_op
	
	;; dq _0x01_add
	;; dq _0x02_add
	;; dq _0x03_add
	;; dq _0x04_add
	;; dq _0x05_add
	;; dq _hello_digit
	
	;; dq _0x08_add
	;; dq _0x09_add
	;; dq _0x0a_add
	;; dq _0x0b_add
	;; dq _0x0c_add
	;; dq _0x0d_add

_architecture:
_processor:
	db 0
_processor_mode:
	db 0
_cpl:
	db 0
_objformat:
	db 0
_meta_page_head:
	dq 0
_meta_page_ptr:
	dq 0
_out_page_head:
	dq 0
_out_page_ptr:
	dq 0
_draw_memory_table_page_head:
	dq 0
_draw_memory_table_page_ptr:
	dq 0
_current_fname:
	dq 0
_current_page_tail:
	dq 0
_fd_num:
	db 0

;;; Note that as reg/rm represents pointer to the register which is created by
;;; get_mod_reg_rm function, it contains dword size, and the rest of data is just 1byte.
align 0x10
_context:
._opcode_table:
	dq _opcode_table
._gen_code_base:
	dq 0
._displacement:
	dq 0
._arg1:
	dq 0
._arg2:
	dq 0
._res:
	dq 0
._imm_op:
	dq 0
._internal_arg1:
	dq 0
._internal_arg2:
	dq 0
._override_reg:
	dq 0
._reg:
	dq 0
._rm:
	dq 0
._imm_val:
	dq 0
._image_base:
	dq 0
._emu_base:
	dq 0
._sib_index:
	dq 0
._sib_base:
	dq 0
._sib_displacement:
	dq 0

._opcode: db 0
._rex: db 0
._data_prefix: db 0
._addr_prefix: db 0
._repz: db 0
._repnz: db 0
._lock: db 0
._vex: db 0
._override: db 0
._mod:
	db 0
._mod_type:
	db 0
._sib:
	db 0
._sib_scale:
	db 0
._rip_rel:
	db 0
._dflag:
	db 0
._aflag:
	db 0
._is_rm_src:
	db 0
;; ._extend_opcode:
;; 	db 0
;;; debugging purpose for temporaily storing the fetched instruction
_debug:
._inst:
	dq 0
	dq 0
._offset:
	db 0

	
;;; following should be aligned as it will be scooped from instruciton given a value of _context._mod.

_op01_f_base:
	dq _add
	dq _or
	dq _add
	dq _sub
	dq _and
	dq _sub
	dq _xor
	dq _cmp

_op_shift_base:	
	dq _rol
	dq _ror
	;; rcl 
	dq _rol
	;; rcr
	dq _ror
	;; shl
	dq _shl
	;; shr
	dq _shr
	;; shl1
	dq _shl
	;; sar
	dq _shr

_op_f6_f_base:
	dq _test
	dq 0
	dq _not
	dq _neg
	dq _mul
	dq _imul
	dq _div
	dq _idiv

_op_fe_f_base:
	dq _inc
	dq _dec
	dq _call
	;; lcall
	dq _call
	;; jmp
	dq _jmp
	;; ljmp
	dq _jmp
	;; push
	dq _push
	dq 0
	
;;; shift operation does not support two register operand but reg is always imm.
;;; [_context._arg2] which was set was as the degree of shift is going to be the offset
;;; to the function which will has one of them is register.

_op_shl_base:
	dq _shl0
	dq _shl1
	dq _shl2
	dq _shl3
	dq _shl4
	dq _shl5
	dq _shl6
	dq _shl7
	;; dq _shl8
	
_mod_load_base:
	dq _mod00_load
	dq _mod01_load
	dq _mod10_load
	dq _mod11_load	

_mod_store_base:
	dq _mod00_store
	dq _mod01_store
	dq _mod10_store
	dq _mod11_store	

_mod_fetch_base:
	dq _mod00_fetch_displacement
	dq _mod01_fetch_displacement
	dq _mod10_fetch_displacement
	dq _mod11_fetch_displacement

_load_base:
	dq _load8
	dq _load16
	dq _load32
	dq _load64

_fetch_base:
	dq _fetch8
	dq _fetch16
	dq _fetch32
	dq _fetch64

_add_base:
	dq _add8
	dq _add16
	dq _add32
	dq _add64

_sub_base:
	dq _sub8
	dq _sub16
	dq _sub32
	dq _sub64

_store_base:
	dq _store8
	dq _store16
	dq _store32
	dq _store64

_assign_base:
	dq _assign8
	dq _assign16
	dq _assign32
	dq _assign64

_and_base:
	dq _and8
	dq _and16
	dq _and32
	dq _and64

_or_base:
	dq _or8
	dq _or16
	dq _or32
	dq _or64

_xor_base:
	dq _xor8
	dq _xor16
	dq _xor32
	dq _xor64
	
msg1: db "Hello, world!", 10
.len: equ $ - msg1

msg2: db "Hello, pika!!", 10
.len: equ $ - msg2

_reg_size8:
	db 0x30,0x78,0x30,0x30,0x30,0x30,0x30,0x30,0x30,0x30,0x0a
	;; .len: equ $ - _reg_size8

msg4: db 0x30,0x62,0x30,0x30,0x30,0x30,0x30,0x30,0x30,0x30,0x0a
.len: equ $ - msg4
	
print_0x:
	db 0x32,0x78,0x00,
print_str:
	dq 0x00

_extend_opcode_table:

	;; 0x0f00
	dq 0
	dq 0
	dq 0
	dq 0
	dq 0
	dq _0x0f05_syscall
	dq 0
	dq 0
	;; 0x0f08
	dq 0
	dq 0
	dq 0
	dq 0
	dq 0
	dq 0
	dq 0
	dq 0
	;; 0x0f10
	dq _0x0f10_mov_xmm
	dq _0x0f11_mov_xmm
	dq _0x0f12_mov_xmm
	dq _0x0f13_mov_xmm
	dq _0x0f14_mov_xmm
	dq _0x0f15_mov_xmm
	dq _0x0f16_mov_xmm
	dq _0x0f17_mov_xmm
	;; 0x0f18
	dq _0x0f18_hint_nop
	dq _0x0f19_hint_nop
	dq _0x0f1a_hint_nop
	dq _0x0f1b_hint_nop
	dq _0x0f1c_hint_nop
	dq _0x0f1d_hint_nop
	dq _0x0f1e_hint_nop
	dq _0x0f1f_hint_nop
	;; 0x0f20
	dq _0x0f20_mov_crn
	dq _0x0f21_mov_drn
	dq _0x0f22_mov_crn
	dq _0x0f23_mov_drn
	dq 0
	dq 0
	dq 0
	dq 0
	;; 0x0f28
	dq _0x0f28_movaps
	dq _0x0f29_movaps
	dq _0x0f2a_convert_xmm
	dq _0x0f2b_mov_xmm
	dq _0x0f2c_convert_xmm
	dq _0x0f2d_convert_xmm
	dq _0x0f2e_ucmp_xmm
	dq _0x0f2f_cmp_xmm
	;; 0x0f30
	dq 0
	dq 0
	dq 0
	dq 0
	dq 0
	dq 0
	dq 0
	dq 0
	;; 0x0f38
	dq 0
	dq 0
	dq 0
	dq 0
	dq 0
	dq 0
	dq 0
	dq 0
	;; 0x0f40
	dq _0x0f40_cmovo
	dq _0x0f41_cmovno
	dq _0x0f42_cmovb
	dq _0x0f43_cmovnb
	dq _0x0f44_cmovz
	dq _0x0f45_cmovnz
	dq _0x0f46_cmovbe
	dq _0x0f47_cmovnbe
	;; 0x0f48
	dq _0x0f48_cmovs
	dq _0x0f49_cmovns
	dq _0x0f4a_cmovp
	dq _0x0f4b_cmovnp
	dq _0x0f4c_cmovl
	dq _0x0f4d_cmovnl
	dq _0x0f4e_cmovle
	dq _0x0f4f_cmovnle
	;; 0x0f50
	dq _0x0f50_movmskp
	dq _0x0f51_sqrt
	dq _0x0f52_rsqrt
	dq _0x0f53_rcp
	dq _0x0f54_rcp
	dq _0x0f55_andnp
	dq _0x0f56_orp
	dq _0x0f57_xorp
	;; 0x0f58
	dq _0x0f58_add
	dq _0x0f59_mul
	dq _0x0f5a_cvt2
	dq _0x0f5b_cvt2
	dq _0x0f5c_sub
	dq _0x0f5d_min
	dq _0x0f5e_div
	dq _0x0f5f_max
	;; 0x0f60
	dq _0x0f60_punpckl
	dq _0x0f61_punpckl
	dq _0x0f62_punpckl
	dq _0x0f63_packss
	dq _0x0f64_pcmpgt
	dq _0x0f65_pcmpgt
	dq _0x0f66_pcmpgt
	dq _0x0f67_packus
	;; 0x0f68
	dq _0x0f68_punpckh
	dq _0x0f69_punpckh
	dq _0x0f6a_punpckh
	dq _0x0f6b_packss
	dq _0x0f6c_punpckl
	dq _0x0f6d_punpckh
	dq _0x0f6e_mov
	dq _0x0f6f_mov
	;; 0x0f70
	dq _0x0f70_pshuf
	dq _0x0f71_pshift
	dq _0x0f72_pshift
	dq _0x0f73_pshift	
	dq _0x0f74_pcmpeq
	dq _0x0f75_pcmpeq
	dq _0x0f76_pcmpeq
	dq _0x0f77_emms
	;; 0x0f78
	dq _0x0f78_vmread
	dq _0x0f79_vmwrite
	dq 0
	dq 0	
	;; dq _0x0f7a
	;; dq _0x0f7b
	dq _0x0f7c_haddp
	dq _0x0f7d_hsubp
	dq _0x0f7e_mov
	dq _0x0f7f_mov
	;; 0x0f80
	dq _0x0f80_jo
	dq _0x0f81_jno
	dq _0x0f82_jnae
	dq _0x0f83_jnc
	dq _0x0f84_je
	dq _0x0f85_jne
	dq _0x0f86_jna
	dq _0x0f87_jnbe
	;; 0x0f88
	dq _0x0f88_js
	dq _0x0f89_jns
	dq _0x0f8a_jpe
	dq _0x0f8b_jpo
	dq _0x0f8c_jnl
	dq _0x0f8d_jng
	dq _0x0f8e_jng
	dq _0x0f8f_jnle
	;; 0x0f90
	dq _0x0f90_seto
	dq _0x0f91_setno
	dq _0x0f92_setb
	dq _0x0f93_setnb
	dq _0x0f94_sete
	dq _0x0f95_setne
	dq _0x0f96_setbe
	dq _0x0f97_setnbe
	;; 0x0f98
	dq _0x0f98_sets
	dq _0x0f99_setns
	dq _0x0f9a_setp
	dq _0x0f9b_setnp
	dq _0x0f9c_setl
	dq _0x0f9d_setnl
	dq _0x0f9e_setle
	dq _0x0f9f_setnle
	;; 0x0fa0
	dq _0x0fa0_pushfs
	dq _0x0fa1_popfs
	dq _0x0fa2_cpuid
	dq _0x0fa3_bt
	dq _0x0fa4_shld
	dq _0x0fa5_shld
	dq 0
	dq 0
	;; 0x0fa8	
	dq _0x0fa8_pushgs
	dq _0x0fa9_popgs
	dq _0x0faa_rsm
	dq _0x0fab_bts
	dq _0x0fac_shrd
	dq _0x0fad_shrd
	dq _0x0fae_fpu
	dq _0x0faf_imul
	;; 0x0fb0
	dq _0x0fb0_cmpxchg
	dq _0x0fb1_cmpxchg
	dq _0x0fb2_lss
	dq _0x0fb3_btr
	dq _0x0fb4_lfs
	dq _0x0fb5_lgs
	dq _0x0fb6_movzbS
	dq _0x0fb7_movzwS
	;; 0x0fb8
	dq 0
	dq 0
	dq 0
	dq 0
	dq 0
	dq 0
	dq _0x0fbe_movsbS
	dq _0x0fbf_movswS
	;; 0x0fc0
	dq _0x0fc0_xadd
	dq _0x0fc1_xadd
	dq _0x0fc2_cmp
	dq _0x0fc3_movnti
	dq _0x0fc4_pinsrw
	dq _0x0fc5_pextrw	
	dq _0x0fc6_shufp
	dq _0x0fc7_cmpxchg_vm
	;; 0x0fc8
	dq _0x0fc8_bswap
	dq _0x0fc9_bswap
	dq _0x0fca_bswap
	dq _0x0fcb_bswap
	dq _0x0fcc_bswap
	dq _0x0fcd_bswap
	dq _0x0fce_bswap
	dq _0x0fcf_bswap
	;; 0x0fd0
	dq _0x0fd0_addsubp
	dq _0x0fd1_psrlw
	dq _0x0fd2_psrld
	dq _0x0fd3_psrlq
	dq _0x0fd4_paddq
	dq _0x0fd5_pmullw	
	dq _0x0fd6_mov
	dq _0x0fd7_pmovmskb
	;; 0x0fd8
	dq _0x0fd8_psubus
	dq _0x0fd9_psubus
	dq _0x0fda_pminub
	dq _0x0fdb_pand
	dq _0x0fdc_paddusb
	dq _0x0fdd_paddusw
	dq _0x0fde_pmaxub
	dq _0x0fdf_pandn
	;; 0x0fe0
	dq _0x0fe0_pavgb
	dq _0x0fe1_psraw
	dq _0x0fe2_psrad
	dq _0x0fe3_pavgw
	dq _0x0fe4_pmulhuw
	dq _0x0fe5_pmulhw
	dq _0x0fe6_cvt2
	dq _0x0fe7_movnt
	;; 0x0fe8
	dq _0x0fe8_psubsb
	dq _0x0fe9_psubsw
	dq _0x0fea_pminsw
	dq _0x0feb_por
	dq _0x0fec_paddsb
	dq _0x0fed_paddsw
	dq _0x0fee_pmaxsw
	dq _0x0fef_pxor
	;; 0x0ff0
	dq _0x0ff0_lddqu
	dq _0x0ff1_psllw
	dq _0x0ff2_pslld
	dq _0x0ff3_psllq
	dq _0x0ff4_pmuludq
	dq _0x0ff5_pmaddwd
	dq _0x0ff6_psadbw
	dq _0x0ff7_maskmov
	;; 0x0ff8
	dq _0x0ff8_psubb
	dq _0x0ff9_psubw
	dq _0x0ffa_psubd
	dq _0x0ffb_psubq
	dq _0x0ffc_paddb
	dq _0x0ffd_paddw
	dq _0x0ffe_paddd
	dq _0x0fff

_reg_names:  db "rax rdx rcx rbx rsi rdi r8 r9 r10 r11 r12 r13 r14 r15 eflags rip rsp rbp"

%include "extern_data.asm"

