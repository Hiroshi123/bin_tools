
	default rel

	section .text

	global _0x00_add
	
	extern _rip
	extern _gen_code
	extern _get_mod_reg_rm
	extern _set_gen_code_base_by_mod
	
_0x00_add:
	push rbp
	add dword [_rip],0x01
	call _get_mod_reg_rm
	call _set_gen_code_base_by_mod
	;; call set_gen_code_op_by_dflag
	call _gen_code
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

	
