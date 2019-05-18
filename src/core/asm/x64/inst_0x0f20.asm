
	default rel
	section .text
	
	global _0x0f20_mov_crn
	global _0x0f21_mov_drn
	global _0x0f22_mov_crn
	global _0x0f23_mov_drn

	extern _get_cr_reg_rm

	extern _context._reg
	
	extern _context._arg1
	extern _context._arg2
	extern _context._res
	extern print
	extern _eflags
	extern _rip
	extern _fetch32
	extern _cr0
	
	extern _mov_reg_to_arg1
	extern _mov_reg_to_arg2
	extern _set_reg_to_arg1
	extern _set_reg_to_arg2
	
	extern _mov_rm_to_arg1
	extern _mov_rm_to_arg2
	extern _set_rm_to_arg1
	extern _set_rm_to_arg2

	extern _assign
	
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
_0x0f22_mov_crn:
	add dword [_rip],0x01
	call _get_cr_reg_rm	
	call _mov_reg_to_arg1
	call _set_rm_to_arg2
	call _assign
	ret

_0x0f23_mov_drn:
	ret


