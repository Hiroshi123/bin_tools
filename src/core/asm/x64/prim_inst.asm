
	default rel
	section .text

	global _fetch
	
	extern _load_base
	extern _store_base
	extern _fetch_base
	extern _add_base
	extern _sub_base
	extern _assign_base

	global _fetch
	global _fetch8
	global _fetch16
	global _fetch32
	global _fetch64

	global _load
	global _load8
	global _load16
	global _load32
	global _load64

	global _store
	global _store8
	global _store16
	global _store32
	global _store64

	global _assign
	global _assign8
	global _assign16
	global _assign32
	global _assign64

	global _add
	global _add8
	global _add16
	global _add32
	global _add64

	global _sub
	global _sub8
	global _sub16
	global _sub32
	global _sub64

	global _and
	global _or
	global _xor
	global _cmp
	
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

	extern _eflag
	extern _rip
	
	extern _context._dflag
	extern _context._rex
	extern _context._arg1
	extern _context._arg2
	extern _context._res

	extern print
	extern get_diff_host_guest_addr
	extern _get_host_addr_from_guest
	
_fetch:
	push rbp
	mov ax,0
	mov al,[_context._dflag]
	lea rdx,[_fetch_base]
	add dx,ax
	call [rdx]
	pop rbp
	ret

_fetch8:
	mov qword [_context._res],0x00
	mov rax,[_rip]
	mov al,[rax]
	mov [_context._res],al
	ret
_fetch16:
	mov qword [_context._res],0x00
	mov rax,[_rip]
	mov ax,[rax]
	ret
_fetch32:
	mov qword [_context._res],0x00	
	mov rax,[_rip]
	mov eax,[rax]
	mov [_context._res],eax	
	ret
_fetch64:
	ret
	
;;;  you have	
_load:
	push rbp
	mov ax,0
	mov al,[_context._dflag]
	lea rdx,[_load_base]
	add dx,ax
	call [rdx]
	pop rbp
	ret
	
_load8:
	mov rax,[_context._arg1]
	mov rax,[rax]
	mov r8,[_context._arg1]
	call print
	ret
	
_load16:
	ret
	
_load32:
	ret
	
_load64:
	push rbp
	mov rax,[_context._arg1]
	;; this is util function which will 
	call _get_host_addr_from_guest
	mov rax,[rax]
	;; add rax,[_rsp]
	mov [_context._res],rax
	mov r8,[_context._res]
	call print
	pop rbp
	ret

_store:
	push rbp
	mov ax,0
	mov al,[_context._dflag]
	lea rdx,[_store_base]
	add dx,ax
	call [rdx]
	pop rbp
	ret
	
_store8:
	push rbp
	mov rax,[_context._arg1]
	;; 
	mov rdi,0
	mov rsi,rax
	call get_diff_host_guest_addr
	add rax,[_context._arg1]	
	;; 
	mov rax,[_context._arg2]
	pop rbp
	ret
	
_store16:
	ret
	
_store32:
	ret
	
_store64:
	push rbp
	mov rax,[_context._arg1]
	call _get_host_addr_from_guest
	mov rdx,[_context._arg2]
	mov [rax],rdx
	pop rbp
	ret

;;; assign means setting a value on a register.
;;  assign is identical with store because
;;; in this virual condition, all registers are put on
;;; on a designated memory.
;;; Nevertheless, the distiguinshment is needed for
;;; measuring the count of two operations.

_assign:
	push rbp
	mov ax,0
;;; 
	mov r8,0x88
	call print

	lea rdx,[_assign_base]
	mov al,[_context._dflag]
	add dx,ax
	call [rdx]
	pop rbp
	ret
	
_assign8:
	push rbp
	mov rax,[_context._arg1]
	mov cl,[_context._arg2]
	mov [rax],cl
	pop rbp
	ret
	
_assign16:
	ret
	
_assign32:
	push rbp
	mov r8,[_context._arg1]
	call print
	mov r8,[_context._arg2]
	call print
	mov rax,[_context._arg1]
	mov ecx,[_context._arg2]
	mov [rax],ecx
	pop rbp
	ret
	
_assign64:
	push rbp
	mov rax,[_context._arg1]
	mov rcx,[_context._arg2]
	mov [rax],rcx
	pop rbp
	ret

;;; addition with signed

_add:
	push rbp
	mov ax,0
	mov al,[_context._dflag]
	lea rdx,[_add_base]
	add dx,ax
	call [rdx]
	pop rbp
	ret
	
_add8:
	mov qword [_context._res],0
	mov al,[_context._arg1]
	mov bl,[_context._arg2]
	add al,bl
	mov [_context._res],ax
	;; [_context._res]
	mov r8,[_context._res]
	call print
	ret

_add16:
	mov qword [_context._res],0
	mov al,[_context._arg1+6]
	mov bl,[_context._arg2+6]
	add al,bl
	mov [_context._res],ax
	ret
	
_add32:
	mov rax,[_context._arg1]
	mov edx,[_context._arg2]
	add eax,edx
	mov [_context._res],rax
	ret
	
_add64:
	mov rax,[_context._arg1]
	mov rdx,[_context._arg2]
	add rax,rdx
	mov [_context._res],rax
	ret

_sub:
	push rbp
	mov r8,0x77
	call print

	mov ax,0
	mov al,[_context._dflag]
	lea rdx,[_sub_base]
	add dx,ax
	call [rdx]
	pop rbp
	ret

_sub8:
	ret

_sub16:
	ret
	
_sub32:
	ret
	
_sub64:
	push rbp
	mov r8,0x77
	call print
	mov rax,[_context._arg1]
	mov rdx,[_context._arg2]
	sub rax,rdx
	mov [_context._res],rax
	pop rbp
	ret


_and:
	ret

_or:
	ret

_xor:
	ret

_cmp:
	ret


