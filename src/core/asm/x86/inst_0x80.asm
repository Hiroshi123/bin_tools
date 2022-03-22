
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
	
%include "extern_for_inst.asm"

__com01:
	add dword [_rip],1
	mov r8,_op01_f_base	
	call _set_imm_op_base
	call _get_mod_op_rm
	call _set_scale_index_base
	call _fetch_displacement_by_mod
	call _fetch8
	mov rax,[_context._res]
	mov [_context._imm_val],rax
	ret

__com02:
	add dword [_rip],1
	mov r8,_op01_f_base
	call _set_imm_op_base
	call _get_mod_op_rm
	call _set_scale_index_base
	call _fetch_displacement_by_mod
	call _fetch_no64
	mov rax,[_context._res]
	mov [_context._imm_val],rax
	ret


_0x80_arith_imm:
	;; call _set_dflag_as_1byte
	call __com01
	ret
	
_0x81_arith_imm:
	call _set_dflag
	call __com02
	ret
	
	add dword [_rip],1
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
	
	pop rbp

	ret
	
_0x82_arith_imm:
	call __com01
	ret

;;; 0x83 feeds just imm 1byte to the register which can contain longer bytes.
;;; Note that the all rest of bytes are sign extended.
_0x83_arith_imm:
	call __com01
	ret

	add byte [_rip],1
	mov r8,_op01_f_base	
	call _set_imm_op_base
	call _get_mod_op_rm

	call _set_scale_index_base
	call _fetch_displacement_by_mod
	call _fetch8
	mov rax,[_context._res]
	mov [_context._imm_val],rax
	ret
	
	call _load_rm_by_mod
	call _mov_res_to_arg1
	call _fetch8_imm_set_to_arg2_with_sign

	mov r8,0x83
	call print
	
	mov r8,[_context._arg2]
	call print
		
	call [_context._imm_op]
	
	call _mov_rm_to_arg1
	call _mov_res_to_arg2
	call _store_or_assign_arg1_by_mod

	ret
	
;;; test operation will only change SF,ZF,PF.
;;; CF and OF is not affected.
_0x84_test:
	add byte [_rip],1
	call _get_mod_reg_rm
	call _set_scale_index_base
	call _fetch_displacement_by_mod	
	ret
	
_0x85_test:
	add dword [_rip],1
	call _get_mod_reg_rm
	call _set_scale_index_base
	call _fetch_displacement_by_mod
	ret

	call _load_rm_by_mod
	call _mov_res_to_arg1
	call _mov_reg_to_arg2
	call _and
	
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
	ret
	
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
	ret
	
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

	add dword [_rip],1
	call _get_mod_reg_rm
	mov byte [_context._dflag],0x00
	call _set_scale_index_base
	call _fetch_displacement_by_mod

	ret

	call _mov_rm_to_arg1
	call _set_reg_to_arg2
	call _store_or_assign_arg1_by_mod
	pop rbp	
	ret
	
_0x89_mov:
	
	add dword [_rip],1
	call _get_mod_reg_rm

	call _set_scale_index_base
	call _fetch_displacement_by_mod

	ret
	
	call _mov_rm_to_arg1
	call _set_reg_to_arg2
	call _store_or_assign_arg1_by_mod	
	pop rbp
	ret
	
_0x8a_mov:
	add byte [_rip],1
	call _get_mod_reg_rm
	mov byte [_context._dflag],0x00
	call _set_scale_index_base
	call _fetch_displacement_by_mod
	ret

_0x8b_mov:
	
	add byte [_rip],1
	call _get_mod_reg_rm
	call _set_scale_index_base
	call _fetch_displacement_by_mod

	ret

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
	add dword [_rip],1
	call _get_mod_reg_rm
	call _set_scale_index_base
	call _fetch_displacement_by_mod
	;; call _fetch32
	;; mov rax,[_context._res]
	;; mov [_context._imm_val],rax
	ret
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

