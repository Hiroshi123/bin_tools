
	default rel
	section .text
	;; extern __libc_start_main
	;; extern h1
	extern __start

	global __go_entry
	global __read
	global __write
	global __open
	global __close
	global __pread
	global __os__mmap

_m1:
	;; pop rdi
	;; pop rsi
	;; pop rdx
	;; pop rcx
	mov rdi,rsp
	call __start

	mov rax, 60
	mov rdi, 0
	syscall	

__clean:
	mov qword [rsp],0
	add rsp,8	
__go_entry:
	cmp rsp,rsi
	jne __clean
	
	mov rsp,rsi
	;; call rdi
	;; otherwise
	add rsp,0x8	
	jmp rdi
	ret	
__read:	
	mov rax,0
	syscall
	ret

__write:
	mov rax, 1
	syscall
	ret

__open:
	mov rax, 2
	syscall
	ret

__close:
	mov rax, 3
	syscall
	ret

__os__mmap:
	mov rax, 9
	push r10
	mov r10,rcx
	syscall
	mov rcx,r10
	pop r10
	ret

__pread:
	mov rax,17
	syscall
	ret

;; __start:
;; 	xor ebp,ebp
;; 	mov r9,rdx
;; 	pop rsi
;; 	mov rdx,rsp
;; 	and qword rsp,0xfffffffffffffff0
;; 	push rax
;; 	push rsp
;; 	mov r8,0
;; 	;; lea r8 [csu_init]
;; 	;; lea rcx [csu_fini]
;; 	mov rcx,0
;; 	lea rdi,[_m]
;; 	call __libc_start_main
;; 	nop
;; 	ret
	
	;; xor ebp,ebp
	;; mov r9,rdx

_m:

	;; pop rdi
	;; pop rsi
	
	call __start
	
	;; mov rax, 1
	;; mov rdi, 1
	;; ;; mov rsi, msg1
	;; mov rdx, 4
	;; syscall

	;; mov rax,0
	;; mov rdi,0
	;; lea rsi,[rsp]
	;; mov rdx,0x10
	;; syscall
	
	mov rax, 60
	mov rdi, 0
	syscall

