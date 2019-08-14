
	section .text
	default rel

	global _0x20_get_local
	global _0x21_set_local
	global _0x41_i32_const
	global _0x42_i64_const
	global _0x43_f32_const
	global _0x44_f64_const

	extern _inp
	extern _outp
	extern _codep
	extern _stack_depth	
	extern print
	extern _localv_base_reg
	
;;; load the variable which was specified on the subsequent index on top of stack.
;;; stuff needs to be referenced.
;;; 1. stack index (how many numbers of register had already been stacked)
;;; 2. index of memory(rbx offset) reprenseted as displacement.

;;; in x86, mostly, mod == 0x01(10),
;;; displacement, reg is fixed. rm is chosen by stack index.
	
;;; 
_0x20_get_local:

	mov r8,0x20
	call print

;;; ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	mov  al,0x48
	call _write_on_codep
	mov al,0x89
	call _write_on_codep

;;; ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	;; fix mod as 0x01 in this moment.
	mov al,0x01
	shl al,0x06	
	;; reg == 
	mov byte cl,[_localv_base_reg]
	shl cl,0x03	
	;; rm == stack depth(rax,rcx,rdx,rbx,rdi,rsi,..)
	mov dl,0
	mov dl,[_stack_depth]	
	;; composite above three.
	or al,cl
	or al,dl
	call _write_on_codep
	;; 2,3,3
	;; mod(0b01/reg is defined by next value of /)
;;; ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	
	;; arguents needs to be retrieved...
	;; which is local index.
	mov rcx,[_inp]
	mov byte al,[rcx]
	shl al,0x03
	call _write_on_codep

;;; ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

	;; increment stack_depth as you should add
	add dword [_stack_depth],0x01	
	;; increments current exmining pointer for arugment.
	add dword [_inp],0x1
	ret

;;; only 1byte
_write_on_codep:
	mov r9,[_codep]
	mov byte [r9],al
	add byte [_codep],0x01
	ret

_write_on_codep_2byte:
	mov r9,[_codep]
	mov word [r9],ax
	add byte [_codep],0x02
	ret

_write_on_codep_4byte:
	mov r9,[_codep]
	mov dword [r9],eax
	add byte [_codep],0x04
	ret

_write_on_codep_8byte:
	mov r9,[_codep]
	mov [r9],rax
	add byte [_codep],0x08
	ret

_0x21_set_local:
	mov r8,0x28
	call print
	ret

;;; most straight forward conversion is
_0x41_i32_const:
	mov r8,0x41
	call print
	
;;; ;;;;;
	mov r9,0xb8
	call _write_on_codep
	mov r9,0x0
	call _write_on_codep_4byte
	mov rax,[_codep]
;;; ;;;;;;;;;;;;;;;;;;;;;;;;;
	;; increment stack_depth as you should add
	add dword [_stack_depth],0x01	
	;; increments current exmining pointer for arugment.
	add dword [_inp],0x1
	ret

_0x42_i64_const:
	ret
_0x43_f32_const:
	ret
_0x44_f64_const:	
	ret
	
	
;;; 0x48,0x89,0xc0
