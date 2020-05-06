
	section .text
	global main
	extern puts
	extern __libc_start_main
	extern __f1
	extern puts

write:
	pop r8
	;; mov r8,[rsp]
	cmp r8,2
	je done
	pop r8
	pop r8
p1:
	pop r8
	cmp r8,0
	jne p1

	;; pop r8
	pop r8
	pop r8
	;; pop r8
	mov rax, 1
	mov rdi, 1
	mov rsi, r8
	mov rdx, 4
	
	syscall

	call __f1
	;; cmp qword [rdi],0x3

	;; mov rax,[rsi]
	;; cmp rax,[msg]
	;; je done
	;; jmp [rsp]
	
	jmp done
	
	;; mov rdx,r9
	;; mov rsp,rdx
	;; push rax
	;; push rsp
	;; lea r8,[csu_init]
	;; lea rcx,[csu_fini]
	;; lea rdi,[main]
	;; jmp done
	;; call __libc_start_main
	
h1:
	jmp [rsp]
	
csu_init:
	ret
	
csu_fini:
	ret

main:	
	;; mov rax,0
	;; mov rdi,0
	;; lea rsi,[rsp]
	;; mov rdx,0x10
	;; syscall
	ret
	
done:
	mov rax, 60
	mov rdi, 0
	syscall

msg:	db "./ex03.o"

	