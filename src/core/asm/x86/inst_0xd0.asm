
	default rel
	section .text
	global _0xd0_shift
	global _0xd1_shift
	global _0xd2_shift
	global _0xd3_shift
	global _0xd4_aam
	global _0xd5_aad
	global _0xd6_salc
	global _0xd7_xlat
	
	global _0xd8_float
	global _0xd9_float
	global _0xda_float
	global _0xdb_float
	global _0xdc_float
	global _0xdd_float
	global _0xde_float
	global _0xdf_float

%include "extern_for_inst.asm"

_0xd0_shift:
	add byte [_rip],1
	mov r8, _op_shift_base
	call _set_imm_op_base
	call _get_mod_op_rm
	call _set_dflag_as_1byte
	call _set_scale_index_base
	;; displacement fetch (mod/reg/rm are no longer on the range of an eye)
	call _fetch_displacement_by_mod
	;; shift is always just 1
	mov byte [_context._imm_val],1
	ret

	;; call _fetch
	;; mov rax,[_context._res]
	ret
	
_0xd1_shift:
	;; ret
	add dword [_rip],1
	mov r8, _op_shift_base
	call _set_imm_op_base
	call _get_mod_op_rm
	call _set_dflag
	call _set_scale_index_base
	call _fetch_displacement_by_mod
	mov byte [_context._imm_val],1
	ret
	
_0xd2_shift:
	ret
_0xd3_shift:
	ret
_0xd4_aam:
	ret
_0xd5_aad:
	ret
_0xd6_salc:
	ret
_0xd7_xlat:
	ret
	
_0xd8_float:
	jmp _do01
	ret
_0xd9_float:
	jmp _do01
	ret
_0xda_float:
	jmp _do01
	ret
_0xdb_float:
	add dword [_rip],1
	add dword [_rip],1
	;; mov rax,[_rip]
	;; mov byte al,[rax]
	;; cmp  al,0xe3
	;; je
	ret
_0xdc_float:
	jmp _do01
	ret
_0xdd_float:
	jmp _do01
	ret
_0xde_float:
	jmp _do01
	ret
_0xdf_float:
	ret
