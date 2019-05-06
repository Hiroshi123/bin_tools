
	default rel

	section .text

	global _0x20_and
	global _0x21_and
	global _0x22_and
	global _0x23_and
	global _0x24_and
	global _0x25_and

	global _0x28_sub
	global _0x29_sub
	global _0x2a_sub
	global _0x2b_sub
	global _0x2c_sub
	global _0x2d_sub
	
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
	
	extern _sub
	extern _sub8
	extern _sub16
	extern _sub32
	extern _sub64

	
	extern _fetch_displacement_by_mod
	
_0x20_and:
	push rbp
	pop rbp
	ret

_0x21_and:
	push rbp
	pop rbp
	ret
_0x22_and:
	push rbp
	pop rbp
	ret
_0x23_and:
	push rbp
	pop rbp
	ret
_0x24_and:
	push rbp
	pop rbp
	ret
_0x25_and:
	push rbp
	pop rbp
	ret

_0x28_sub:
	push rbp
	pop rbp
	ret

_0x29_sub:
	push rbp
	mov r8,0x29
	call print

	add byte [_rip],1	
	call _get_mod_reg_rm
	call _set_scale_index_base
	call _fetch_displacement_by_mod
	call _mov_rm_to_arg1
	call _load_rm_by_mod
	call _mov_res_to_arg1
	call _set_reg_to_arg2
	call _sub
	call _mov_rm_to_arg1
	call _mov_res_to_arg2
	call _store_or_assign_arg1_by_mod	

	pop rbp
	ret
_0x2a_sub:
	push rbp
	pop rbp
	ret
_0x2b_sub:
	push rbp
	pop rbp
	ret
_0x2c_sub:
	push rbp
	pop rbp
	ret
_0x2d_sub:
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
	
