
	SECTION .text
        global start
	extern puts
	extern gf1
ff1:
	call gf1
	jmp ff1
start:
	;; jmp start
	mov rdi, msg	
	call puts
	mov rax, 60		
	mov rdi, 0
	syscall
	
msg:	db "Pikachu",10
