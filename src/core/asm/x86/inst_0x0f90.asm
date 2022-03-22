
	default rel
	section .text

	global _0x0f90_seto
	global _0x0f91_setno
	global _0x0f92_setb
	global _0x0f93_setnb
	global _0x0f94_sete
	global _0x0f95_setne
	global _0x0f96_setbe
	global _0x0f97_setnbe
	global _0x0f98_sets
	global _0x0f99_setns
	global _0x0f9a_setp
	global _0x0f9b_setnp	
	global _0x0f9c_setl
	global _0x0f9d_setnl
	global _0x0f9e_setle
	global _0x0f9f_setnle
	
%include "extern_for_inst.asm"

_0x0f90_seto:	
_0x0f91_setno:
_0x0f92_setb:	
_0x0f93_setnb:	
_0x0f94_sete:	
_0x0f95_setne:
_0x0f96_setbe:	
_0x0f97_setnbe:	
_0x0f98_sets:	
_0x0f99_setns:	
_0x0f9a_setp:	
_0x0f9b_setnp:	
_0x0f9c_setl:	
_0x0f9d_setnl:	
_0x0f9e_setle:	
_0x0f9f_setnle:
	add dword [_rip],1	
	call _get_mod_reg_rm
	cmp byte r15b,0xff
	jne _0x0f9f_setnle.b1
	call _set_dflag_as_1byte
	mov byte r15b,0
.b1:
	call _set_scale_index_base
	call _fetch_displacement_by_mod
	ret
	;; todo!
	;; call _do01
	;; ret

