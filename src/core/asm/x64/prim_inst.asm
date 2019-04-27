
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
	extern _context._arg1
	extern _context._arg2
	extern _context._res

	extern print
	
_fetch:
	push rbp
	mov ax,0
	mov al,[_context._dflag]
	mov al,[_fetch_base]
	add dx,ax
	call [rdx]
	pop rbp
	ret

_fetch8:
	mov rax,[_rip]
	mov al,[rax]
	mov [_context._res],al
	ret
_fetch16:
	mov rax,[_rip]
	mov ax,[rax]
	ret
_fetch32:
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
	mov rax,[_context._res]
	mov [_context._arg1],rax
	pop rbp
	ret
	
_store16:
	ret
	
_store32:
	ret
	
_store64:
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
	mov al,[_context._dflag]
	lea rdx,[_assign_base]
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
	ret

;;; addition with signed

_add:
	push rbp
	mov ax,0
	mov al,[_context._dflag]
	mov al,[_add_base]
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
	mov edx,[rax+4]
	mov ecx,[rax+8]
	add edx,ecx
	mov [rax+12],edx
	ret
	
_add64:
	mov rax,rsi
	add rax,rdi
	ret

_sub8:
	mov al,sil
	sub al,dil
	ret

_sub16:
	mov ax,si
	sub ax,di
	ret
	
_sub32:
	mov eax,esi
	sub eax,edi
	ret
	
_sub64:
	mov rax,rsi
	sub rax,rdi
	ret

	
