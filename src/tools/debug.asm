
	default rel
	section .text
	;; extern __libc_start_main
	;; extern h1
	extern start
	global __write
_m1:
	;; pop rdi
	;; pop rsi
	mov rdi,rsp
	call start
	mov rax, 60
	mov rdi, 0
	syscall	

__write:
	mov rax, 1
	syscall
	ret
	
__start:
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
	lea rdi,[_m]
	;; call __libc_start_main
	nop
	ret
	
	;; xor ebp,ebp
	;; mov r9,rdx

_m:

	;; pop rdi
	;; pop rsi
	
	call start
	
	;; mov rax, 1
	;; mov rdi, 1
	;; ;; mov rsi, msg1
	;; mov rdx, 4
	;; syscall

	;; mov rax,0
	;; mov rdi,0
	;; lea rsi,[rsp]
	;; mov rdx,0x10
	;; syscall
	
	mov rax, 60
	mov rdi, 0
	syscall
