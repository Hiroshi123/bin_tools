
	SECTION .text
        global start
start:
	mov rax,1
	cmp rax,1
	jne noflag
	mov rax, 1
	mov rdi, 1
	mov rsi, msg
	mov rdx, lens
	syscall
noflag:
	mov rax, 60
	mov rdi, 0
	syscall
	
	SECTION .data
	
msg:	db "flag",4
	db 0
lens:	equ $-msg


