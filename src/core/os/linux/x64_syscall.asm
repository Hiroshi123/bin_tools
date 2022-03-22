
	section .text
	global __os__exit
	global __os__read
	global __os__write
	global __os__open
	global __os__close
	global __os__stat
	global __os__fstat
	global __os__lstat
	global __os__lseek
	global __os__mmap

	global __os__clone
	global __os__fork
	global __os__vfork
	global __os__execve
	global __os__exit
	global __os__wait4
	global __os__kill
	global __z__os__fcntl
	
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

__os__stat:
	mov rax, 4
	syscall
	ret

__os__fstat:
	mov rax, 5
	syscall
	ret

__os__lstat:
	mov rax, 6
	syscall
	ret

__os__poll:
	mov rax, 7
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

__os__mprotect:
	mov rax, 10
	syscall
	ret

__os__munmap:
	mov rax, 11
	syscall
	ret

__os__access:
	mov rax, 21
	syscall
	ret

__os__clone:
	mov rax, 56
	syscall
	ret

__os__fork:
	mov rax, 57
	syscall
	ret

__os__vfork:
	mov rax, 58
	syscall
	ret

__os__execve:
	mov rax, 59
	syscall
	ret

__os__exit:
	mov rax, 60
	syscall
	ret

__os__wait4:
	mov rax, 61
	syscall
	ret

__os__kill:
	mov rax, 62
	syscall
	ret

__z__os__fcntl:
	mov rax, 72
	syscall
	ret

	
__os__futex:
	mov rax, 202
	syscall
	ret

	