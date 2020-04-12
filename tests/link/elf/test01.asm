
	SECTION .text
        global main
main:
	mov rax, 1
	mov rdi, 1
	mov rsi, msg
	mov rdx, lens
	syscall
	
	mov rax, 60
	mov rdi, 0
	syscall
	
	SECTION .data
msg:	db "Ki World",10
lens:	equ $-msg
	
	
	