
	default rel

	section .text

	global _0x30_xor
	global _0x31_xor
	global _0x32_xor
	global _0x33_xor
	global _0x34_xor
	global _0x35_xor

	global _0x38_cmp
	global _0x39_cmp
	global _0x3a_cmp
	global _0x3b_cmp
	global _0x3c_cmp
	global _0x3d_cmp
	
	extern _rip

	extern _gen_code
	extern _get_mod_reg_rm

	extern _set_scale_index_base
	extern _set_gen_code_base_by_mod
	extern _set_immidiate
	extern _set_gen_code_op_by_dflag

	extern _load_arg1_by_mod
	extern _load_rm_by_mod

	extern _store_or_assign_arg1_by_mod
	
	extern _mov_reg_to_arg1	
	extern _mov_reg_to_arg2
	extern _mov_rm_to_arg1	
	extern _mov_rm_to_arg2
	extern _set_reg_to_arg1	
	extern _set_reg_to_arg2
	extern _set_rm_to_arg1
	extern _set_rm_to_arg2

	extern _mov_res_to_arg1	
	extern _mov_res_to_arg2
	
	extern _set_arg1_rm_arg2_reg
	extern _set_arg1_reg_arg2_rm
	
	extern print

	extern  _context._mod	
	extern  _context._reg
	extern  _context._rm
	
	extern _rax
	extern _rcx
	extern _rdx
	extern _rbx
	extern _rsp
	extern _rbp
	extern _rsi
	extern _rdi
	extern _r8
	extern _r9
	extern _r10
	extern _r11
	extern _r12
	extern _r13
	extern _r14
	extern _r15
	extern _rip
	
	extern _add
	extern _add8
	extern _add16
	extern _add32
	extern _add64
	
	extern _cmp
	extern _cmp8
	extern _cmp16
	extern _cmp32
	extern _cmp64

	
	extern _fetch_displacement_by_mod
	
_0x30_xor:
	push rbp
	pop rbp
	ret

_0x31_xor:
	push rbp
	pop rbp
	ret
_0x32_xor:
	push rbp
	pop rbp
	ret
_0x33_xor:
	push rbp
	pop rbp
	ret
_0x34_xor:
	push rbp
	pop rbp
	ret
_0x35_xor:
	push rbp
	pop rbp
	ret

_0x38_cmp:
	push rbp
	pop rbp
	ret

_0x39_cmp:
	push rbp
	mov r8,0x39
	call print

	add byte [_rip],1	
	call _get_mod_reg_rm
	call _set_scale_index_base
	call _fetch_displacement_by_mod
	call _mov_rm_to_arg1
	call _load_rm_by_mod
	call _mov_res_to_arg1
	call _set_reg_to_arg2
	call _cmp
	call _mov_rm_to_arg1
	call _mov_res_to_arg2
	call _store_or_assign_arg1_by_mod	

	pop rbp
	ret
_0x3a_cmp:
	push rbp
	pop rbp
	ret
_0x3b_cmp:
	push rbp
	pop rbp
	ret
_0x3c_cmp:
	push rbp
	pop rbp
	ret
_0x3d_cmp:
	push rbp
	pop rbp
	ret
	
___f:
	push rbp
	add byte [_rip],1	
	call _get_mod_reg_rm
	call _set_scale_index_base
	call _fetch_displacement_by_mod
	call _mov_rm_to_arg1
	call _load_rm_by_mod
	call _mov_res_to_arg1
	call _set_reg_to_arg2
	call _add
	call _mov_rm_to_arg1
	call _mov_res_to_arg2
	call _store_or_assign_arg1_by_mod	
	
	mov r8,0x01
	call print
	pop rbp
	ret
	