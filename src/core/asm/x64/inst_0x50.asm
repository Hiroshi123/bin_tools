
	default rel
	section .text
	global _0x50_push
	global _0x51_push
	global _0x52_push
	global _0x53_push
	global _0x54_push
	global _0x55_push
	global _0x56_push
	global _0x57_push
	global _0x58_pop
	global _0x59_pop
	global _0x5a_pop
	global _0x5b_pop
	global _0x5c_pop
	global _0x5d_pop
	global _0x5e_pop
	global _0x5f_pop
	
	extern _rax
	extern _rsp
	extern print
	
_0x50_push:
	mov rax,[_rax]
	mov rbx,[_rsp]
	mov [rbx],rax
	sub dword [_rsp],8
	;; mov r8,[_rsp]
	mov r8,0x33
	call print
	ret
	
_0x51_push:
	ret
_0x52_push:
	ret
_0x53_push:
	ret
_0x54_push:
	ret
_0x55_push:
	ret
_0x56_push:
	ret
_0x57_push:
	ret
_0x58_pop:
	mov rax,[_rsp]
	mov rbx,[rax]
	mov r8,rbx
	call print
	mov [_rax],rbx
	add dword [_rsp],8
	ret
_0x59_pop:
	ret
_0x5a_pop:
	ret
_0x5b_pop:
	ret
_0x5c_pop:
	ret
_0x5d_pop:
	ret
_0x5e_pop:
	ret
_0x5f_pop:
	ret

	
