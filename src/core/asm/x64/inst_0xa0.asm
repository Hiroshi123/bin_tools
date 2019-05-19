
	default rel

	section .text
	global _0xa0_mov
	global _0xa1_mov
	global _0xa2_mov
	global _0xa3_mov
	global _0xa4_movsS	
	global _0xa5_movsS
	global _0xa6_cmpsS	
	global _0xa7_cmpsS
	global _0xa8_test	
	global _0xa9_test
	global _0xaa_stosS
	global _0xab_stosS
	global _0xac_lodsS
	global _0xad_lodsS
	global _0xae_scasS
	global _0xaf_scasS

	extern _rip
	extern _rax
	extern _rcx
	extern _rdi
	extern print

	extern _set_dflag
	extern _select_reg
	extern _context._arg1
	extern _context._arg2
	extern _store
	
_0xa0_mov:
	ret
_0xa1_mov:
	ret
_0xa2_mov:
	ret
_0xa3_mov:
	ret
_0xa4_movsS:
	ret
_0xa5_movsS:
	ret
_0xa6_cmpsS:
	ret
_0xa7_cmpsS:
	ret

;;; this test also requires imm and fixed al
_0xa8_test:
	ret
;;; this test also requires imm and fixed ax/eax
_0xa9_test:
	ret

_0xaa_stosS:
	ret
_0xab_stosS:
	push rbp
	add dword [_rip],1
	call _set_dflag
	call _select_reg
	mov r8,0xab
	call print
	mov r8,[_rax]
	call print
	mov r8,[_rdi]
	call print
	mov r8,[_rcx]
	
._done1:

	;; store value
	mov rax,[_rax]
	mov rdi,[_rdi]
	mov [_context._arg1],rdi
	mov [_context._arg2],rax
	call _store
	;; update destination memory
	add dword [_rdi],0x08
	;; if rcx is still above 0, go on.
	sub byte [_rcx],0x01
	mov rcx,[_rcx]
	test rcx,rcx
	jne _0xab_stosS._done1
	pop rbp	
	ret
	
_0xac_lodsS:
	ret
_0xad_lodsS:
	ret
_0xae_scasS:
	ret
_0xaf_scasS:
	ret
