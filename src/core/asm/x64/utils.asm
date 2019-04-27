
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

	extern _reg_size8

	extern __rax
	extern __rcx
	extern __rdx
	extern __rbx
	extern __rsp
	extern __rbp
	extern __rsi
	extern __rdi
	extern __r8
	extern __r9
	extern __r10
	extern __r11
	extern __r12
	extern __r13
	extern __r14
	extern __r15
	extern __rip
	
test_sub_addr:
	dq 0
	
	section .text
	
	global print
	global _check_register
	global _initialize_v_regs
	global _test_on_real_cpu
	extern _exec
print:
	push rbp
	lea r10,[_reg_size8+0x0a]
	
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
	call _reg_save
	call _write
	call _reg_regain
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

__set_to_virtual_register:
	mov [__rax],rax
	mov [__rcx],rcx
	mov [__rdx],rdx
	mov [__rbx],rbx
	mov [__rsp],rsp
	mov [__rbp],rbp
	mov [__rsi],rsi
	mov [__r8 ],r8
	mov [__r9 ],r9
	mov [__r10],r10
	mov [__r11],r11
	mov [__r12],r12
	mov [__r13],r13
	mov [__r14],r14	
	mov [__r15],r15
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
	;; mov rsp,[_rsp]
	;; mov rbp,[_rbp]
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
	push rbp
	mov [_rip],rdi
	add rdi,rsi
	;; insert return at the end of it
	mov byte [rdi],0xc3
	;; mov byte [rdi],0x48
	;; inc rdi
	;; mov byte [rdi],0xb8
	;; inc rdi
	;; lea rbx,[_test_on_real_cpu.done]
	;; mov qword [rdi],rbx
	;; add rdi,8
	;; mov byte [rdi],0xff
	;; inc rdi
	;; mov byte [rdi],0xe0	
	;; inc rdi
	
	;; set all of registers but rip
	mov [_rsp],rsp
	call _set_to_real_register
	mov rsp,[_rsp]
	call [_rip]
.done:
	call __set_to_virtual_register
	mov rsp,[_rsp]
	pop rbp
	ret
	
_test_on_v_cpu:	
	call _exec 
	call _set_to_real_register
	call [test_sub_addr]
	call _set_to_virtual_register
	
_initialize_v_regs:
	mov dword [_rax],0
	mov dword [_rcx],0
	mov dword [_rdx],0
	mov dword [_rbx],0
	mov dword [_rsp],0
	mov dword [_rbp],0
	mov dword [_rsi],0
	mov dword [_rdi],0
	mov dword [_r8 ],0
	mov dword [_r9 ],0
	mov dword [_r10],0
	mov dword [_r11],0
	mov dword [_r12],0
	mov dword [_r13],0
	mov dword [_r14],0
	mov dword [_r15],0
	
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

_reg_save:
	mov r12,rax
	mov r13,rdi
	mov r14,rsi
	mov r15,rdx
	ret

_reg_regain:
	mov rax,r12
	mov rdi,r13
	mov rsi,r14
	mov rdx,r15
	ret
	
_write:
	mov rax, 0x2000004 ; write
	mov rdi, 1 ; stdout
	lea rsi, [_reg_size8]
	mov rdx, 0x0b
	;; rcx,r8,r9 is another register
	;; mov rdx, _reg_size8.len
	syscall
	ret



