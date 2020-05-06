
	SECTION .text
	extern g1

read:
	mov rax,0
	mov rdi,0
	lea rsi,[d2]
	mov rdx,0x10
	syscall

write:
	mov rax, 1
	mov rdi, 1
	mov rsi, msg
	mov rdx, lens
	syscall

	jmp read
	
done:
	mov rax, 60
	mov rdi, 0
	syscall

	nop

	section .data
d2:	dq 0
msg:	db "flag",4
lens:	equ $-msg
