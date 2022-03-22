
	default rel

	section .text

	global _0xe0_loopnz
	global _0xe1_loopz
	global _0xe2_loop
	global _0xe3_jecxz
	global _0xe4_port_io
	global _0xe5_port_io
	global _0xe6_port_io
	global _0xe7_port_io

	global _0xe8_call
	global _0xe9_jmp
	global _0xea_jmp
	global _0xeb_jmp	
	
	global _0xec_port_io
	global _0xed_port_io	
	global _0xee_port_io
	global _0xef_port_io

	extern _rip
	extern _rsp
	
	extern _context._arg1
	extern _context._arg2
	extern _context._res
	extern _context._internal_arg1
	extern _context._imm_val
	extern _add32
	extern _assign64
	extern print
	extern _fetch32_imm_set_to_arg2
	extern _mov_res_to_arg2
	extern _gen_push
	extern _fetch8
	extern _fetch32
	extern _add_edge
	
_0xe0_loopnz:
	add r8,0xe0
	call print
	
	ret
_0xe1_loopz:
	ret
_0xe2_loop:
	ret
_0xe3_jecxz:
	ret

;;; ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

_0xe4_port_io:
	ret
_0xe5_port_io:
	ret
_0xe6_port_io:
	ret
_0xe7_port_io:
	;; add [rdi],rdx

	;; push rbp
	mov r8,0xe7

	call print
	
	;; pop rbp
	ret

;;; rip is special which means it is represented as 16(15)bytes on windows.
;;; if you want to compute it in a way that other registers are done using such as
;;; fetch, add, assign, then you need to be careful about half of bytes starts from the head
;;; is constant against replacement of lower bits.

;;; when you call, what needs to be done is
;;; 1. fetch offset information.
;;; 2. push current rip to [rsp+4]
;;; 3. add fetched value with rip.
;;; 4. update rip

;;; 
;;; shifting instruction pointer (4) should be done 

_0xe8_call:
	add dword [_rip],0x01
	;; call _fetch32_imm_set_to_arg2
	call _fetch32
	mov rax,[_context._res]
	mov [_context._imm_val],rax
	ret
	
	;; mov rax,[_rip]
	;; mov [_context._arg1],rax	
	;; call _add32
	
	;; mov rdx,[_context._arg2]
	;; mov rdi,[_rip]
	;; add rdi,rdx
	;; ;; bring rdi to calee
	;; call _add_edge

	mov rax,[_context._arg2]

	;; before adding rax, you need to store rip to be returned on it.
	mov rdx,[_rip]

	;; cannot be rax
	add [_rip],eax
	;; when you add [rip]
	;; original rip and

	mov [_context._internal_arg1],rdx
	call _gen_push
	
	;; add r8,[_rsp]
	;; call print
	
	;; mov rax,_rip
	;; mov [_context._arg1],rax
	;; call _mov_res_to_arg2
	;; call _assign64
	pop rbp
	ret

_0xe9_jmp:
	add dword [_rip],0x01
	call _fetch32
	mov rax,[_context._res]
	mov [_context._imm_val],rax
	ret
	
	;; call _fetch32_imm_set_to_arg2
	;; mov rax,[_rip]
	;; mov [_context._arg1],rax	
	;; call _add32

	;; mov rdx,[_context._arg2]
	;; mov rdi,[_rip]
	;; add rdi,rdx
	;; ;; bring rdi to calee
	;; call _add_edge

	mov rax,[_context._arg2]
	;; before adding rax, you need to store rip to be returned on it.
	mov rdx,[_rip]
	;; cannot be rax
	add [_rip],eax
	ret
;;; this is far jmp
_0xea_jmp:
	mov r8,0xea
	call print
	ret
	
;;; this is near jmp
_0xeb_jmp:
	add dword [_rip],1
	mov qword [_context._res],0
	call _fetch8
	mov rax,0
	mov al,[_context._res]
	mov [_context._imm_val],al
	ret
	;; if 8th bit is set,
	mov dl,al
	and dl,0b10000000
	cmp dl,0b10000000
	je _do_sub
	mov r8,rax
	call print
	add byte [_rip],al
	pop rbp
	ret

_do_sub:
	mov r8,rax
	call print	
	neg al
	mov r8,rax
	call print
	sub [_rip],ax
	pop rbp
	ret

;;; ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
_0xec_port_io:
	ret
_0xed_port_io:
	ret
_0xee_port_io:
	mov r8,0xee
	call print
	ret
_0xef_port_io:
	mov r8,0xef
	call print

	ret
