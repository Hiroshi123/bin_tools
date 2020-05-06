
	section .text
	extern __libc_start_main
	extern h1
	global f1
	global ____init1
	global ____init2
	global ____fini
	
f1:
	;; call h1
	;; jmp done

	xor ebp,ebp
	mov r9,rdx
	pop rsi
	mov rdx,rsp
	and qword rsp,0xfffffffffffffff0
	push rax
	push rsp
	mov r8,0
	;; lea r8 [csu_init]
	;; lea rcx [csu_fini]
	mov rcx,0
	lea rdi,[main]
	call __libc_start_main
	nop
	ret
	
csu_init:

	mov rax, 1
	mov rdi, 1
	mov rsi, msg
	mov rdx, 4
	syscall
	
	ret

csu_fini:

	mov rax, 1
	mov rdi, 1
	mov rsi, msg
	mov rdx, 4
	syscall	
	
	ret

____init1:

	mov rax, 1
	mov rdi, 1
	mov rsi, msg
	mov rdx, 4
	syscall
	
	ret

____init2:

	mov rax, 1
	mov rdi, 1
	mov rsi, msg1
	mov rdx, 4
	syscall
	
	ret

____fini:

	mov rax, 1
	mov rdi, 1
	mov rsi, msg2
	mov rdx, 4
	syscall
	
	ret
	
main:
	;; push rsp
	;; push rbp
	call h1
	;; pop rbp
	;; pop rsp
	
	;; push rax
	mov rax,0
	mov rdi,0
	lea rsi,[rsp]
	mov rdx,0x10
	syscall
	
	;; mov qword [rsp],0x4000b0
	
	;; pop rax
	;; push rax
	
	ret
	
done:	
	mov rax, 60
	mov rdi, 0
	syscall


	SECTION .data

msg:	db "flag",4
msg1:	db "pika",4
msg2:	db "hei!",4

lens:	equ $-msg
