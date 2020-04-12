
	SECTION .text
        global main
	extern puts
main:
	mov rdi, msg
	call puts
	mov rax, 60
	mov rdi, 0
	syscall
	
msg:	db "Pikachu",10
