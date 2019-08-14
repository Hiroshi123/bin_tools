	
	default rel
	section .data

_reg_size8:
	db 0x30,0x78,0x30,0x30,0x30,0x30,0x30,0x30,0x30,0x30,0x0a
	;; .len: equ $ - _reg_size8
	
	section .text
	global print	
	;; extern _reg_size8

%include "constant.asm"

print:
	push rbp
	lea r10,[_reg_size8+0x0a]
	
	mov r9,r8
	call print1

	mov r8,r9
	sar r8,8
	mov r9,r8
	call print1

	mov r8,r9
	sar r8,8
	mov r9,r8
	call print1

	mov r8,r9
	sar r8,8
	mov r9,r8
	call print1
	call _reg_save
	call _write
	call _reg_regain
	pop rbp
	ret
	
print1:
	push rbp
	sub r10,1
	mov r9b,r8b
	call print1.f1
	mov byte [r10],r8b
	mov r8b,r9b
	sar r8b,4
	call print1.f1
	sub r10,1
	mov byte [r10],r8b
	;; mov byte [2+reg_size8],0x31
	pop rbp
	ret
	;; and r8b,0xf0
.f1:
	and r8b,0x0f
	cmp r8b,0x0a
	jae print1.more_than_0x0a
	jmp print1.less_than_0x0a
.more_than_0x0a:
	add r8b,0x57
	ret
.less_than_0x0a:
	add r8b,0x30
	ret


_reg_save:
	mov r12,rax
	mov r13,rdi
	mov r14,rsi
	mov r15,rdx
	ret

_reg_regain:
	mov rax,r12
	mov rdi,r13
	mov rsi,r14
	mov rdx,r15
	ret
	
_write:
	mov rax, SYS_write
%ifidn __OUTPUT_FORMAT__, macho64
	add rax,0x2000000
%endif
	mov rdi, STDOUT
	lea rsi, [_reg_size8]
	mov rdx, 0x0b
	;; rcx,r8,r9 is another register
	;; mov rdx, _reg_size8.len
	syscall
	ret
