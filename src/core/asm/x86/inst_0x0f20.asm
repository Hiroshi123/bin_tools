
	default rel
	section .text
	
	global _0x0f20_mov_crn
	global _0x0f21_mov_drn
	global _0x0f22_mov_crn
	global _0x0f23_mov_drn
	global _0x0f28_movaps
	global _0x0f29_movaps
	global _0x0f2a_convert_xmm
	global _0x0f2b_mov_xmm
	global _0x0f2c_convert_xmm
	global _0x0f2d_convert_xmm
	global _0x0f2e_ucmp_xmm
	global _0x0f2f_cmp_xmm

%include "extern_for_inst.asm"

;;; mov the value of a control register to GPR
_0x0f20_mov_crn:
	add dword [_rip],0x01
	call _get_cr_reg_rm	
	call _mov_rm_to_arg1
	call _set_reg_to_arg2
	call _assign
	ret

_0x0f21_mov_drn:
	ret

;;; mov the value of GPR to a control register
;;; vpinsrd
_0x0f22_mov_crn:
	add dword [_rip],0x01
	call _get_cr_reg_rm	
	call _mov_reg_to_arg1
	call _set_rm_to_arg2
	call _assign
	ret

_0x0f23_mov_drn:
	ret
;;; 
_0x0f28_movaps:
	;; ret
_0x0f29_movaps:
	add dword [_rip],0x01
	call _get_mod_xmm_rm
	call _set_scale_index_base
	call _fetch_displacement_by_mod
	ret
	
_0x0f2a_convert_xmm:
	ret
_0x0f2b_mov_xmm:
	ret
_0x0f2c_convert_xmm:
	ret
_0x0f2d_convert_xmm:
	ret

;;; float/double comparison

_0x0f2e_ucmp_xmm:
	jmp _do_xmm_xmm
	
_0x0f2f_cmp_xmm:
	jmp _do_xmm_xmm

