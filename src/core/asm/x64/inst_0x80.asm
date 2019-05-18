
	default rel
	
	section .text
	global _0x80_arith_imm
	global _0x81_arith_imm
	global _0x82_arith_imm
	global _0x83_arith_imm
	global _0x84_test
	global _0x85_test
	global _0x86_xchg
	global _0x87_xchg
	global _0x88_mov
	global _0x89_mov
	global _0x8a_mov
	global _0x8b_mov
	global _0x8c_mov_seg
	global _0x8d_lea
	global _0x8e_mov_seg	
	global _0x8f_pop_ev

	extern _rax
	extern _rcx
	extern _rbp	
	extern _rsp
	extern _rip

	extern _sub
	extern _and
	
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
	extern _fetch8_imm_set_to_arg2_with_sign

	extern _fetch32_imm_set_to_arg2

	extern _fetch_displacement_by_mod

	extern _op01_f_base
	extern _set_imm_op_base
	extern _set_dflag_as_1byte
	
;;; 0x80

_0x80_arith_imm:
	ret
	
_0x81_arith_imm:
	push rbp
	add byte [_rip],1

	mov r8,0x81
	call print

	;; tmp
	mov r8,_op01_f_base	
	call _set_imm_op_base
	call _get_mod_op_rm

	call _set_scale_index_base
	call _fetch_displacement_by_mod
	
	call _load_rm_by_mod
	call _mov_res_to_arg1	
	call _fetch32_imm_set_to_arg2

	call [_context._imm_op]
	
	call _mov_rm_to_arg1	
	call _mov_res_to_arg2
	call _store_or_assign_arg1_by_mod
	mov r8,0x81
	call print
	
	pop rbp

	ret
	
_0x82_arith_imm:
	ret

;;; 0x83 feeds just imm 1byte to the register which can contain longer bytes.
;;; Note that the all rest of bytes are sign extended.

_0x83_arith_imm:
	
	push rbp
	add byte [_rip],1

	
	;; tmp
	mov r8,_op01_f_base	
	call _set_imm_op_base
	call _get_mod_op_rm

	call _set_scale_index_base
	call _fetch_displacement_by_mod
	
	call _load_rm_by_mod
	call _mov_res_to_arg1
	call _fetch8_imm_set_to_arg2_with_sign

	mov r8,0x83
	call print
	
	mov r8,[_context._arg2]
	call print
		
	call [_context._imm_op]
	
	mov r8,[_context._res]
	call print
	
	call _mov_rm_to_arg1
	call _mov_res_to_arg2
	call _store_or_assign_arg1_by_mod

	pop rbp
	ret

_0x84_test:
	ret
_0x85_test:

	push rbp
	add byte [_rip],1	
	call _get_mod_op_rm
	call _set_scale_index_base
	call _fetch_displacement_by_mod
	call _and
	call _mov_rm_to_arg1
	call _set_reg_to_arg2
	call _store_or_assign_arg1_by_mod
	
	mov r8,0x85
	call print
	
	pop rbp
	ret

_0x86_xchg:
	ret

_0x87_xchg:
	
	ret
	
_0x88_mov:
	push rbp

	add byte [_rip],1
	call _get_mod_reg_rm
	mov byte [_context._dflag],0x00

	call _set_scale_index_base
	call _fetch_displacement_by_mod
	call _mov_rm_to_arg1
	call _set_reg_to_arg2
	call _store_or_assign_arg1_by_mod

	mov r8,0x88
	call print
	
	pop rbp	
	ret
	
_0x89_mov:
	
	push rbp
	add byte [_rip],1
	call _get_mod_reg_rm
	call _set_scale_index_base
	call _fetch_displacement_by_mod
	call _mov_rm_to_arg1
	call _set_reg_to_arg2
	call _store_or_assign_arg1_by_mod
	
	mov r8,0x89
	call print
	
	pop rbp
	ret
	
_0x8a_mov:
	ret

_0x8b_mov:
	
	push rbp
	add byte [_rip],1
	call _get_mod_reg_rm
	call _set_scale_index_base

	mov r8,0x8b
	call print

	call _fetch_displacement_by_mod

	mov r8,0x8b
	call print
	
	call _load_rm_by_mod
	mov r8,0x8b
	call print
	call _mov_res_to_arg2
	call _mov_reg_to_arg1
	call _assign
	pop rbp
	ret

_0x8c_mov_seg:
	ret
_0x8d_lea:
	push rbp
	mov r8,0x8d
	call print

	add byte [_rip],1
	call _get_mod_reg_rm
	call _set_scale_index_base
	call _fetch_displacement_by_mod
	;; note lea does not need memory access.
	;; call _load_rm_by_mod
	call _mov_rm_to_arg2
	call _mov_reg_to_arg1
	call _assign

	pop rbp
	ret
_0x8e_mov_seg:
	ret
_0x8f_pop_ev:
	ret

