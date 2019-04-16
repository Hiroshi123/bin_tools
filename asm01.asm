
	default rel
	
	section .data

	global _reg_names

	global _rax
	global _rdx
	global _rcx
	global _rbx
	global _rsi
	global _rdi
	global _r8
	global _r9
	global _r10
	global _r11
	global _r12
	global _r13
	global _r14
	global _r15
	global _eflags	
	global _rip
	global _rsp
	global _rbp

_reg_names:  db "rax rdx rcx rbx rsi rdi r8 r9 r10 r11 r12 r13 r14 r15 eflags rip rsp rbp "

_rax:	dq 8
_rdx:	dq 8
_rcx:	dq 8
_rbx:	dq 8
_rsi:	dq 8
_rdi:	dq 8
_r8:	dq 8
_r9:	dq 8
_r10:	dq 8
_r11:	dq 8
_r12:	dq 8
_r13:	dq 8
_r14:	dq 8
_r15:	dq 8
_eflags:dq 8
_rip:	dq 8
_rsp:	dq 8
_rbp:	dq 8

;;;  when these are called,
;;; you need to set _rdx
;;;  you cannot determine which register will be used
;;; because it is according to the reg & r/m which was provided.
;;;  then
;;; what you need to do is to set %rdi & %rsi,
;;; mov _rsi

context:
._rex: dq 0	
	
mod:
	dq 0
reg:
	dq 0
rm:
	dq 0
dflag:
	db 0
aflag:
	db 0
	
;;; store8(*reg, f(load8(*reg),*rm));

_mod00_load8: dq _load8
._arg1:	dq reg
._arg2:	dq 0
._res:	dq 0

_mod00_f_arg2: dq 0
._arg1:	dq _mod00_load8
._arg2:	dq rm
._res:	dq 0
	
_mod00_store8: dq 0
._arg1:	dq reg
._arg2:	dq _mod00_f_arg2._res
._res:	dq 0

;;; store8(*reg, f(load8(add8(*reg,load8(RIP++)),*rm)));

_mod01_load8: dq _load8
._arg1:	dq _rip
._arg2:	dq 0
._res:	dq 0

_mod01_add8: dq _load8
._arg1:	dq reg
._arg2:	dq _mod01_load8._res
._res:	dq 0

_mod01_load8_: dq _load8
._arg1:	dq _mod01_add8._res
._arg2:	dq 0
._res:	dq 0
	
_mod01_store8_: dq _load8
._arg1:	dq reg
._arg2:	dq rm
._res:	dq 0

;;; store8(*reg, f(load8(add32((uint32_t)*reg,load32(RIP++))),*rm)) 

_mod10_load32: dq _load32
._arg1:	dq _rip
._arg2:	dq 0
._res:	dq 0

_mod10_add32: dq _load8
._arg1:	dq reg
._arg2:	dq _mod10_load32._res
._res:	dq 0

;; _mod10_load32 : dq _load32
;; ._arg1:	dq _mod10_add32._res
;; ._arg2:	dq 0
;; ._res:	dq 0
	
;; _mod10_f_store8_: dq _load8
;; ._arg1:	dq reg
;; ._arg2:	dq rm
;; ._res:	dq 0

;;; store8(*reg, f(load8(add32((uint32_t)*reg,load32(RIP++))),*rm));

;;; assign8(rm,f(*rm,*reg));

_mod11_f: dq _load8
._arg1:	dq rm
._arg2:	dq reg
._res:	dq 0

_mod11_assign8: dq _assign8
._arg1:	dq rm
._arg2:	dq _mod11_f._res
._res:	dq 0

msg: db "Hello, world!", 10
.len: equ $ - msg

section .text
	global _asm_add
	global _check_register
	global _get_rax
	global _set_rax
	global _hello_world
	global _load8
	global _load16
	global _load32
	global _load64
	global _store8
	global _store16
	global _store32
	global _store64
	global _assign8
	global _assign16
	global _assign32
	global _assign64
	global _add8
	global _add16
	global _add32
	global _add64


	
_check_register:
	mov rax,0x100014000
	db 0x02
	db 0x00
	db 0x02
	db 0x00
	db 0x02
	db 0x00
	
	mov [_rbp],rbp
	mov [_rax],rax
	mov rax,rsp
	push rsp
	mov rbp,rsp
	lea rsp,[_rbp]
	;; push %rsp
	push rax
	;; push %rip
	push qword [rbp + 8]
	;; push eflags
	pushf
	push r15
	push r14
	push r13
	push r12
	push r11
	push r10
	push r9
	push r8
	push rdi
	push rsi
	push rbx
	push rdx
	push rcx
	mov rsp,rbp
	pop rsp
	ret

_get_rax:
	mov rax,rax
	ret

_set_rax:
	mov rax,rdi
	ret

_asm_add:
	
	enter 0,0
	mov rax, rdi
    mov ebx, esi
    add eax, ebx
    leave
    ret

_eval:
	mov rax,_rip 
	call rax
	mov rax,rax
	add rax,8
	jmp _eval
	
_hello_world:
	mov rax, 0x2000004 ; write
	mov rdi, 1 ; stdout
	mov rsi, msg
	mov rdx, msg.len
	syscall
	ret

;;;  you have	
_load8:
	mov al,[rdi]
	ret
	
_load16:
	mov ax,[rdi]
	ret
	
_load32:
	mov eax,[rdi]
	ret
	
_load64:
	mov rax,[rdi]
	ret
	
_store8:
	mov [rsi],dil
	ret
	
_store16:
	mov [rsi],di
	ret
	
_store32:
	mov [rdi],edi
	ret
	
_store64:
	mov [rdi],rdi
	ret

;;; assign means setting a value on a register.
;;  assign is identical with store because
;;; in this virual condition, all registers are put on
;;; on a designated memory.
;;; Nevertheless, the distiguinshment is needed for
;;; measuring the count of two operations.
	
_assign8:
	mov [rsi],dil
	ret
	
_assign16:
	mov [rsi],di
	ret
	
_assign32:
	mov [rdi],edi
	ret
	
_assign64:
	mov [rdi],rdi
	ret

;;; addition with signed

_add8:
	mov eax,0
	mov al,sil
	add al,dil
	ret

_add16:
	mov eax,0
	mov ax,si
	add ax,di
	ret
	
_add32:
	mov eax,esi
	add eax,edi
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
	
;;; call has two distinct way to reach out next instruction pointer,
;;; namely, relative call and absolute call
;;; Regard absolute call as special version of relative call
;;; where the offset is zero.
;;: Note generated code will be always absolute for flexibility.

;;; 1st argument :: current instruction pointer
;;; 2nd arugment :: offset to the next instruction pointer(if the call is abs, this is 0)
;;; 3rd argument :: stack pointer which will store current instruction pointer
	
_call:
	;; actually means %rip
	mov rax,rdi
	;; store 1st argument(%rip) onto 3rd argument(%rsp)
	mov [rdx],rdi
	;; diff from current %rip
	add rax,rsi
	call rax
	
;;; ret

_jmp:
	;; actually means %rip
	mov rax,rdi
	;; diff from current %rip
	add rax,rsi
	jmp rax

;;; mod/reg/rm
_0x80:
	add rax,[rdx]
	call rax

;;; mod_reg_rm

_get_mod_reg_rm:
	mov al, [_rip]
	mov bl,al
	and bl, 0b11000000
	mov [mod],bl

	mov al,[_rip]
	and bl, 0b00111000
	mov [reg],bl

	mov al,[context._rex]
	and al, 0b00001000

	call _get_aflag
	call _set_dflag

	;; reg_base + bl(reg)
	
	;; mov al,[_rex]
	;; mov [dflag] al
	
	;; mov al,[_rip]	
	;; mov [aflag] al

_set_dflag:
	;; dflags
	mov al,0x8
	mov [dflag], al
	mov al,0x6
	
	mov al,0x4
	mov [dflag], al
	mov al,0x4
	
	mov al,0x2
	mov [dflag], al
	mov al,0x2	
	
_get_aflag:
	;; aflags
	mov al,0x8
	mov [aflag], al
	mov al,0x4
	mov [aflag], al
	mov al,0x2
	mov [aflag], al

_get_reg_base:
	mov rax,_rax
	mov rax,_r8

_get_rm_base:	
	mov rax,_r8
	mov rax,_r8

;;; _0x00

;;  _0x00:
	
	
;; _0x4_0f:
;; 	mov 0x01 mov 
	

;;; jmp instruction
	
_0x70_jo:
	call _set_eflags	
	jo _set_rip
	ret
	
_0x71_jno:
	call _set_eflags
	jno _set_rip
	ret

_0x72_jnae:
	call _set_eflags
	jnae _set_rip
	ret

_0x73_jnc:
	call _set_eflags
	jnc _set_rip
	ret

_0x74_je:
	call _set_eflags
	je _set_rip
	ret
	
_0x75_jne:
	call _set_eflags
	jne _set_rip
	ret

_0x76_jna:
	call _set_eflags
	jna _set_rip
	ret

_0x77_jnbe:
	call _set_eflags
	jnbe _set_rip
	ret

_0x78_js:
	call _set_eflags
	js _set_rip
	ret

_0x79_jns:
	call _set_eflags
	jns _set_rip
	ret

_0x7a_jpe:
	call _set_eflags
	jpe _set_rip
	ret

_0x7b_jpo:
	call _set_eflags
	jpo _set_rip
	ret

_0x7c_jnl:
	call _set_eflags
	jnl _set_rip
	ret

_0x7d_jng:
	call _set_eflags
	jng _set_rip
	ret

_0x7e_jng:
	call _set_eflags
	jng _set_rip
	ret

_0x7f_jnle:
	call _set_eflags
	jnle _set_rip
	ret

_set_eflags:
	push qword [_eflags]
	popf
	ret
	
_set_rip:
	add [_rip],rdi
	ret

;;; jmp instruction

_0x80_jo:
	call _set_eflags	
	jo _set_rip
	ret
	
_0x81_jno:
	call _set_eflags
	jno _set_rip
	ret

_0x82_jnae:
	call _set_eflags
	jnae _set_rip
	ret

_0x83_jnc:
	call _set_eflags
	jnc _set_rip
	ret

_0x84_je:
	call _set_eflags
	je _set_rip
	ret
	
_0x85_jne:
	call _set_eflags
	jne _set_rip
	ret

_0x86_jna:
	call _set_eflags
	jna _set_rip
	ret

_0x87_jnbe:
	call _set_eflags
	jnbe _set_rip
	ret

_0x88_js:
	call _set_eflags
	js _set_rip
	ret

_0x89_jns:
	call _set_eflags
	jns _set_rip
	ret

_0x8a_jpe:
	call _set_eflags
	jpe _set_rip
	ret

_0x8b_jpo:
	call _set_eflags
	jpo _set_rip
	ret

_0x8c_jnl:
	call _set_eflags
	jnl _set_rip
	ret

_0x8d_jng:
	call _set_eflags
	jng _set_rip
	ret

_0x8e_jng:
	call _set_eflags
	jng _set_rip
	ret

_0x8f_jnle:
	call _set_eflags
	jnle _set_rip
	ret
	
