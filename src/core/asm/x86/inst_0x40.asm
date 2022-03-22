
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
	extern _context._opcode
	extern _context._data_prefix
	extern _context._addr_prefix
	
	extern print
	extern _fetch8
	
	extern _exec
	extern _exec_one
	
_0x40_set_rex:
	add dword [_rip],0x01
	mov dl,[_context._opcode]
	mov byte [_context._rex],dl
	jmp _exec_one
	ret
_0x41_set_rex:
	add dword [_rip],0x01
	mov dl,[_context._opcode]
	mov byte [_context._rex],dl
	jmp _exec_one
	ret	
_0x42_set_rex:
	add dword [_rip],0x01
	mov dl,[_context._opcode]
	mov byte [_context._rex],dl
	jmp _exec_one
	ret
	
_0x43_set_rex:
	add dword [_rip],0x01
	mov dl,[_context._opcode]
	mov byte [_context._rex],dl
	jmp _exec_one	
	ret
_0x44_set_rex:
	add dword [_rip],0x01
	mov dl,[_context._opcode]
	mov byte [_context._rex],dl
	jmp _exec_one	
	ret
_0x45_set_rex:
	add dword [_rip],0x01
	mov dl,[_context._opcode]
	mov byte [_context._rex],dl
	jmp _exec_one	
	ret
_0x46_set_rex:
	add dword [_rip],0x01
	mov dl,[_context._opcode]
	mov byte [_context._rex],dl
	jmp _exec_one	
	ret
_0x47_set_rex:
	add dword [_rip],0x01
	mov dl,[_context._opcode]
	mov byte [_context._rex],dl
	jmp _exec_one
	ret
_0x48_set_rex:
	add dword [_rip],0x01
	mov dl,[_context._opcode]
	mov byte [_context._rex],dl
	jmp _exec
	ret
	
_0x49_set_rex:
	add dword [_rip],0x01
	mov dl,[_context._opcode]
	mov byte [_context._rex],dl
	jmp _exec_one	
	ret
_0x4a_set_rex:
	add dword [_rip],0x01
	mov dl,[_context._opcode]
	mov byte [_context._rex],dl
	jmp _exec_one	
	ret
_0x4b_set_rex:
	add dword [_rip],0x01
	mov dl,[_context._opcode]
	mov byte [_context._rex],dl
	jmp _exec_one
	ret
_0x4c_set_rex:
	add dword [_rip],0x01
	mov dl,[_context._opcode]
	mov byte [_context._rex],dl
	jmp _exec_one
	ret
_0x4d_set_rex:
	add dword [_rip],0x01
	mov dl,[_context._opcode]
	mov byte [_context._rex],dl
	jmp _exec_one	
	ret
_0x4e_set_rex:
	add dword [_rip],0x01
	mov dl,[_context._opcode]
	mov byte [_context._rex],dl
	jmp _exec_one	
	ret
_0x4f_set_rex:
	add dword [_rip],0x01
	mov dl,[_context._opcode]
	mov byte [_context._rex],dl
	jmp _exec_one	
	ret

	
