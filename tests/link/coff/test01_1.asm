
	SECTION .text
        global start
	extern f1
	extern ExitProcess
start:
	call f1
	;; jmp start
	mov rcx,0
	call ExitProcess
	
	SECTION .data
	
msg:	db "flag",4

lens:	equ $-msg



	