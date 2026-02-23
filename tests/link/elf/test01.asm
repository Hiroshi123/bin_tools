
	SECTION .text
        global start
start:
	mov rax,0
	;; state which are going to be fed from 
	mov rax,[rsp+0x10]
	mov rax,[rax]
	cmp al,0x30
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


