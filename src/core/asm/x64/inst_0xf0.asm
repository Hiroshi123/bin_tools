
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

	extern _exec_one
	
	extern _op_f6_f_base
	extern _op_fe_f_base

	extern _context._repz
	extern _context._repnz
	extern _context._lock	
	
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
	extern _fetch_displacement_by_mod

	extern _op01_f_base
	extern _set_imm_op_base
	
_0xf0_prefix_lock:
	
	add r8,0xf0
	call print

	ret
_0xf1_icebp:
	add r8,0xf1
	call print
	ret

;;; repnz stops when cz register is 0 & flag register is 0. 
_0xf2_prefix_repnz:
	add dword [_rip],1
	mov byte [_context._repnz],0xff
	jmp _exec_one
	
_0xf3_prefix_repz:
	add dword [_rip],1
	mov byte [_context._repz],0xff
	jmp _exec_one

_0xf4_hlt:
	ret
_0xf5_cmc:
	ret

;;; f6/f7/fe/ff cannot set reg by itself as is is substituted for seleciton of opcode.
;;; It means reg is assumed from the opcode.

;;; test/not/neg/mul/imul/div/idiv

_0xf6_op:
	
	mov r8,0xf6
	call print
	ret
	
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
	call _mov_res_to_arg1	
	;; call _fetch8_imm_set_to_arg2

	call [_context._imm_op]
	
	call _mov_rm_to_arg1	
	call _mov_res_to_arg2
	call _store_or_assign_arg1_by_mod
	
	pop rbp
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
;;; when you jmp to absolute position, if it will retrieve the value on .rdata
;;; when you load dll or exe, you just mark where is iat which needs to be filled out
;;; as it is referred from calling to other function of another dll.
;;; From abs jmp/call side, if you try to check the value of iat, and it still holds the
;;; reference to int, it does not read the value of iat.
;;; Instead pause the execution of the instruction until loader should handle resolution.
;;; loader will look for the name of function and dll which contains it.
;;; resolve only the function.

;;; what you need to implement.
;;; 1. checking of range of IAT
;;; 2. resolution which targets only one pair of function assuming the dll had been already mapped.

;;; [memory access]
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
	
	;; call _load_rm_by_mod
	;; call _mov_res_to_arg1
	call _mov_rm_to_arg1
	call [_context._imm_op]

	;; storing will be done by
	
	;; call _mov_rm_to_arg1	
	;; call _mov_res_to_arg2
	;; call _store_or_assign_arg1_by_mod
	
	mov r8,0xff
	call print

	pop rbp
	ret

	
