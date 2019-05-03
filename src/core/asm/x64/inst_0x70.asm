
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
	extern _set_eflags
	extern setrip

	extern _fetch8
	extern _context._res
	
;;; eflags carry flag
%define eflags_cf 0x0001
;;; eflags parity flag
%define eflags_pf 0x0004
;;; eflags adjust flag
%define eflags_af 0x0010
;;; eflags zero flag
%define eflags_zf 0x0040
;;; eflags sign flag
%define eflags_sf 0x0080
;;; eflags trap flag
%define eflags_tf 0x0100
;;; eflags interrupt enable flag
%define eflags_if 0x0200
;;; eflags direction flag
%define eflags_df 0x0400
;;; eflags overflow flag
%define eflags_of 0x0800
;;; eflags iopl
%define eflags_iopl 0x3000
;;; eflags nested task flag
%define eflags_nt 0x4000

;;; sign flag & overflow flag
%define eflags_sof 0x0880

;;; carry flag & zero flag
%define eflags_czf 0x0041
	
;;; jmp instruction
	
_0x70_jo:
	call _set_eflags	
	jo setrip
	ret
	
_0x71_jno:
	call _set_eflags
	jno setrip
	ret
;;; CF (x < 0)
_0x72_jnae:
	call _set_eflags
	jnae setrip
	ret

;;; CF (x > 0)
_0x73_jnc:
	call _set_eflags
	jnc setrip
	ret
;;; je is equal to 0 which means zero flag is set
_0x74_je:
	call _set_eflags
	je setrip
	ret
;;; jne means the result of the last instruction was not 0
;;; meaning zero flag is not set
_0x75_jne:
	call _set_eflags
	jne setrip
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
	call _set_eflags
	js setrip
	ret

;;; sign flag is not set
_0x79_jns:
	call _set_eflags
	jns setrip
	ret
;;; partiy flag is set(even parity)
_0x7a_jpe:
	call _set_eflags
	jpe setrip
	ret

;;; partiy flag is not set(odd parity)
_0x7b_jpo:
	call _set_eflags
	jpo setrip
	ret

;;; sign <> overflow
_0x7c_jnge:
	call _set_eflags
	jnl setrip
	ret

;;; sign <> overflow
_0x7d_jge:
	call _set_eflags
	jng setrip
	ret

;;; zero or sign <> overflow
_0x7e_jng:

	push rbp
	add byte [_rip],1
	
	call _fetch8
	add byte [_rip],1
	
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

;;; zero sign overflow
_0x7f_jnle:
	call _set_eflags
	jnle setrip
	ret
	

;;; this is temporary implementation to change ip.
;;; needs to be set more properly later on.
_add_rip:
	mov eax,[_context._res]
	add [_rip],eax
	pop rbp
	ret
	