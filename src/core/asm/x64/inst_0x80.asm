
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
	extern _rip

	extern _set_scale_index_base
	
	extern _set_reg_to_arg1
	extern _mov_reg_to_arg1
	
	extern _load_arg1_by_mod
	extern _load_arg2_by_mod
	extern _store_or_assign_arg1_by_mod
	
	extern print
	extern _get_mod_reg_rm

	extern _context._dflag
	extern _assign
	
;;; 0x80

_0x80_arith_imm:
	ret
	
_0x81_arith_imm:
	ret
	
_0x82_arith_imm:
	ret
	
_0x83_arith_imm:
	ret

_0x84_test:
	ret
_0x85_test:
	ret

_0x86_xchg:
	ret

_0x87_xchg:
	ret
	
_0x88_mov:
	ret
	
_0x89_mov:
	push rbp
	add byte [_rip],1

	mov r8,0x11
	call print
	call _get_mod_reg_rm
	mov r8,[_context._dflag]
	call print
	call _set_scale_index_base
	call _store_or_assign_arg1_by_mod
	pop rbp
	ret

_0x8a_mov:
	ret

_0x8b_mov:
	push rbp
	mov qword [_rcx],7
	add byte [_rip],1
	call _get_mod_reg_rm
	call _set_scale_index_base
	call _load_arg2_by_mod
	call _mov_reg_to_arg1
	call _assign
	pop rbp
	ret

_0x8c_mov_seg:
	ret
_0x8d_lea:
	ret
_0x8e_mov_seg:
	ret
_0x8f_pop_ev:
	ret

