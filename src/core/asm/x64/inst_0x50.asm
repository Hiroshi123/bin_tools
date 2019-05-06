
	default rel
	section .text
	global _gen_push
	global _0x50_push
	global _0x51_push
	global _0x52_push
	global _0x53_push
	global _0x54_push
	global _0x55_push
	global _0x56_push
	global _0x57_push
	global _gen_pop
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
	extern _context._opcode
	extern _context._rex	

	extern _store64
	extern _assign64
	extern _add64
	extern _sub64
	extern _load64
	extern _mov_res_to_arg2

	extern _context._internal_arg1
	extern _context._internal_arg2
	
	extern get_diff_host_guest_addr

	extern _rip
	extern _r8
	
_0x50_push:
	push rbp
	add byte [_rip],1
	call _select_reg
	mov r8,[r8]
	mov [_context._internal_arg1],r8
	call _gen_push
	pop rbp
	ret
	
_0x51_push:
	push rbp
	add byte [_rip],1
	call _select_reg
	mov r8,[r8]
	mov [_context._internal_arg1],r8
	call _gen_push
	pop rbp
	ret

_0x52_push:
	push rbp
	add byte [_rip],1
	call _select_reg
	mov r8,[r8]
	mov [_context._internal_arg1],rax
	call _gen_push
	pop rbp
	ret

_0x53_push:
	push rbp
	add byte [_rip],1
	call _select_reg
	mov r8,[r8]
	mov [_context._internal_arg1],r8
	call _gen_push
	pop rbp
	ret
	
_0x54_push:
	push rbp
	add byte [_rip],1
	mov r8,0x54
	call print

	;; if rex_prefix (rex_b) is set, then
	;; it means you must use registers of r8-r15.
	;; [_context._opcode] will choose kind of registers among the given 8.
	call _select_reg
	mov r8,[r8]
	mov [_context._internal_arg1],r8
	call _gen_push
	pop rbp
	ret

_0x55_push:
	
	push rbp
	add byte [_rip],1
	call _select_reg
	mov r8,[r8]
	mov [_context._internal_arg1],r8
	call _gen_push
	pop rbp
	ret
	
_0x56_push:
	push rbp
	add byte [_rip],1
	call _select_reg
	mov r8,[r8]
	mov [_context._internal_arg1],r8
	call _gen_push
	pop rbp
	ret
	
_0x57_push:
	push rbp
	add byte [_rip],1
	call _select_reg
	mov r8,[r8]
	mov [_context._internal_arg1],r8
	call _gen_push
	pop rbp
	ret

;;; ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	
_0x58_pop:
	push rbp
	add byte [_rip],1
	call _select_reg
	mov [_context._internal_arg2],r8
	call _gen_pop
	pop rbp
	ret
	
_0x59_pop:
	push rbp
	add byte [_rip],1
	call _select_reg
	mov [_context._internal_arg2],r8
	call _gen_pop
	pop rbp
	ret
	
_0x5a_pop:
	push rbp
	add byte [_rip],1
	call _select_reg
	mov [_context._internal_arg2],r8
	call _gen_pop
	pop rbp
	ret

_0x5b_pop:
	push rbp
	add byte [_rip],1
	call _select_reg
	mov [_context._internal_arg2],r8
	call _gen_pop
	pop rbp
	ret

_0x5c_pop:
	push rbp
	add byte [_rip],1
	call _select_reg
	mov [_context._internal_arg2],r8
	call _gen_pop
	pop rbp
	ret

_0x5d_pop:
	push rbp
	add byte [_rip],1
	call _select_reg
	mov [_context._internal_arg2],r8
	call _gen_pop
	pop rbp
	ret
	
_0x5e_pop:
	push rbp
	add byte [_rip],1
	call _select_reg
	mov [_context._internal_arg2],r8
	call _gen_pop
	pop rbp
	ret

_0x5f_pop:
	push rbp
	add byte [_rip],1
	call _select_reg
	mov [_context._internal_arg2],r8
	call _gen_pop
	pop rbp
	ret
	
;;;

_select_reg:
	
	lea r12,[_select_reg.done1]
	mov r8b,[_context._rex]
	and r8b,0b00000001
	cmp r8b,0b00000001
	jne _set_base_reg
	jmp _set_base_reg_ex
.done1:
	;; 2. get kind of register
	mov r9,0x00
	mov r9b,[_context._opcode]
	;; only rightest 3bit is needed for register selection.
	and r9b,0b00000111
	;; you need to shift left 3times considering size of each register.
	shl r9b,3
	add r8b,r9b
	ret
	
_set_base_reg:	
	mov r8,_rax
	jmp r12
	
_set_base_reg_ex:
	mov r8,_r8
	jmp r12
	
;;; this is a general implementation of push.
;;; it produces 3 primitive instructions.

;;; 1. let rsp up 8 bytes step(subtract 0x08 from original value)
;;; 2. assign the value on rsp.
;;; 3. store the value which is given on [_context._internal_arg1].

_gen_push:
	
	push rbp
		
	mov rax,[_rsp]
	mov qword [_context._arg1],rax
	mov qword [_context._arg2],0x08
	call _sub64
	
	mov rax,_rsp
	mov [_context._arg1],rax
	call _mov_res_to_arg2
	call _assign64
	
	mov rax,[_context._internal_arg1]
	mov [_context._arg2],rax
	mov rax,[_rsp]
	mov [_context._arg1],rax
	call _store64
	
	pop rbp
	ret
	
;;; 1. load a value of [_rsp]
;;; 2. assign a value to given memory[_context._internal_arg1]
;;; 3. add rsp
;;; 4. assign rsp

_gen_pop:
	
	push rbp
	mov rax,[_rsp]
	mov [_context._arg1],rax
	call _load64

	mov rax,[_context._internal_arg2]
	mov [_context._arg1],rax
	call _mov_res_to_arg2
	call _assign64

	mov rax,[_rsp]
	mov qword [_context._arg1],rax
	mov qword [_context._arg2],0x08
	call _add64
	
	mov rax,_rsp
	mov [_context._arg1],rax
	call _mov_res_to_arg2
	call _assign64

	pop rbp	
	ret
	