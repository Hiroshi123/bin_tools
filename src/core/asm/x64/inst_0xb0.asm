
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
	extern _rip
	extern _context._arg1
	extern _fetch32
	extern _assign32
	extern _mov_res_to_arg2
	
_0xb0_mov:
	ret
_0xb1_mov:
	ret
_0xb2_mov:
	ret
_0xb3_mov:
	ret
_0xb4_mov:
	ret
_0xb5_mov:
	ret
_0xb6_mov:
	ret
_0xb7_mov:
	ret
_0xb8_mov:

	push rbp
	add byte [_rip],1

	call _fetch32
	add byte [_rip],4
	
	call _mov_res_to_arg2

	mov rax,_rax
	mov [_context._arg1],rax

	call _assign32
	
	pop rbp
	ret
_0xb9_mov:
	ret
_0xba_mov:
	ret
_0xbb_mov:
	ret
_0xbc_mov:
	ret
_0xbd_mov:
	ret
_0xbe_mov:
	ret
_0xbf_mov:
	ret
