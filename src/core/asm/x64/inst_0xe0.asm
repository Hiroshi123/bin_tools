
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
	extern _context._arg1
	extern _context._arg2
	extern _context._res
	extern _add32
	extern _assign64
	extern print
	extern _fetch32_imm_set_to_arg2
	extern _mov_res_to_arg2
	
_0xe0_loopnz:
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
	ret
;;; rip is special which means it is represented as 16(15)bytes on windows.
;;; if you want to compute it in a way that other registers are done using such as
;;; fetch, add, assign, then you need to be careful about half of bytes starts from the head
;;; is constant against replacement of lower bits.
_0xe8_call:
	push rbp
	;; mov r8,0x44
	;; call print
	;; mov r8,[_rip]
	;; call print
	
	add byte [_rip],0x01
	call _fetch32_imm_set_to_arg2	
	add byte [_rip],0x04
	;; mov rax,[_rip]
	;; mov [_context._arg1],rax	
	;; call _add32
	mov rax,[_context._arg2]
	add [_rip],rax
	
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
	ret
_0xeb_jmp:
	ret
	
_0xec_port_io:
	ret
_0xed_port_io:
	ret
_0xee_port_io:
	ret
_0xef_port_io:
	ret
