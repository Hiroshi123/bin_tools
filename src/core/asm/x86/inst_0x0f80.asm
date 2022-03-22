
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

%include "extern_for_inst.asm"

;;; jmp instruction

__com:
	add dword [_rip],1
	call _fetch32
	mov eax,[_context._res]
	mov [_context._imm_val],eax
	ret
	
_0x0f80_jo:
_0x0f81_jno:
_0x0f82_jnae:
_0x0f83_jnc:
_0x0f84_je:
_0x0f85_jne:
_0x0f86_jna:
_0x0f87_jnbe:	
_0x0f88_js:
_0x0f89_jns:	
_0x0f8a_jpe:	
_0x0f8b_jpo:	
_0x0f8c_jnl:	
_0x0f8d_jng:	
_0x0f8e_jng:	
_0x0f8f_jnle:
	jmp __com
	
;; _0x0f81_jno:
;; 	call _set_eflags
;; 	jno setrip
;; 	ret

;; _0x0f82_jnae:
;; 	jmp __com
;; 	call _set_eflags
;; 	jnae setrip
;; 	ret

;; _0x0f83_jnc:
;; 	jmp __com
;; 	call _set_eflags
;; 	jnc setrip
;; 	ret

;; _0x0f84_je:
;; 	jmp __com
;; 	ret
	
;; 	jmp _0x0f84_je
;; 	call _set_eflags
;; 	je setrip
;; 	ret
	
;; _0x0f85_jne:
;; 	jmp __com

;; 	push rbp
;; 	add dword [_rip],1
;; 	call _fetch32
;; 	mov r8,[_context._res]
	
;; 	mov r8,0x85
;; 	call print
;; 	call _set_eflags
;; 	jne setrip
;; 	pop rbp
	
;; 	ret

;; _0x0f86_jna:
;; 	jmp __com
;; 	call _set_eflags
;; 	jna setrip
;; 	ret

;; _0x0f87_jnbe:
;; 	jmp __com
;; 	call _set_eflags
;; 	jnbe setrip
;; 	ret

;; _0x0f88_js:
;; 	jmp __com
;; 	call _set_eflags
;; 	js setrip
;; 	ret

;; _0x0f89_jns:
;; 	jmp __com
;; 	call _set_eflags
;; 	jns setrip
;; 	ret

;; _0x0f8a_jpe:
;; 	call _set_eflags
;; 	jpe setrip
;; 	ret

;; _0x0f8b_jpo:
;; 	call _set_eflags
;; 	jpo setrip
;; 	ret

;; _0x0f8c_jnl:
;; 	add dword [_rip],1
;; 	call _fetch32
;; 	mov eax,[_eflags]
;; 	;; and eax,eflags_cf
;; 	;; cmp eax,eflags_cf
;; 	;; je  _add_rip
;; 	ret

;; _0x0f8d_jng:
;; 	call _set_eflags
;; 	jng setrip
;; 	ret

;; _0x0f8e_jng:
;; 	jmp __com
;; 	call _set_eflags
;; 	jng setrip
;; 	ret

;; _0x0f8f_jnle:
;; 	call _set_eflags
;; 	jnle setrip
;; 	ret

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


