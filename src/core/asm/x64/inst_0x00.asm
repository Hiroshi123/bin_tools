
	default rel

	section .text

	global _0x00_add
	global _0x01_add
	global _0x02_add
	global _0x03_add
	global _0x04_add
	global _0x05_add

	global _0x08_or
	global _0x09_or
	global _0x0a_or
	global _0x0b_or
	global _0x0c_or
	global _0x0d_or
	
	global _0x0f
	
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

	extern _set_dflag
	extern _fetch
	
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

	extern _context._arg1
	extern _context._arg2
	
	extern  _context._mod	
	extern  _context._reg
	extern  _context._rm
	extern _context._opcode_table
	
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

	extern _or
	extern _or8

	extern _fetch
	extern _fetch8
	
	extern _assign
	extern _assign8
	
	extern _fetch_displacement_by_mod
	extern _extend_opcode_table
	extern _exec_one

_0x00_add:
	push rbp
	add byte [_rip],1
	call _get_mod_reg_rm
	call _set_scale_index_base
	call _fetch_displacement_by_mod
	call _mov_rm_to_arg1
	call _load_rm_by_mod
	call _mov_res_to_arg1
	call _set_reg_to_arg2
	call _add8
	call _mov_rm_to_arg1
	call _mov_res_to_arg2
	call _store_or_assign_arg1_by_mod	
	pop rbp
	ret
	
_0x01_add:
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
	pop rbp
	ret
;;; 0x02 & 0x03 is the inverse version of r/m & reg.
;;; in this case, destination is always reg and source is r/m
_0x02_add:
	add  dword [_rip],1
	call _get_mod_reg_rm
	call _set_scale_index_base
	call _fetch_displacement_by_mod
	call _mov_rm_to_arg1
	call _load_rm_by_mod
	call _mov_res_to_arg2
	call _set_reg_to_arg1
	call _add8
	call _mov_reg_to_arg1
	call _mov_res_to_arg2
	call _store_or_assign_arg1_by_mod
	ret

_0x03_add:
	add  dword [_rip],1
	call _get_mod_reg_rm
	call _set_scale_index_base
	call _fetch_displacement_by_mod
	call _mov_rm_to_arg1
	call _load_rm_by_mod
	call _mov_res_to_arg2
	call _set_reg_to_arg1
	call _add
	call _mov_reg_to_arg1
	call _mov_res_to_arg2
	call _store_or_assign_arg1_by_mod
	ret

;;; 0x04,0x05 does not need anything about 

_0x04_add:
	add byte [_rip],1
	call _fetch8
	call _mov_res_to_arg2
	mov rax,[_rax]
	mov [_context._arg1],rax
	call _add8
	mov rax,_rax
	mov [_context._arg1],rax
	call _mov_res_to_arg2
	call _assign8
	ret
_0x05_add:
	add byte [_rip],1
	call _set_dflag
	call _fetch
	call _mov_res_to_arg2
	mov rax,[_rax]
	mov [_context._arg1],rax
	call _add
	mov rax,_rax
	mov [_context._arg1],rax
	call _mov_res_to_arg2
	call _assign
	ret

_0x08_or:
	ret
_0x09_or:
	ret
_0x0a_or:
	ret
_0x0b_or:
	ret
_0x0c_or:
	add byte [_rip],1
	call _set_dflag
	call _fetch8
	call _mov_res_to_arg2
	mov rax,[_rax]
	mov [_context._arg1],rax
	call _or8
	mov rax,_rax
	mov [_context._arg1],rax
	call _mov_res_to_arg2
	call _assign8
	ret
_0x0d_or:
	add byte [_rip],1
	call _set_dflag
	call _fetch
	call _mov_res_to_arg2
	mov rax,[_rax]
	mov [_context._arg1],rax
	call _or
	mov rax,_rax
	mov [_context._arg1],rax
	call _mov_res_to_arg2
	call _assign
	ret
	
_0x0f:
	add byte [_rip],0x01
	mov rax,_extend_opcode_table
	mov [_context._opcode_table],rax	
	jmp _exec_one
	ret

	
