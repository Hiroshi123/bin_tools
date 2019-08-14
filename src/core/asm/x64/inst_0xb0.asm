
	default rel
	
	section .text
		
	global _0xb0_mov	
	global _0xb1_mov	
	global _0xb2_mov	
	global _0xb3_mov	
	global _0xb4_mov	
	global _0xb5_mov	
	global _0xb6_mov	
	global _0xb7_mov
	global _0xb8_mov	
	global _0xb9_mov	
	global _0xba_mov	
	global _0xbb_mov	
	global _0xbc_mov	
	global _0xbd_mov	
	global _0xbe_mov	
	global _0xbf_mov
	
	extern _rax
	extern _rdx

	extern _rip
	extern _context._arg1
	extern _fetch32
	extern _assign
	extern _assign32
	extern _mov_res_to_arg2
	extern _set_dflag
	extern _set_dflag_as_1byte
	extern _fetch

	extern _context._rex
	extern _context._opcode
	extern _r8
	extern _select_reg
	extern print
	
_0xb0_mov:
	push rbp
	call _set_dflag_as_1byte
	call _gen_0xb0
	pop rbp
	ret
_0xb1_mov:
	push rbp
	call _set_dflag_as_1byte
	call _gen_0xb0
	pop rbp
	ret
_0xb2_mov:
	push rbp
	call _set_dflag_as_1byte
	call _gen_0xb0
	pop rbp
	ret
_0xb3_mov:
	push rbp
	call _set_dflag_as_1byte
	call _gen_0xb0
	pop rbp	
	ret
_0xb4_mov:
	push rbp
	call _set_dflag_as_1byte
	call _gen_0xb0
	pop rbp
	ret
_0xb5_mov:
	push rbp
	call _set_dflag_as_1byte
	call _gen_0xb0
	pop rbp	
	ret
_0xb6_mov:
	push rbp
	call _set_dflag_as_1byte
	call _gen_0xb0
	pop rbp
	ret
_0xb7_mov:
	push rbp
	call _set_dflag_as_1byte
	call _gen_0xb0
	pop rbp
	ret
_0xb8_mov:
	push rbp
	call _set_dflag
	call _gen_0xb0
	pop rbp
	ret
_0xb9_mov:
	push rbp
	call _set_dflag
	call _gen_0xb0
	pop rbp
	ret
_0xba_mov:
	push rbp
	call _set_dflag
	call _gen_0xb0
	pop rbp
	ret
_0xbb_mov:
	push rbp
	call _set_dflag
	call _gen_0xb0
	pop rbp
	ret
_0xbc_mov:
	push rbp
	call _set_dflag
	call _gen_0xb0
	pop rbp
	ret
_0xbd_mov:
	push rbp
	call _set_dflag	
	call _gen_0xb0
	pop rbp
	ret
_0xbe_mov:
	push rbp
	call _set_dflag	
	call _gen_0xb0
	pop rbp	
	ret
_0xbf_mov:
	call _set_dflag
	call _gen_0xb0
	ret

_gen_0xb0:
	push rbp
	add dword [_rip],1
	call _fetch
	call _mov_res_to_arg2
	call _select_reg
	mov [_context._arg1],r8
	call _assign
	pop rbp	
	ret
	
