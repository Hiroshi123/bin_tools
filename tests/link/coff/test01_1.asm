
	SECTION .text
        global start
	extern f1
	extern ExitProcess
start:
	movaps xmm0,xmm1
	movaps xmm1,xmm0
	cpuid
	add qword [rax],1
	xchg rdi,rax
	mov [nosplit rax*0x4 + rax + 3],rcx
	jo start
	add [eax+0x0b],r15b
	call f1
	;; jmp start
	mov rcx,0
	call ExitProcess
	
	SECTION .data
	
msg:	db "flag",4

lens:	equ $-msg
