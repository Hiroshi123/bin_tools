
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

;;; zero sign overflow
_0x7e_jng:
	call _set_eflags
	jng setrip
	ret

;;; zero sign overflow
_0x7f_jnle:
	call _set_eflags
	jnle setrip
	ret
	

	
