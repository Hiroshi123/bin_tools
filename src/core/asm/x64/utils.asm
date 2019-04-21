
	default rel
	section .data
	
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

	extern _rip


reg_size8:
	db 0x30,0x78,0x30,0x30,0x30,0x30,0x30,0x30,0x30,0x30,0x0a
	.len: equ $ - reg_size8

test_sub_addr:
	dq 0
	
	section .text

	global print
	global _check_register
	global _check_register
	
	extern _test_on_real_cpu
	
print:
	push rbp
	mov r10,reg_size8+0x0a
	
	mov r9,r8
	call print1

	mov r8,r9
	sar r8,8
	mov r9,r8	
	call print1

	mov r8,r9
	sar r8,8
	mov r9,r8
	call print1

	mov r8,r9
	sar r8,8
	mov r9,r8
	call print1
	
	call _write
	pop rbp
	ret
	
print1:
	push rbp
	sub r10,1
	mov r9b,r8b
	call print1.f1
	mov byte [r10],r8b
	mov r8b,r9b
	sar r8b,4
	call print1.f1
	sub r10,1
	mov byte [r10],r8b
	;; mov byte [2+reg_size8],0x31
	pop rbp
	ret
	;; and r8b,0xf0
.f1:
	and r8b,0x0f
	cmp r8b,0x0a
	jae print1.more_than_0x0a
	jmp print1.less_than_0x0a
.more_than_0x0a:
	add r8b,0x57
	ret
.less_than_0x0a:
	add r8b,0x30
	ret
	
_set_to_virtual_register:
	mov [_rax],rax
	mov [_rcx],rcx
	mov [_rdx],rdx
	mov [_rbx],rbx
	mov [_rsp],rsp
	mov [_rbp],rbp
	mov [_rsi],rsi
	mov [_r8 ],r8
	mov [_r9 ],r9
	mov [_r10],r10
	mov [_r11],r11
	mov [_r12],r12
	mov [_r13],r13
	mov [_r14],r14	
	mov [_r15],r15
	ret
	;; mov [_rip],rip

_set_to_real_register:
	mov rax,[_rax]
	mov rcx,[_rcx]
	mov rdx,[_rdx]
	mov rbx,[_rbx]
	mov rsp,[_rsp]
	mov rbp,[_rbp]
	mov rsi,[_rsi]
	mov rdi,[_rdi]	
	mov r8, [_r8 ]
	mov r9, [_r9 ]
	mov r10,[_r10]
	mov r11,[_r11]
	mov r12,[_r12]
	mov r13,[_r13]
	mov r14,[_r14]
	mov r15,[_r15]
	ret
	
;;; set_to_real_register
;;; jump to instruction that you want to jump on
;;; set_to_virtual register after the operation

test_f1:
	push rax
	
_test_on_real_cpu:
	;; set the starting address of test subject
	mov [_rip],rdi
	add rdi,rsi
	inc rdi
	;; insert return at the end of it
	mov [rdi],0xc3
	;; set all of registers but rip
	call _set_to_real_register
	call [_rip]
	call _set_to_virtual_register
	
_test_on_v_cpu:	
	call _exec 
	call _set_to_real_register
	call [test_sub_addr]
	call _set_to_virtual_register
	
_initialize_regs:
	mov [_rax],0
	mov [_rcx],0
	mov [_rdx],0
	mov [_rbx],0
	;; mov [_rsp],0
	;; mov [_rbp],0
	mov [_rsi],0
	mov [_rdi],0
	mov [_r8 ],0
	mov [_r9 ],0
	mov [_r10],0
	mov [_r11],0
	mov [_r12],0
	mov [_r13],0
	mov [_r14],0
	mov [_r15],0
	
	
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
	
_write:
	mov rax, 0x2000004 ; write
	mov rdi, 1 ; stdout
	mov rsi, reg_size8
	mov rdx, reg_size8.len
	syscall
	ret



