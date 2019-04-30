
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
	extern _add32
	extern _assign64
	extern print
	extern _fetch32_imm_set_to_arg2
	extern _mov_res_to_arg2

	extern _gen_push
	
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

_ex:
	
	mov r8,[rsp]
	call print
	ret
	
_0xe8_call:
	push rbp
	;; mov r8,0x44
	;; call print
	;; mov r8,[_rip]
	;; call print
	mov r8,0xe8
	call print
	mov r8,rsp
	call print
	
	call _ex
	call _ex
	add byte [_rip],0x01
	call _fetch32_imm_set_to_arg2	
	add byte [_rip],0x04
	;; mov rax,[_rip]
	;; mov [_context._arg1],rax	
	;; call _add32
	
	mov rax,[_context._arg2]
	;; before adding rax, you need to store rip to be returned on it.
	mov rdx,[_rip]	
	add [_rip],rax
	
	mov [_context._internal_arg1],rdx
	call _gen_push
	
	add r8,[_rsp]
	call print
	;; mov rax,_rip
	;; mov [_context._arg1],rax
	;; call _mov_res_to_arg2
	;; call _assign64

	;; mov r8,0x22
	;; call print
	;; mov r8,_rip
	;; call print
	pop rbp
	ret
	
_0xe9_jmp:
	ret
_0xea_jmp:
	mov r8,0xea
	call print

	ret
_0xeb_jmp:
	ret
	
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
