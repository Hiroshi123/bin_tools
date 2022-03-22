
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

%include "extern_for_inst.asm"
	
_0x00_add:
	add byte [_rip],1
	;; mov rax,_get_mod_reg_rm
	call _get_mod_reg_rm
	call _reset_higher8_access
	call _set_scale_index_base	
	;; call _inc_displacement
	call _fetch_displacement_by_mod
	ret

	;; you do not need to execute below if your purpose is just for decrementing
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

	add dword [_rip],1
	call _get_mod_reg_rm
	call _set_scale_index_base
	call _inc_displacement
	;; call _fetch_displacement_by_mod
	ret
	
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
	ret

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
	ret

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
	
	ret
	
	mov [_context._arg1],rax
	call _add8
	mov rax,_rax
	mov [_context._arg1],rax
	call _mov_res_to_arg2
	call _assign8
	ret
	
_0x05_add:
	add dword [_rip],1
	call _set_dflag
	call _fetch_no64
	mov rax,[_context._res]
	mov [_context._imm_val],rax
	mov rax,[_rax]
	mov [_context._rm],rax
	ret

	mov [_context._arg1],rax
	call _add
	mov rax,_rax
	mov [_context._arg1],rax
	call _mov_res_to_arg2
	call _assign
	ret

_0x08_or:
	mov r15b,0xff
_0x09_or:
	jmp _do01
_0x0a_or:
	mov r15b,0xff
_0x0b_or:
	mov byte [_context._is_rm_src],0xff
	jmp _do01
_0x0c_or:
	jmp _do02	
	
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
	call _set_dflag
	jmp _do02	
	
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
	add dword [_rip],0x01
	;; mov [_context._opcode],rax	
	mov rax,_extend_opcode_table
	mov [_context._opcode_table],rax
	jmp _exec_one
	ret

	
