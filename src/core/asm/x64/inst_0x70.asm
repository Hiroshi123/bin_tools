
	section .text
	default rel
	
	global _0x70_jo
	global _0x71_jno
	global _0x72_jnae
	global _0x73_jnc
	global _0x74_je
	global _0x75_jne
	global _0x76_jna
	global _0x77_ja
	global _0x78_js
	global _0x79_jns
	global _0x7a_jpe
	global _0x7b_jpo
	global _0x7c_jnge
	global _0x7d_jge
	global _0x7e_jng
	global _0x7f_jnle

	extern _rip
	extern _eflags
	;; extern _set_eflags
	;; extern setrip
	
	extern _fetch8
	extern _context._res
	extern print

	extern _context._arg1
	
%include "constant.asm"
	
;;; jmp instruction
	
_0x70_jo:
	push rbp
	add dword [_rip],0x1
	call _fetch8
	mov eax,[_eflags]
	and eax,eflags_of
	cmp eax,eflags_of
	je  _add_rip
	pop rbp
	ret
	
_0x71_jno:
	push rbp
	add dword [_rip],0x1
	call _fetch8
	mov eax,[_eflags]
	and eax,eflags_of
	cmp eax,eflags_of
	jne  _add_rip
	pop rbp
	ret

;;; CF (x < 0)
_0x72_jnae:
	push rbp
	add dword [_rip],0x1
	call _fetch8
	mov eax,[_eflags]
	and eax,eflags_cf
	cmp eax,eflags_cf
	je  _add_rip
	pop rbp
	ret

;;; CF (x > 0)
_0x73_jnc:
	push rbp
	add dword [_rip],0x1
	call _fetch8
	mov eax,[_eflags]
	and eax,eflags_cf
	cmp eax,eflags_cf
	jne _add_rip
	pop rbp
	ret
	
;;; je is equal to 0 which means zero flag is set
_0x74_je:
	push rbp
	add byte [_rip],1	
	call _fetch8
	mov eax,[_eflags]
	and eax,eflags_zf
	cmp eax,eflags_zf
	je _add_rip
	pop rbp
	;; call _set_eflags
	;; je setrip
	ret
	
;;; jne means the result of the last instruction was not 0
;;; meaning zero flag is not set
_0x75_jne:
	push rbp
	add byte [_rip],1	
	call _fetch8
	mov eax,[_eflags]
	and eax,eflags_zf
	cmp eax,eflags_zf
	jne _add_rip
	pop rbp
	ret

;;; CF or ZF is set >= 0
_0x76_jna:
	call _set_eflags
	jna setrip
	ret
;;; CF == 0 or ZF is set <= 0
_0x77_ja:
	call _set_eflags
	jnbe setrip
	ret

;;; sign flag is set
_0x78_js:
	push rbp
	add byte [_rip],1	
	call _fetch8
	mov eax,[_eflags]
	and eax,eflags_sf
	cmp eax,eflags_sf
	je  _add_rip
	pop rbp
	ret

;;; sign flag is not set
_0x79_jns:
	push rbp
	add byte [_rip],1	
	call _fetch8
	mov eax,[_eflags]
	and eax,eflags_sf
	cmp eax,eflags_sf
	jne _add_rip
	pop rbp
	ret

;;; partiy flag is set(even parity)
_0x7a_jpe:
	push rbp
	add byte [_rip],1	
	call _fetch8
	mov eax,[_eflags]
	and eax,eflags_pf
	cmp eax,eflags_pf
	je  _add_rip
	pop rbp
	ret

;;; partiy flag is not set(odd parity)
_0x7b_jpo:
	push rbp
	add byte [_rip],1	
	call _fetch8
	mov eax,[_eflags]
	and eax,eflags_pf
	cmp eax,eflags_pf
	jne _add_rip
	pop rbp
	ret

;;; sign <> overflow
_0x7c_jnge:
	add byte [_rip],1
	call _fetch8
	mov rax,[_eflags]
	and rax,eflags_sof
	cmp rax,eflags_sf
	je _add_rip
	cmp rax,eflags_of
	je  _add_rip
	ret
	
;;; sign == overflow
;;; sign == overflow
_0x7d_jge:
	add byte [_rip],1
	call _fetch8
	mov rax,[_eflags]
	and rax,eflags_sof
	cmp rax,eflags_sof
	je _add_rip
	cmp rax,0
	jne _add_rip
	ret
	
;;; zero or sign <> overflow
_0x7e_jng:

	push rbp
	add byte [_rip],1
	call _fetch8
	
	mov rax,[_eflags]
	
	and rax,eflags_zf
	cmp rax,eflags_zf
	je _add_rip

	mov rax,[_eflags]
	
	and rax,eflags_sof
	cmp rax,eflags_sf
	je _add_rip
	cmp rax,eflags_of
	je _add_rip

	pop rbp
	ret

;;; zero == 0
;;; sign == overflow
_0x7f_jnle:
	push rbp
	add byte [_rip],1
	call _fetch8
	
	mov rax,[_eflags]
	
	and rax,eflags_zf
	cmp rax,eflags_zf
	je _0x7f_jnle.done
	mov rax,[_eflags]
	and rax,eflags_sof
	cmp rax,eflags_sof
	je _add_rip
	cmp rax,0
	je _add_rip
.done:
	pop rbp
	ret

;;; this is temporary implementation to change ip.
;;; needs to be set more properly later on.
_add_rip:
	mov eax,[_context._res]
	add [_rip],eax
	pop rbp
	ret

_set_eflags:
	mov rax,[_eflags]
	mov rdx,[_context._arg1]
	or rax,rdx
	ret

_unset_eflags:
	mov rax,[_eflags]
	mov rdx,[_context._arg1]
	not rdx
	and rax,rdx
	ret

_check_eflags:
	mov rax,[_eflags]
	and rax,[_context._arg1]
	cmp rax,[_context._arg1]
	ret
	
setrip:
	add [_rip],rdi
	ret



