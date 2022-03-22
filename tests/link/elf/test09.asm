

	section .text
	extern f3
	;; extern f2
h1:	
	call f3
	;; call f1
	;; call f2

	mov rax, 60
	mov rdi, 0
	syscall

	