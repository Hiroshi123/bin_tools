
	default rel
	section .text
	global __z__obj__run_through_coff_shdr3
	global __z__obj__run_through_coff_symtable3

;;; microsoft
;;; caller saved register RAX, RCX, RDX, R8, R9, R10, R11
;;; callee saved register RBX, RBP, RDI, RSI, RSP, R12, R13, R14, and R15

;;; sysv args
;; RDI, RSI, RDX, RCX, R8, R9
	
__z__obj__run_through_coff_shdr3:
	
	push rbp
	mov rbp,rsp
	push rbx
	push rdx

	;; 2nd(rsi) should be set on r13(which is one of registers kept on caller)
	mov r13,rsi
	;; add number of sections
	mov rax,0
	mov word ax,[rdi+2]
	mov r15,rax
	cmp r15,0
	je __z__obj__run_through_coff_shdr3._b2
	;; r12 = string table pointer (end of symbol table)
	;; get End of SymbolTable which is beginning of string table

	;; get NumberOfSymbols
	;; mov dword eax,[rdi+0xc]
	;; mov r14,rax
	;; mov rax,0x12
	;; mul r14
	;; ;; get PointerToSymbolTable(+0x8)
	;; mov r12,rax
	;; mov dword eax,[rdi+0x8]
	;; add r12,rax
	;; add r12,rdi
	mov r12,rdi
	
	;; 1st argument is the top of file pointer
	mov r14,rdi
	add r14,0x14
	;; add size of optional header
	mov rax,0
	mov word ax,[rdi+0x10]
	add r14,rax

	;; preserve rdx(3rd argument) as it is volatile
	;; mov rbx,rcx
	
._b1:
	;; 1st arg
	mov rdi,r14
	;; 2nd arg
	mov rsi,r12
	;; 3rd arg
	mov rdx,[rsp]
	
	call r13

	;; increment size of ImageSectionHeader
	add r14,40
	sub r15,1
	cmp r15,0
	jne __z__obj__run_through_coff_shdr3._b1
	
	;; mov rax,3
	
	;; pop rdx
	;; pop rsi
	;; pop rdi
	;; ;; these values are promised to be kept from the System5 amd64 convention.
	;; pop r15
	;; pop r14
	;; pop r13
._b2:
	pop rdx
	pop rbx
	pop rbp
	
	ret
	
__z__obj__run_through_coff_symtable3:

	push rbp
	mov rbp,rsp
	push rbx
	push r12
	push rdx
	
	;; 2nd(rsi) should be set on r13(which is one of registers kept on caller)
	mov r13,rsi
	;; get PointerToSymbolTable
	mov dword eax,[rdi+0x8]
	mov r15,rax
	add r15,rdi
	
	;; get NumberOfSymbols
	mov dword eax,[rdi+0xc]
	mov r14,rax

	;; get End of SymbolTable which is beginning of string table
	mov rax,0x12
	mul r14
	mov r12, r15
	add r12, rax
._b1:

	;; 1st arg
	mov rdi,r15
	;; 2nd arg
	mov rsi,r12
	;; 3rd arg
	mov rdx,[rsp]
	call r13

	add r15,0x12
	sub r14,1
	cmp r14,0
	jne __z__obj__run_through_coff_symtable3._b1

	pop rdx
	pop r12
	pop rbx
	pop rbp
	
	ret
