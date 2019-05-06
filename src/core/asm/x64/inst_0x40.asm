
	section .text
	default rel
	global _0x40_set_rex
	global _0x41_set_rex
	global _0x42_set_rex
	global _0x43_set_rex
	global _0x44_set_rex
	global _0x45_set_rex
	global _0x46_set_rex
	global _0x47_set_rex
	global _0x48_set_rex
	global _0x49_set_rex
	global _0x4a_set_rex
	global _0x4b_set_rex
	global _0x4c_set_rex
	global _0x4d_set_rex
	global _0x4e_set_rex
	global _0x4f_set_rex

	extern _rip
	extern _context
	extern _context._rex
	extern _context._res	
	extern _context._data_prefix
	extern _context._addr_prefix
	
	extern print
	extern _fetch8
	
	extern _exec_one
	
_0x40_set_rex:
	push rbp
	;; mov rax,[_rip]
	call _fetch8
	mov al,[_context._res]
	and al,0x0f
	pop rbp
	;; mov byte [_context._rex],al
	ret
	
_0x41_set_rex:
	mov r8,0x41
	call print
	call _fetch8
	mov dl,[_context._res]
	mov byte [_context._rex],dl
	jmp _exec_one	
	ret
	
_0x42_set_rex:
	ret
_0x43_set_rex:
	ret
_0x44_set_rex:
	ret
_0x45_set_rex:
	ret
_0x46_set_rex:
	ret
_0x47_set_rex:
	ret
_0x48_set_rex:
	call _fetch8
	mov dl,[_context._res]
	mov byte [_context._rex],dl
	jmp _exec_one
	;; mov [_context._rex],[_rip]	
	ret
_0x49_set_rex:
	ret
_0x4a_set_rex:
	ret
_0x4b_set_rex:
	ret
_0x4c_set_rex:
	ret
_0x4d_set_rex:
	ret
_0x4e_set_rex:
	ret
_0x4f_set_rex:
	ret

	
