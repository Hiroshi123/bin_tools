
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
	extern _context._dflag
	
	extern _store
	extern _assign
	extern _add
	extern _sub
	extern _load
	extern _mov_res_to_arg2

	extern _context._internal_arg1
	extern _context._internal_arg2
	
	extern get_diff_host_guest_addr

	extern _rip
	extern _r8
	extern _select_reg

	extern _set_dflag
	extern _reset_dflag_when_x64

	extern _dflag_len
	
_0x50_push:
	;; push rbp
	add dword [_rip],1
	call _select_reg
	ret
	
	mov r8,[r8]
	
	mov [_context._internal_arg1],r8
	call _gen_push
	;; pop rbp
	ret
	
_0x51_push:

	add dword [_rip],1
	call _select_reg
	ret
	
	mov r8,[r8]
	mov [_context._internal_arg1],r8
	call _gen_push
	pop rbp
	ret

_0x52_push:

	add dword [_rip],1
	call _select_reg
	ret
	
	mov r8,[r8]
	mov [_context._internal_arg1],rax
	call _gen_push
	ret

_0x53_push:
	add dword [_rip],1
	call _select_reg
	ret
	mov r8,[r8]
	mov [_context._internal_arg1],r8
	call _gen_push
	ret
	
_0x54_push:

	add dword [_rip],1
	call _select_reg
	ret
	
	;; if rex_prefix (rex_b) is set, then
	;; it means you must use registers of r8-r15.
	;; [_context._opcode] will choose kind of registers among the given 8.
	mov r8,[r8]
	mov [_context._internal_arg1],r8
	call _gen_push
	pop rbp
	ret

_0x55_push:
	
	add dword [_rip],1
	call _select_reg
	ret

	mov r8,[r8]
	mov [_context._internal_arg1],r8
	call _gen_push
	pop rbp
	ret
	
_0x56_push:

	add dword [_rip],1
	call _select_reg
	ret

	mov r8,[r8]
	mov [_context._internal_arg1],r8
	call _gen_push
	pop rbp
	ret
	
_0x57_push:

	;; push rbp
	add dword [_rip],1
	call _select_reg
	ret
	
	mov r8,[r8]
	mov [_context._internal_arg1],r8
	call _gen_push
	pop rbp
	ret

;;; ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	
_0x58_pop:
	;; push rbp
	add dword [_rip],1
	call _select_reg
	ret
	
	mov [_context._internal_arg2],r8
	call _gen_pop
	pop rbp
	ret
	
_0x59_pop:

	;; push rbp
	add dword [_rip],1
	call _select_reg
	ret
	
	mov [_context._internal_arg2],r8
	call _gen_pop
	pop rbp
	ret
	
_0x5a_pop:
	;; push rbp
	add dword [_rip],1
	call _select_reg
	ret
	mov [_context._internal_arg2],r8
	call _gen_pop
	pop rbp
	ret

_0x5b_pop:
	;; push rbp
	add dword [_rip],1
	call _select_reg
	ret

	mov [_context._internal_arg2],r8
	call _gen_pop
	pop rbp
	ret

_0x5c_pop:
	;; push rbp
	add dword [_rip],1
	call _select_reg
	ret
	
	mov [_context._internal_arg2],r8
	call _gen_pop
	pop rbp
	ret

_0x5d_pop:
	;; push rbp
	add dword [_rip],1
	call _select_reg
	ret
	mov [_context._internal_arg2],r8
	call _gen_pop
	pop rbp
	ret
	
_0x5e_pop:
	;; push rbp
	add dword [_rip],1
	call _select_reg
	ret

	mov [_context._internal_arg2],r8
	call _gen_pop
	pop rbp
	ret

_0x5f_pop:
	;; push rbp
	add dword [_rip],1
	call _select_reg
	ret
	mov [_context._internal_arg2],r8
	call _gen_pop
	pop rbp
	ret
	
;;;
	
;;; this is a general implementation of push.
;;; it produces 3 primitive instructions.

;;; 1. let rsp up 8 bytes step(subtract 0x08 from original value)
;;; 2. assign the value on rsp.
;;; 3. store the value which is given on [_context._internal_arg1].

;;; Note push is processor dependent operation.
;;; simple 0x50 stores 4bytes on x86, and 8bytes on x86-64 no matter what
;;; values are set on rex prefix.
;;; Both of them, takes data_prefix account of its calculation, and let the length
;;; 16bit if it is set.
;;; It means you need to read processor_mode when performing below.
_gen_push:
	
	push rbp
	call _set_dflag
	call _reset_dflag_when_x64	

	mov rax,[_rsp]
	mov qword [_context._arg1],rax
	
	lea rax,[_dflag_len]
	mov dx,0x00
	mov dl,[_context._dflag]
	add ax,dx
	mov rax,[rax]
	mov qword [_context._arg2],rax

	call _sub
	
	mov rax,_rsp
	mov [_context._arg1],rax
	call _mov_res_to_arg2
	call _assign
	
	mov rax,[_context._internal_arg1]
	mov [_context._arg2],rax
	mov rax,[_rsp]
	mov [_context._arg1],rax
	call _store
	
	pop rbp
	ret
	
;;; 1. load a value of [_rsp]
;;; 2. assign a value to given memory[_context._internal_arg1]
;;; 3. add rsp
;;; 4. assign rsp

_gen_pop:
	
	push rbp
	call _set_dflag
	call _reset_dflag_when_x64	

	mov rax,[_rsp]
	mov [_context._arg1],rax
	call _load

	mov rax,[_context._internal_arg2]
	mov [_context._arg1],rax
	call _mov_res_to_arg2
	call _assign

	mov rax,[_rsp]
	mov qword [_context._arg1],rax
	;; mov al,[_context._dflag]
	;; shr al,0x03
	;; mov qword [_context._arg2],0x00
	;; mov [_context._arg2],al

	lea rax,[_dflag_len]
	mov dx,0x00
	mov dl,[_context._dflag]
	add ax,dx
	mov rax,[rax]
	mov qword [_context._arg2],rax
	
	call _add
	
	mov rax,_rsp
	mov [_context._arg1],rax
	call _mov_res_to_arg2
	call _assign

	pop rbp	
	ret
	
