
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

	extern _fetch_displacement_by_mod
	
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
	ret
_0x19_sbb:
	ret
_0x1a_sbb:
	ret
_0x1b_sbb:
	ret
_0x1c_sbb:
	ret
_0x1d_sbb:
	ret
