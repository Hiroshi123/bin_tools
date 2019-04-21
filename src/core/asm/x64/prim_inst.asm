
	default rel
	section .text
	
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

	
