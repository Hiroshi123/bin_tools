
	default rel
	SECTION .text
        global start
	global f1
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

f1:
	mov rax, 1
	mov rdi, 1
	lea rsi, [rel msg]
	mov rdx, 4
	syscall
	nop
	nop
	nop
	ret
	
	SECTION .datadddddddddddd
	
msg:	db "flag",4

lens:	equ $-msg

	