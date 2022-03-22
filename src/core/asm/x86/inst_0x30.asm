
	default rel

	section .text

	global _0x30_xor
	global _0x31_xor
	global _0x32_xor
	global _0x33_xor
	global _0x34_xor
	global _0x35_xor
	global _0x36_prefix_seg_ss
	global _0x37_bcd_aaa

	global _0x38_cmp
	global _0x39_cmp
	global _0x3a_cmp
	global _0x3b_cmp
	global _0x3c_cmp
	global _0x3d_cmp
	global _0x3e_prefix_seg_ds
	global _0x3f_bcd_aas
	
%include "extern_for_inst.asm"

__com:
	call _get_mod_reg_rm
	call _set_scale_index_base
	call _fetch_displacement_by_mod
	;; mov rax,[_context._res]
	;; mov [_context._imm_val],rax
	ret

_0x30_xor:
	mov byte r15b,0xff
	add byte [_rip],1
	jmp __com

_0x31_xor:
	jmp _do01
	
	call _get_mod_reg_rm
	call _set_scale_index_base
	call _fetch_displacement_by_mod
	call _mov_rm_to_arg1
	call _load_rm_by_mod
	call _mov_res_to_arg1
	call _set_reg_to_arg2
	call _xor
	call _mov_rm_to_arg1
	call _mov_res_to_arg2
	call _store_or_assign_arg1_by_mod	

	pop rbp
	ret
_0x32_xor:	
	jmp __com	
_0x33_xor:
	mov byte [_context._is_rm_src],0xff
	jmp _do01
	
	add byte [_rip],1	
	call _get_mod_reg_rm
	call _set_scale_index_base
	call _fetch_displacement_by_mod
	call _mov_rm_to_arg1
	call _load_rm_by_mod
	call _mov_res_to_arg1
	call _set_reg_to_arg2
	call _xor
	call _mov_rm_to_arg1
	call _mov_res_to_arg2
	call _store_or_assign_arg1_by_mod	
	ret
_0x34_xor:
	jmp __com
_0x35_xor:
	jmp __com
_0x36_prefix_seg_ss:
	add dword [_rip],1
	mov byte [_context._override],0xff
	mov rax,_ss
	mov [_context._override_reg],rax
	jmp _exec_one

_0x37_bcd_aaa:
	ret

_0x38_cmp:
	mov r15b,0xff
	jmp _do01

	ret
	call _load_rm_by_mod
	call _mov_res_to_arg1
	call _set_reg_to_arg2
	call _cmp8
	call _mov_rm_to_arg1
	call _mov_res_to_arg2
	call _store_or_assign_arg1_by_mod
	ret

_0x39_cmp:
	jmp _do01

	add dword [_rip],1	
	call _get_mod_reg_rm
	call _set_scale_index_base
	call _fetch_displacement_by_mod
	ret

	call _mov_rm_to_arg1	
	call _load_rm_by_mod
	call _mov_res_to_arg1
	call _set_reg_to_arg2
	call _cmp
	call _mov_rm_to_arg1
	call _mov_res_to_arg2
	call _store_or_assign_arg1_by_mod	
	ret
_0x3a_cmp:
	mov r15b,0xff
	mov byte [_context._is_rm_src],0xff	
	jmp _do01

	call _get_mod_reg_rm
	call _set_scale_index_base
	call _fetch_displacement_by_mod
	call _mov_rm_to_arg1
	call _load_rm_by_mod
	call _mov_res_to_arg2
	call _set_reg_to_arg1
	call _cmp8
	call _mov_reg_to_arg1
	call _mov_res_to_arg2
	call _store_or_assign_arg1_by_mod
	ret

_0x3b_cmp:
	mov byte [_context._is_rm_src],0xff	
	jmp _do01

	call _get_mod_reg_rm
	call _set_scale_index_base
	call _fetch_displacement_by_mod
	call _mov_rm_to_arg1
	call _load_rm_by_mod
	call _mov_res_to_arg2
	call _set_reg_to_arg1
	call _cmp
	call _mov_reg_to_arg1
	call _mov_res_to_arg2
	call _store_or_assign_arg1_by_mod
	ret

_0x3c_cmp:
	add  dword [_rip],1
	mov al,0b00011000
	mov [_context._mod],al
	mov rax,_rax
	mov [_context._rm],rax
	call _set_dflag_as_1byte
	call _fetch
	mov rax,[_context._res]
	mov [_context._imm_val],rax
	ret

_0x3d_cmp:
	add  dword [_rip],1
	;; call __com
	mov al,0b00011000
	mov [_context._mod],al
	mov rax,_rax
	mov [_context._rm],rax
	call _set_dflag
	call _fetch
	mov rax,[_context._res]
	mov [_context._imm_val],rax
	ret
	
_0x3e_prefix_seg_ds:
	add dword [_rip],1
	mov byte [_context._override],0xff
	mov rax,_ds
	mov [_context._override_reg],rax
	jmp _exec_one

_0x3f_bcd_aas:
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
	
