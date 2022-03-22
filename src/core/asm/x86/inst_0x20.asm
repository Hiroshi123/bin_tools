
	default rel

	section .text

	global _0x20_and
	global _0x21_and
	global _0x22_and
	global _0x23_and
	global _0x24_and
	global _0x25_and
	global _0x26_prefix_seg_es
	global _0x27_bcd_daa
	
	global _0x28_sub
	global _0x29_sub
	global _0x2a_sub
	global _0x2b_sub
	global _0x2c_sub
	global _0x2d_sub
	global _0x2e_prefix_seg_cs
	global _0x27_bcd_das
	
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
	extern _exec
	extern _exec_one

	extern _cs
	extern _es
	
	extern  _context._mod	
	extern  _context._reg
	extern  _context._rm
	extern  _context._override
	extern  _context._override_reg
	
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
	
	extern _and
	extern _and8
	extern _and16
	extern _and32
	extern _and64
	
	extern _sub
	extern _sub8
	extern _sub16
	extern _sub32
	extern _sub64
	
	extern _fetch_displacement_by_mod

%include "extern_for_inst.asm"
	
_0x20_and:
	mov r15b,0xff
	jmp _do01
	ret

_0x21_and:
	jmp _do01	
	add byte [_rip],1	
	call _get_mod_reg_rm
	call _set_scale_index_base
	call _fetch_displacement_by_mod
	call _mov_rm_to_arg1
	call _load_rm_by_mod
	call _mov_res_to_arg1
	call _set_reg_to_arg2
	call _and
	call _mov_rm_to_arg1
	call _mov_res_to_arg2
	call _store_or_assign_arg1_by_mod	

	ret
_0x22_and:
	mov r15b,0xff
	mov byte [_context._is_rm_src],0xff
	jmp _do01
_0x23_and:
	mov byte [_context._is_rm_src],0xff
	jmp _do01
_0x24_and:
	jmp _do02
_0x25_and:
	call _set_dflag
	call _do02
	ret
	
_0x26_prefix_seg_es:
	add dword [_rip],1
	mov byte [_context._override],0xff
	mov rax,_es
	mov [_context._override_reg],rax
	jmp _exec_one	

_0x27_bcd_daa:
	ret

_0x28_sub:
	mov r15b,0xff
	jmp _do01
	ret

_0x29_sub:
	jmp _do01
	
	add byte [_rip],1	
	call _get_mod_reg_rm
	call _set_scale_index_base
	call _fetch_displacement_by_mod

	ret
	
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
	mov r15b,0xff
	mov byte [_context._is_rm_src],0xff	
	jmp _do01
	ret
_0x2b_sub:
	mov byte [_context._is_rm_src],0xff
	jmp _do01
	ret
_0x2c_sub:
	add byte [_rip],1
	call _set_dflag
	call _fetch8
	mov rax,[_context._res]
	mov [_context._imm_val],rax
	mov rax,[_rax]
	mov [_context._rm],rax
	ret
_0x2d_sub:
	add byte [_rip],1
	call _set_dflag
	call _fetch_no64
	mov rax,[_context._res]
	mov [_context._imm_val],rax
	mov rax,_rax
	mov [_context._rm],rax
	ret

_0x2e_prefix_seg_cs:
	add dword [_rip],1
	;; probably different behavior on 32/64
	jmp _exec
	mov byte [_context._override],0xff
	mov rax,_cs
	mov [_context._override_reg],rax
	jmp _exec_one	

_0x2f_bcd_das:
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
	call _and
	call _mov_rm_to_arg1
	call _mov_res_to_arg2
	call _store_or_assign_arg1_by_mod	
	
	mov r8,0x01
	call print
	pop rbp
	ret
	
