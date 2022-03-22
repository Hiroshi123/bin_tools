
	default rel	
	section .text

	global _0x0f50_movmskp
	global _0x0f51_sqrt
	global _0x0f52_rsqrt
	global _0x0f53_rcp
	global _0x0f54_rcp
	global _0x0f55_andnp
	global _0x0f56_orp
	global _0x0f57_xorp
	global _0x0f58_add
	global _0x0f59_mul
	global _0x0f5a_cvt2
	global _0x0f5b_cvt2
	global _0x0f5c_sub
	global _0x0f5d_min
	global _0x0f5e_div
	global _0x0f5f_max

%include "extern_for_inst.asm"

_0x0f50_movmskp:
_0x0f51_sqrt:
_0x0f52_rsqrt:	
_0x0f53_rcp:
_0x0f54_rcp:
_0x0f55_andnp:
_0x0f56_orp:
_0x0f57_xorp:
_0x0f59_mul:
_0x0f5b_cvt2:
_0x0f5c_sub:
_0x0f5d_min:
_0x0f5e_div:
_0x0f5f_max:
	jmp _do_xmm_xmm
	;; db 0xeb,0xfe

_0x0f58_add:
	jmp _do_xmm_xmm	
_0x0f5a_cvt2:
	jmp _do_xmm_xmm
	
__do:
	add dword [_rip],0x01
	call _get_mod_xmm_xmm
	call _set_scale_index_base
	call _fetch_displacement_by_mod
	ret


