

;;; cmov

	default rel
	section .text
	
	global _0x0f40_cmovo
	global _0x0f41_cmovno
	global _0x0f42_cmovb
	global _0x0f43_cmovnb
	global _0x0f44_cmovz
	global _0x0f45_cmovnz
	global _0x0f46_cmovbe
	global _0x0f47_cmovnbe
	global _0x0f48_cmovs
	global _0x0f49_cmovns
	global _0x0f4a_cmovp
	global _0x0f4b_cmovnp
	global _0x0f4c_cmovl
	global _0x0f4d_cmovnl
	global _0x0f4e_cmovle
	global _0x0f4f_cmovnle	

%include "extern_for_inst.asm"

_0x0f40_cmovo:
_0x0f41_cmovno:	
_0x0f42_cmovb:	
_0x0f43_cmovnb:	
_0x0f44_cmovz:	
_0x0f45_cmovnz:	
_0x0f46_cmovbe:	
_0x0f47_cmovnbe:	
_0x0f48_cmovs:	
_0x0f49_cmovns:	
_0x0f4a_cmovp:	
_0x0f4b_cmovnp:	
_0x0f4c_cmovl:	
_0x0f4d_cmovnl:	
_0x0f4e_cmovle:	
_0x0f4f_cmovnle:
	add dword [_rip],1
	call _get_mod_reg_rm
	call _set_scale_index_base
	call _fetch_displacement_by_mod
	;; db 0xeb,0xfe
	ret
	