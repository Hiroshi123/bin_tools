
	section .text
	global __os__exit
	global __os__read
	global __os__write
	global __os__open
	global __os__close
	global __os__lseek
	global __os__mmap
	
__os__exit:
	mov rax, 60
	mov rdi, 0
	syscall
	ret
	
__os__read:
	mov rax,0
	;; mov rdi,0
	;; mov rsi,[rsi]
	;; mov rdx,4
	syscall
	ret
	
__os__write:
	mov rax, 1
	syscall
	ret

__os__open:
	mov rax, 2
	syscall
	ret

__os__close:
	mov rax, 3
	syscall
	ret
	
__os__lseek:
	mov rax, 8
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
	
	