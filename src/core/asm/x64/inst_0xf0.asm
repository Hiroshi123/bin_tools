
	default rel
	section .text
	global _0xf0_prefix_lock
	global _0xf1_icebp
	global _0xf2_prefix_repnz
	global _0xf3_prefix_repz
	global _0xf4_hlt
	global _0xf5_cmc
	global _0xf6_op
	global _0xf7_op
	global _0xf8_clc
	global _0xf9_stc
	global _0xfa_cli
	global _0xfb_sti	
	global _0xfc_cld
	global _0xfd_std
	global _0xfe_op
	global _0xff_op

	extern _op_f6_f_base
	extern _op_fe_f_base	
	
	extern print
	extern _rax
	extern _rcx
	extern _rbp	
	extern _rsp
	extern _rip

	extern _sub
	
	extern _set_scale_index_base

	extern _mov_reg_to_arg1
	extern _mov_reg_to_arg2
	extern _set_reg_to_arg1
	extern _set_reg_to_arg2
	
	extern _mov_rm_to_arg1
	extern _mov_rm_to_arg2
	extern _set_rm_to_arg1
	extern _set_rm_to_arg2

	extern _mov_res_to_arg1
	extern _mov_res_to_arg2
	
	extern _load_arg1_by_mod
	extern _load_arg2_by_mod
	extern _load_rm_by_mod
	
	extern _store_or_assign_arg1_by_mod
	
	extern print
	extern _get_mod_reg_rm
	extern _get_mod_op_rm

	extern _context._reg
	extern _context._rm
	extern _context._dflag
	extern _context._rex
	extern _context._arg1	
	extern _context._arg2
	extern _context._res
	extern _context._imm_op
	
	extern _assign
	
	extern _fetch8_imm_set_to_arg2
	extern _fetch32_imm_set_to_arg2
	extern _fetch_displacement_by_mod

	extern _op01_f_base
	extern _set_imm_op_base

	extern _test
	extern _not
	extern _neg
	
_0xf0_prefix_lock:
	
	add r8,0xf0
	call print

	ret
_0xf1_icebp:
	add r8,0xf1
	call print

	ret
_0xf2_prefix_repnz:
	mov r8,0xf2
	call print

	ret
_0xf3_prefix_repz:
	ret
_0xf4_hlt:
	ret
_0xf5_cmc:
	ret

;;; f6/f7/fe/ff cannot set reg by itself as is is substituted for seleciton of opcode.
;;; It means reg is assumed from the opcode.

;;; test/not/neg/mul/imul/div/idiv
;;; in case of f6/f7, there is no 
;;; test is the only op which requires fetching imm32.
;;; not / neg needs only 1 operand.
;;; mul/imul/div/idiv requires 2 operand.
;;; register which was supposed to be indicated by reg
;;; is fixed on al/ax/eax/rax.

_0xf6_op:
	test rdx,0x77
	mov r8,0xf6
	call print
	ret

;;; test/not/neg/mul/imul/div/idiv
_0xf7_op:
	push rbp
	add byte [_rip],1

	mov r8,0xf7
	call print

	;; tmp
	mov r8,_op_f6_f_base
	call _set_imm_op_base
	call _get_mod_op_rm

	call _set_scale_index_base
	call _fetch_displacement_by_mod
	
	call _load_rm_by_mod
	;; note that the register that "load by mod" is put
	;; depends on opcode, which are determined by following.
	call _compensate_reg_f6f7
	
	call [_context._imm_op]
	
	call _mov_rm_to_arg1	
	call _mov_res_to_arg2
	call _store_or_assign_arg1_by_mod
	
	pop rbp
	ret
	
;;; if reg == test, fetch 32bit
;;; if 
_compensate_reg_f6f7:
	mov rax,[_context._imm_op]
	mov rdx,_test
	cmp rax,rdx
	je _compensate_reg_f6f7._test
	mov rdx,_not	
	cmp rax,rdx
	je _compensate_reg_f6f7._not_neg
	mov rdx,_neg
	cmp rax,rdx
	je _compensate_reg_f6f7._not_neg
	;; else, set rax to reg
	call _mov_res_to_arg2
	mov  rdx,_rax
	mov  [_context._reg],rdx
	call _mov_reg_to_arg1
	ret
	
._test:
	call _mov_res_to_arg1
	call _fetch32_imm_set_to_arg2
	ret
._not_neg:
	call _mov_res_to_arg1
	ret
	
_0xf8_clc:
	mov r8,0xf8
	call print

	ret
_0xf9_stc:
	ret
_0xfa_cli:
	mov r8,0xfa
	call print

	ret
_0xfb_sti:
	mov r8,0xfb
	call print
	ret
_0xfc_cld:
	mov r8,0xfc
	call print
	ret
_0xfd_std:
	mov r8,0xfd
	call print
	ret

;;; inc/dec/call/jmp/push
;;; this fe/ff does not require any immidiate fetch.
;;; it does not have reg specification as all of them does require just 1 operand.
;;; Nevertheless note that you need to load displacement
;;; if mod == 00 / 01 / 10, you need to load
	
_0xfe_op:
	
	push rbp
	add byte [_rip],1

	mov r8,0xf7
	call print

	;; tmp
	mov r8,_op_fe_f_base
	call _set_imm_op_base
	call _get_mod_op_rm

	call _set_scale_index_base
	call _fetch_displacement_by_mod
	
	call _load_rm_by_mod
	call _mov_res_to_arg1	
	;; call _fetch8_imm_set_to_arg2

	call [_context._imm_op]
	
	call _mov_rm_to_arg1	
	call _mov_res_to_arg2
	call _store_or_assign_arg1_by_mod
	
	mov r8,0xfe
	call print

	pop rbp
	ret

;;; inc/dec/call/jmp/push


;;; 
_0xff_op:

	push rbp
	add byte [_rip],1

	mov r8,0xff
	call print

	;; tmp
	mov r8,_op_fe_f_base
	call _set_imm_op_base
	call _get_mod_op_rm

	call _set_scale_index_base
	;; 
	call _fetch_displacement_by_mod
	;; if mod == 00 or 01 or 10, you need memory access.
	call _load_rm_by_mod
	call _mov_res_to_arg1
	mov r8,0xff
	call print	
	;; call _mov_rm_to_arg1
	call [_context._imm_op]

	;; storing will be done by
	
	;; call _mov_rm_to_arg1	
	;; call _mov_res_to_arg2
	;; call _store_or_assign_arg1_by_mod
	
	mov r8,0xff
	call print

	pop rbp
	ret

	
