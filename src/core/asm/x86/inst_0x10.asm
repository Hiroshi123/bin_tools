
	default rel

	section .text

	global _0x10_adc
	global _0x11_adc
	global _0x12_adc
	global _0x13_adc
	global _0x14_adc
	global _0x15_adc
	
	;; 0x18 sbb
	global _0x18_sbb
	global _0x19_sbb
	global _0x1a_sbb
	global _0x1b_sbb
	global _0x1c_sbb
	global _0x1d_sbb
	global _0x1f
	
%include "extern_for_inst.asm"
	
_0x10_adc:
	push rbp
	add byte [_rip],1
	;; scale_index_base	
	mov qword [_rax],2
	mov qword [_rcx],3
	
	call _get_mod_reg_rm
	mov r8,[_context._reg]
	call print
	mov rax,_rcx
	;; [_context._reg]
	call _set_scale_index_base
	call _load_arg1_by_mod
	call _set_reg_to_arg2
	call _add8
	call _mov_rm_to_arg1
	call _mov_res_to_arg2
	call _store_or_assign_arg1_by_mod	

	mov r8,[_rax]
	call print

	pop rbp
	ret
	
_0x11_adc:
	push rbp
	add byte [_rip],1	
	call _get_mod_reg_rm
	add byte [_rip],1
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

_0x12_adc:
	ret
_0x13_adc:
	ret
_0x14_adc:
	ret
_0x15_adc:
	ret

_0x18_sbb:
	mov r15b,0xff
_0x19_sbb:
	jmp _do01
_0x1a_sbb:
	mov r15b,0xff
_0x1b_sbb:
	mov byte [_context._is_rm_src],0xff
	jmp _do01

;; _0x18_sbb:	
;; 	add byte [_rip],1
;; 	call _get_mod_reg_rm
;; 	call _set_scale_index_base
;; 	call _fetch_displacement_by_mod
;; 	ret
;; _0x19_sbb:
;; 	add byte [_rip],1
;; 	call _get_mod_reg_rm
;; 	call _set_scale_index_base
;; 	call _fetch_displacement_by_mod
;; 	ret
;; _0x1a_sbb:
;; 	ret
;; _0x1b_sbb:
;; 	ret

_0x1c_sbb:
	call _set_dflag_as_1byte
	jmp _do02
_0x1d_sbb:
	call _set_dflag
	jmp _do02	

_0x1f:
	add dword [_rip],0x01
	mov rax,_extend_opcode_table
	mov [_context._opcode_table],rax
	jmp _exec_one
	