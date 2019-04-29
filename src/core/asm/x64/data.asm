

	default rel
	
	section .data

	global _reg_names

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
	;; global _write
	global _context
	global _context._rex	
	global _context._mod
	global _context._reg
	global _context._rm
	global _context._scale
	global _context._index
	global _context._base
	global _context._dflag
	global _context._aflag	
	global _context._data_prefix
	global _context._addr_prefix
	global _context._arg1
	global _context._arg2
	global _context._res
	global _context._imm

	global _op01_f_base
	global _mod_f_base01
	global _mod_f_base02
	global _reg_size8

	global _load_base
	global _store_base
	global _fetch_base
	global _add_base
	global _sub_base
	global _assign_base

	
_reg_names:  db "rax rdx rcx rbx rsi rdi r8 r9 r10 r11 r12 r13 r14 r15 eflags rip rsp rbp "
	
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

;; _rsp:	dq 0
	
_opcode_table:
	;; 0x00
	dq _0x00_add
	dq _0x00_add
	dq _0x00_add
	dq _0x00_add
	dq _0x00_add
	dq _0x00_add
	dq _0x00_add
	dq _0x00_add

	;; 0x08
	dq _0x00_add
	dq _0x00_add
	dq _0x00_add
	dq _0x00_add
	dq _0x00_add
	dq _0x00_add
	dq _0x00_add
	dq _0x00_add

	;; 0x10
	dq _0x00_add
	dq _0x00_add
	dq _0x00_add
	dq _0x00_add
	dq _0x00_add
	dq _0x00_add
	dq _0x00_add
	dq _0x00_add

	;; 0x18
	dq _0x00_add
	dq _0x00_add
	dq _0x00_add
	dq _0x00_add
	dq _0x00_add
	dq _0x00_add
	dq _0x00_add
	dq _0x00_add

	;; 0x20
	dq _0x00_add
	dq _0x00_add
	dq _0x00_add
	dq _0x00_add
	dq _0x00_add
	dq _0x00_add
	dq _0x00_add
	dq _0x00_add

	;; 0x28
	dq _0x00_add
	dq _0x00_add
	dq _0x00_add
	dq _0x00_add
	dq _0x00_add
	dq _0x00_add
	dq _0x00_add
	dq _0x00_add

	;; 0x30
	dq _0x00_add
	dq _0x00_add
	dq _0x00_add
	dq _0x00_add
	dq _0x00_add
	dq _0x00_add
	dq _0x00_add
	dq _0x00_add

	;; 0x38
	dq _0x00_add
	dq _0x00_add
	dq _0x00_add
	dq _0x00_add
	dq _0x00_add
	dq _0x00_add
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
	dq _0x98_cwde
	dq _0x99_cwd
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
	
	

;;; Note that as reg/rm represents pointer to the register which is created by
;;; get_mod_reg_rm function, it contains dword size, and the rest of data is just 1byte.
_context:
._rex: db 0	
._data_prefix: db 0
._addr_prefix: db 0
._mod:
	db 0
._reg:
	dq 0
._rm:
	dq 0
._scale:
	db 0
._index:
	dq 0
._base:
	dq 0
._dflag:
	db 0
._aflag:
	db 0
._gen_code_base:
	dq 0
._load:
	dq 0
._arg1:
	dq 0
._arg2:
	dq 0
._res:
	dq 0
._imm:
	dq 0
	
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
	
	
_mod_f_base01:
	dq _mod00_do1
	dq _mod01_do1
	dq _mod10_do1
	dq _mod11_do1	

_mod_f_base02:
	dq _mod00_do2
	dq _mod01_do2
	dq _mod10_do2
	dq _mod11_do2	

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

	
	section .text
;;;

	extern _add
	extern _sub
	extern _and
	extern _or
	extern _xor
	extern _cmp
	
	extern _mod00_do1
	extern _mod01_do1
	extern _mod10_do1
	extern _mod11_do1

	extern _mod00_do2
	extern _mod01_do2
	extern _mod10_do2
	extern _mod11_do2

	extern _load_arg1_by_mod
	extern _load_arg2_by_mod

	extern _0x00_add
	
	extern _0x40_set_rex
	extern _0x41_set_rex
	extern _0x42_set_rex
	extern _0x43_set_rex
	extern _0x44_set_rex
	extern _0x45_set_rex
	extern _0x46_set_rex
	extern _0x47_set_rex
	;; 0x48
	extern _0x48_set_rex
	extern _0x49_set_rex
	extern _0x4a_set_rex
	extern _0x4b_set_rex
	extern _0x4c_set_rex
	extern _0x4d_set_rex
	extern _0x4e_set_rex
	extern _0x4f_set_rex
	;; 0x50
	extern _0x50_push
	extern _0x51_push
	extern _0x52_push
	extern _0x53_push
	extern _0x54_push
	extern _0x55_push
	extern _0x56_push
	extern _0x57_push
	;; 0x58
	extern _0x58_pop
	extern _0x59_pop
	extern _0x5a_pop
	extern _0x5b_pop
	extern _0x5c_pop
	extern _0x5d_pop
	extern _0x5e_pop
	extern _0x5f_pop

	;; 0x60
	extern _0x60_pusha
	extern _0x61_popa

	extern _0x62_bound
	extern _0x63_movslS
	extern _0x64_prefix_seg_fs
	extern _0x65_prefix_seg_gs
	extern _0x66_prefix_data
	extern _0x67_prefix_addr
	
	extern _0x68_push_iv
	extern _0x69_imul
	extern _0x6a_push_iv
	extern _0x6b_imul
	extern _0x6c_insS
	extern _0x6d_insS
	extern _0x6e_outsS
	extern _0x6f_outsS
	
	extern _0x70_jo
	extern _0x71_jno
	extern _0x72_jnae
	extern _0x73_jnc
	extern _0x74_je
	extern _0x75_jne
	extern _0x76_jna
	extern _0x77_ja
	extern _0x78_js
	extern _0x79_jns
	extern _0x7a_jpe
	extern _0x7b_jpo
	extern _0x7c_jnge
	extern _0x7d_jge
	extern _0x7e_jng
	extern _0x7f_jnle
	;;
	extern _0x80_arith_imm
	extern _0x81_arith_imm
	extern _0x82_arith_imm
	extern _0x83_arith_imm
	extern _0x84_test
	extern _0x85_test
	extern _0x86_xchg
	extern _0x87_xchg
	extern _0x88_mov
	extern _0x89_mov
	extern _0x8a_mov
	extern _0x8b_mov
	extern _0x8c_mov_seg
	extern _0x8d_lea
	extern _0x8e_mov_seg	
	extern _0x8f_pop_ev

	extern _0x90_nop
	extern _0x91_xchg_eax
	extern _0x92_xchg_eax
	extern _0x93_xchg_eax
	extern _0x94_xchg_eax
	extern _0x95_xchg_eax
	extern _0x96_xchg_eax
	extern _0x97_xchg_eax
	extern _0x98_cwde
	extern _0x99_cwd
	extern _0x9a_lcall
	extern _0x9b_fwait
	extern _0x9c_pushf
	extern _0x9d_popf
	extern _0x9e_sahf
	extern _0x9f_lahf

	extern _0xa0_mov
	extern _0xa1_mov
	extern _0xa2_mov
	extern _0xa3_mov
	extern _0xa4_movsS	
	extern _0xa5_movsS
	extern _0xa6_cmpsS	
	extern _0xa7_cmpsS
	extern _0xa8_test	
	extern _0xa9_test
	extern _0xaa_stosS
	extern _0xab_stosS
	extern _0xac_lodsS
	extern _0xad_lodsS
	extern _0xae_scasS
	extern _0xaf_scasS
	
	extern _0xb0_mov	
	extern _0xb1_mov	
	extern _0xb2_mov	
	extern _0xb3_mov	
	extern _0xb4_mov	
	extern _0xb5_mov	
	extern _0xb6_mov	
	extern _0xb7_mov
	extern _0xb8_mov	
	extern _0xb9_mov	
	extern _0xba_mov	
	extern _0xbb_mov	
	extern _0xbc_mov	
	extern _0xbd_mov	
	extern _0xbe_mov	
	extern _0xbf_mov

	extern _0xc0_shift
	extern _0xc1_shift	
	extern _0xc2_ret
	extern _0xc3_ret
	extern _0xc4_les
	extern _0xc5_lds
	extern _0xc6_mov
	extern _0xc7_mov
	extern _0xc8_enter
	extern _0xc9_leave	
	extern _0xca_lret
	extern _0xcb_lret
	extern _0xcc_int3
	extern _0xcd_int	
	extern _0xce_into	
	extern _0xcf_iret
	
	extern _0xd0_shift
	extern _0xd1_shift
	extern _0xd2_shift
	extern _0xd3_shift
	extern _0xd4_aam
	extern _0xd5_aad
	extern _0xd6_salc	
	extern _0xd7_xlat
	
	extern _0xd8_float
	extern _0xd9_float
	extern _0xda_float
	extern _0xdb_float
	extern _0xdc_float
	extern _0xdd_float
	extern _0xde_float
	extern _0xdf_float

	extern _0xe0_loopnz
	extern _0xe1_loopz
	extern _0xe2_loop
	extern _0xe3_jecxz
	extern _0xe4_port_io
	extern _0xe5_port_io
	extern _0xe6_port_io
	extern _0xe7_port_io

	extern _0xe8_call
	extern _0xe9_jmp
	extern _0xea_jmp
	extern _0xeb_jmp	
	
	extern _0xec_port_io
	extern _0xed_port_io	
	extern _0xee_port_io
	extern _0xef_port_io
	
	extern _0xf0_prefix_lock
	extern _0xf1_icebp
	extern _0xf2_prefix_repnz
	extern _0xf3_prefix_repz
	extern _0xf4_hlt
	extern _0xf5_cmc
	extern _0xf6_op
	extern _0xf7_op
	extern _0xf8_clc
	extern _0xf9_stc
	extern _0xfa_cli
	extern _0xfb_sti	
	extern _0xfc_cld
	extern _0xfd_std
	extern _0xfe_op
	extern _0xff_op

	;; following will be primitive instruction where x86-63 heads to.

	extern _fetch
	extern _fetch8
	extern _fetch16
	extern _fetch32
	extern _fetch64

	extern _load8
	extern _load16
	extern _load32
	extern _load64

	extern _store8
	extern _store16
	extern _store32
	extern _store64

	extern _assign8
	extern _assign16
	extern _assign32
	extern _assign64

	extern _add8
	extern _add16
	extern _add32
	extern _add64
	
	extern _sub8
	extern _sub16
	extern _sub32
	extern _sub64

	section .text
	global _hello_world
	
_hello_world:
	;; mov rax, 0x2000004 ; write
	mov rax, 0x0000001 ; write
	mov rdi, 1 ; stdout
	mov rsi, msg1
	mov rdx, msg1.len
	syscall
	ret
	