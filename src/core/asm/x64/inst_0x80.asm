
	default rel
	
	section .text
	global _0x80_arith_imm
	global _0x81_arith_imm
	global _0x82_arith_imm
	global _0x83_arith_imm
	global _0x84_test
	global _0x85_test
	global _0x86_xchg
	global _0x87_xchg
	global _0x88_mov
	global _0x89_mov
	global _0x8a_mov
	global _0x8b_mov
	global _0x8c_mov_seg
	global _0x8d_lea
	global _0x8e_mov_seg	
	global _0x8f_pop_ev

	extern _rax
	extern _rcx
	extern _rbp	
	extern _rsp
	extern _rip

	extern _sub
	extern _and
	
	extern _set_scale_index_base

	extern _mov_reg_to_arg1
	extern _mov_reg_to_arg2
	extern _set_reg_to_arg1
	extern _set_reg_to_arg2
	
	extern _mov_rm_to_arg1
	extern _mov_rm_to_arg2
	extern _set_rm_to_arg1
	extern _set_rm_to_arg2

	extern _mov_res_to_arg1
	extern _mov_res_to_arg2
	extern _set_res_to_arg1
	extern _set_res_to_arg2

	extern _load_arg1_by_mod
	extern _load_arg2_by_mod
	extern _load_rm_by_mod
	
	extern _store_or_assign_arg1_by_mod
	
	extern print
	extern _get_mod_reg_rm
	extern _get_mod_seg_rm
	extern _get_mod_op_rm

	extern _context._reg
	extern _context._rm
	extern _context._dflag
	extern _context._rex
	extern _context._arg1	
	extern _context._arg2
	extern _context._res
	extern _context._imm_op
	extern _context._internal_arg1
	
	extern _assign
	extern _assign8	
	
	extern _fetch8_imm_set_to_arg2
	extern _fetch8_imm_set_to_arg2_with_sign

	extern _fetch32_imm_set_to_arg2

	extern _fetch_displacement_by_mod

	extern _op01_f_base
	extern _set_imm_op_base
	extern _set_dflag_as_1byte
	
;;; 0x80

_0x80_arith_imm:
	ret
	
_0x81_arith_imm:
	push rbp
	add byte [_rip],1

	mov r8,0x81
	call print

	;; tmp
	mov r8,_op01_f_base	
	call _set_imm_op_base
	call _get_mod_op_rm

	call _set_scale_index_base
	call _fetch_displacement_by_mod
	
	call _load_rm_by_mod
	call _mov_res_to_arg1
	call _fetch32_imm_set_to_arg2

	call [_context._imm_op]
	
	call _mov_rm_to_arg1	
	call _mov_res_to_arg2
	call _store_or_assign_arg1_by_mod
	mov r8,0x81
	call print
	
	pop rbp

	ret
	
_0x82_arith_imm:
	
	ret

;;; 0x83 feeds just imm 1byte to the register which can contain longer bytes.
;;; Note that the all rest of bytes are sign extended.

_0x83_arith_imm:
	
	push rbp
	add byte [_rip],1
	;; tmp
	mov r8,_op01_f_base	
	call _set_imm_op_base
	call _get_mod_op_rm

	call _set_scale_index_base
	call _fetch_displacement_by_mod
	
	call _load_rm_by_mod
	call _mov_res_to_arg1
	call _fetch8_imm_set_to_arg2_with_sign

	mov r8,0x83
	call print
	
	mov r8,[_context._arg2]
	call print
		
	call [_context._imm_op]
	
	mov r8,[_context._res]
	call print
	
	call _mov_rm_to_arg1
	call _mov_res_to_arg2
	call _store_or_assign_arg1_by_mod

	pop rbp
	ret
	
;;; test operation will only change SF,ZF,PF.
;;; CF and OF is not affected.
_0x84_test:
	ret
_0x85_test:

	push rbp
	add byte [_rip],1	
	call _get_mod_reg_rm
	call _set_scale_index_base
	call _fetch_displacement_by_mod
	call _load_rm_by_mod
	call _mov_res_to_arg1
	call _mov_reg_to_arg2
	call _and
	
	mov r8,0x85
	call print
	
	pop rbp
	ret

;;; first, it will assign r/m to reg.
;;; this is always value assignment on a register.
;;; second, the original value of reg will be assigned to a r/m.
;;; r/m might indicate a memory not a register.
_0x86_xchg:
	add byte [_rip],1
	call _get_mod_reg_rm
	call _set_dflag_as_1byte
	call _set_scale_index_base
	call _fetch_displacement_by_mod
	call _load_rm_by_mod
	call _preserve_v_reg
	call _mov_reg_to_arg1
	call _mov_res_to_arg2
	call _assign8
	;; make sure that the value on previous res is kept after assign.
	call _mov_res_to_arg1
	call _retrieve_v_reg
	;; call _mov_reg_to_arg2
	call _store_or_assign_arg1_by_mod
	ret

_0x87_xchg:
	add byte [_rip],1	
	call _get_mod_reg_rm
	call _set_scale_index_base
	call _fetch_displacement_by_mod
	call _load_rm_by_mod
	call _preserve_v_reg	
	call _mov_reg_to_arg1
	call _mov_res_to_arg2
	call _assign
	;; make sure that the value on previous res is kept after assign.
	call _mov_res_to_arg1
	call _retrieve_v_reg
	;; call _mov_reg_to_arg2
	call _store_or_assign_arg1_by_mod
	ret

_preserve_v_reg:
	mov  rax,[_context._reg]
	mov rax,[rax]
	mov [_context._internal_arg1],rax
	ret

_retrieve_v_reg:
	mov rax,[_context._internal_arg1]
	mov  [_context._arg2],rax
	ret
	
_0x88_mov:
	push rbp

	add byte [_rip],1
	call _get_mod_reg_rm
	mov byte [_context._dflag],0x00
	call _set_scale_index_base
	call _fetch_displacement_by_mod
	call _mov_rm_to_arg1
	call _set_reg_to_arg2
	call _store_or_assign_arg1_by_mod
	pop rbp	
	ret
	
_0x89_mov:
	
	push rbp
	add byte [_rip],1
	call _get_mod_reg_rm
	call _set_scale_index_base
	call _fetch_displacement_by_mod
	call _mov_rm_to_arg1
	call _set_reg_to_arg2
	call _store_or_assign_arg1_by_mod	
	pop rbp
	ret
	
_0x8a_mov:
	ret

_0x8b_mov:
	
	push rbp
		mov r8,0x89
	call print
	

	add byte [_rip],1
	call _get_mod_reg_rm
	call _set_scale_index_base
	call _fetch_displacement_by_mod
	call _load_rm_by_mod
	call _mov_res_to_arg2
	call _mov_reg_to_arg1
	call _assign
	pop rbp
	ret
;;; mov segment register to r/m
_0x8c_mov_seg:

	add  dword [_rip],1
	call _get_mod_seg_rm
	call _set_scale_index_base
	call _fetch_displacement_by_mod
	call _mov_rm_to_arg1
	call _set_reg_to_arg2
	call _store_or_assign_arg1_by_mod
	ret
	
_0x8d_lea:
	push rbp
	mov r8,0x8d
	call print

	add byte [_rip],1
	call _get_mod_reg_rm
	call _set_scale_index_base
	call _fetch_displacement_by_mod
	;; note lea does not need memory access.
	;; call _load_rm_by_mod
	call _mov_rm_to_arg2
	call _mov_reg_to_arg1
	call _assign

	pop rbp
	ret

;;; mov r/m to segment register

;;; reg indicates index of segment register.
;;; r/m indicates the register which will
;;; mod is stil valid.
_0x8e_mov_seg:
	add dword [_rip],1

	call _get_mod_seg_rm
	call _set_scale_index_base
	call _fetch_displacement_by_mod
	call _load_rm_by_mod
	call _mov_res_to_arg2
	call _mov_reg_to_arg1
	call _assign
	ret

_0x8f_pop_ev:
	ret

