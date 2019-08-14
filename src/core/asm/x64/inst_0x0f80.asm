
	default rel
	section .text
	
	global _0x0f80_jo
	global _0x0f81_jno
	global _0x0f82_jnae
	global _0x0f83_jnc
	global _0x0f84_je
	global _0x0f85_jne
	global _0x0f86_jna
	global _0x0f87_jnbe
	global _0x0f88_js
	global _0x0f89_jns
	global _0x0f8a_jpe
	global _0x0f8b_jpo
	global _0x0f8c_jnl
	global _0x0f8d_jng
	global _0x0f8e_jng
	global _0x0f8f_jnle

	extern _context._res
	extern print
	extern _eflags
	extern _rip
	extern _fetch32
	
;;; jmp instruction

_0x0f80_jo:
	call _set_eflags	
	jo setrip
	ret
	
_0x0f81_jno:
	call _set_eflags
	jno setrip
	ret

_0x0f82_jnae:
	call _set_eflags
	jnae setrip
	ret

_0x0f83_jnc:
	call _set_eflags
	jnc setrip
	ret

_0x0f84_je:
	call _set_eflags
	je setrip
	ret
	
_0x0f85_jne:
	push rbp
	add dword [_rip],1
	call _fetch32
	mov r8,[_context._res]
	
	mov r8,0x85
	call print
	call _set_eflags
	jne setrip
	pop rbp
	
	ret

_0x0f86_jna:
	call _set_eflags
	jna setrip
	ret

_0x0f87_jnbe:
	call _set_eflags
	jnbe setrip
	ret

_0x0f88_js:
	call _set_eflags
	js setrip
	ret

_0x0f89_jns:
	call _set_eflags
	jns setrip
	ret

_0x0f8a_jpe:
	call _set_eflags
	jpe setrip
	ret

_0x0f8b_jpo:
	call _set_eflags
	jpo setrip
	ret

_0x0f8c_jnl:
	add dword [_rip],1
	call _fetch32
	mov eax,[_eflags]
	;; and eax,eflags_cf
	;; cmp eax,eflags_cf
	;; je  _add_rip
	ret

_0x0f8d_jng:
	call _set_eflags
	jng setrip
	ret

_0x0f8e_jng:
	call _set_eflags
	jng setrip
	ret

_0x0f8f_jnle:
	call _set_eflags
	jnle setrip
	ret

_set_eflags:
	push qword [_eflags]
	popf
	ret

setrip:
	add [_rip],rdi
	ret

_add_rip:
	mov eax,[_context._res]
	add [_rip],eax
	pop rbp
	ret


