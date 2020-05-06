

	section .text
	global f1

f1:
	;; push rsp
	;; push rbp
	;; call h1
	;; pop rbp
	;; pop rsp
	
	;; push rax
	mov rax,0
	mov rdi,0
	lea rsi,[rsp]
	mov rdx,4
	syscall
	
	;; mov qword [rsp],0x4000b0
	ret	
	
f2:
	mov rax, 60
	mov rdi, 0
	syscall

f3:
	xor rdx, rdx
	push rdx
	mov rax, 0x68732f2f6e69622f
	push rax
	mov rdi, rsp
	push rdx
	push rdi
	mov rsi, rsp
	lea rax, [rdx+59]
	syscall
	