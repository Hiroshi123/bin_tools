
	default rel
	section .text
	global run_through_elf_phdr3
	global run_through_elf_shdr3
	global run_through_elf_symtable3

run_through_elf_phdr3:

	push rdi
	push rsi
	push rdx

	;; r12 == callback function address
	mov r12,rsi	
	;; r13 == section_head
	mov rax,[rdi+0x20]
	add rax,rdi
	mov r13,rax
	;; r14 == program header size
	mov rax,[rdi+0x34]
	and rax,0x7fff0000
	shr rax,0x10
	mov r14,rax
	;; rbx(tmp) == end of program header
	mov rax,[rdi+0x38]
	and rax,0x0000ffff
	imul rax,r14
	mov rbx,r13
	add rbx,rax
	;; r15 == rdi(heads of elf header)
	mov r15,rdi
._b1:
	;; end of section table, get out of the loop
	cmp r13,rbx
	je run_through_elf_phdr3._b2
	;; arg preparation(needed every time before a call)
	mov rdi,r15
	add r13,r14
	mov rsi,r13	
	mov rdx,[rsp]
	call r12
	jmp run_through_elf_phdr3._b1
._b2:
	pop rdx
	pop rsi
	pop rdi
	
	ret

run_through_elf_symtable3:
	;; r13 == pointer to symbol table
	mov r13,rdi
	;; rbx == pointer to symbol table end
	mov rbx,rsi
	;; r12 == callback function address
	mov r12,rdx
	;; r14 == extra data which is passed on argument of callback.
	mov r14,rcx
._b1:
	;; end of section table, get out of the loop
	cmp r13,rbx
	je run_through_elf_symtable3._b2
	;; arg preparation(needed every time before a call)
	;; mov rdi,r15
	;; 0x18 == sizeof(ELF64_SYM)
	mov rdi,r13
	mov rsi,r14
	call r12
	add r13,0x18
	jmp run_through_elf_symtable3._b1
._b2:
	ret

;;; SystemV AMS64 ABI
;;; caller can use rbx,rbp,r12-r15
run_through_elf_shdr3:

	push rbp
	mov rbp,rsp
	push rbx
	push r12
	push r13
	push r14
	push r15
	;; arguments are preserved on a stack.
	push rdi
	push rsi
	push rdx
	
	;; r12 == callback function address
	mov r12,rsi
	
	;; r13 == section_head
	mov rax,[rdi+0x28]
	add rax,rdi
	mov r13,rax
	;; r14 == section size
	mov rax,[rdi+0x38]
	and rax,0x7fff0000
	shr rax,0x10
	mov r14,rax
	
	;; rbx(tmp) == end of section table
	mov rax,[rdi+0x3c]
	and rax,0x0000ffff
	imul rax,r14
	mov rbx,r13
	add rbx,rax
	
	;; r15 == str table section
	mov rax,[rdi+0x3c]
	and rax,0x7fff0000
	shr rax,0x10
	imul rax,r14
	add rax,r13
	mov rax,[rax+0x18]
	add rax,rdi
	mov r15,rax
._b1:
	;; end of section table, get out of the loop
	cmp r13,rbx
	je run_through_elf_shdr3._b2
	;; arg preparation(needed every time before a call)
	mov rdi,[rsp-0x10]
	mov rsi,r15
	add r13,r14
	mov rdx,r13
	;; extra arguments which are passed
	mov rcx,[rsp]
	call r12
	jmp run_through_elf_shdr3._b1
._b2:
	;; callee might violate the contents of registers.
	;; restore them.
	pop rdx
	pop rsi
	pop rdi
	;; these values are promised to be kept from the System5 amd64 convention.
	pop r15
	pop r14
	pop r13
	pop r12
	pop rbx
	pop rbp
	ret

