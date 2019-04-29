
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
	extern _rbp
	extern _rsp
	extern print
	
	extern _context._arg1
	extern _context._arg2
	
	extern _store64
	extern _assign64
	extern _sub64
	extern _mov_res_to_arg2
	
	extern get_diff_host_guest_addr
	
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
	push rbp
	mov rax,[_rbp]
	mov [_context._arg2],rax
	mov rax,[_rsp]
	mov [_context._arg1],rax
	call _store64
	
	mov r8,[_rsp]
	call print
	;; mov rax,[_rsp]
	;; mov r8,[rax]
	;; call print	
	;; sub byte [_rsp],0x08

	mov rax,[_rsp]
	mov qword [_context._arg1],rax
	mov qword [_context._arg2],0x08
	call _sub64
	;; 3rd 
	mov rax,_rsp
	mov [_context._arg1],rax
	;; 
	call _mov_res_to_arg2
	call _assign64
	
	mov r8,[_rsp]
	call print
	pop rbp
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

	
