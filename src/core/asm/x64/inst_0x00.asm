
	default rel

	section .text

	global _0x00_add
	
	extern _rip

	extern _gen_code
	extern _get_mod_reg_rm

	extern _set_scale_index_base
	extern _set_gen_code_base_by_mod
	extern _set_immidiate
	extern _set_gen_code_op_by_dflag

	extern _load_arg1_by_mod
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
	
	
_0x00_add:
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
	
_0x01_add:
	ret

_0x02_add:
	ret
_0x03_add:
	ret
_0x04_add:
	ret
_0x05_add:
	ret

_0x08_sub:
	ret

	
