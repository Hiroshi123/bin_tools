
	default rel
	section .text
	global run_through_coff_shdr3
	global run_through_coff_symtable3

run_through_coff_shdr3:

	push rbp
	mov rbp,rsp
	push rbx
	push r12
	;; push r13
	;; push r14
	;; push r15
	;; ;; arguments are preserved on a stack.
	;; push rdi
	;; push rsi
	;; push rdx
	
	mov rax,rdx
	mov rdx,rcx
	add rdx,0x14
	call rax
	mov rax,3

	;; pop rdx
	;; pop rsi
	;; pop rdi
	;; ;; these values are promised to be kept from the System5 amd64 convention.
	;; pop r15
	;; pop r14
	;; pop r13
	pop r12
	pop rbx
	pop rbp
	
	ret
	
run_through_coff_symtable3:
	ret
