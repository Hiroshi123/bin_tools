
	default rel
	
	section .text
	global _0xc0_shift
	global _0xc1_shift	
	global _0xc2_ret
	global _0xc3_ret
	global _0xc4_les
	global _0xc5_lds
	global _0xc6_mov
	global _0xc7_mov
	global _0xc8_enter
	global _0xc9_leave	
	global _0xca_lret
	global _0xcb_lret
	global _0xcc_int3
	global _0xcd_int	
	global _0xce_into	
	global _0xcf_iret

	extern print
	extern _rbp
	extern _rip
	extern _rsp

	extern _get_host_addr_from_guest
	extern _context._arg1
	extern _context._arg2
	extern _context._dflag
	extern _context._imm_op
	
	extern _context._internal_arg1
	extern _gen_pop

	extern _get_mod_op_rm
	extern _get_mod_reg_rm
	extern _set_scale_index_base
	extern _load_arg1_by_mod
	extern _fetch8_imm_set_to_arg2	
	extern _fetch32_imm_set_to_arg2
	extern _store_or_assign_arg1_by_mod
	extern _fetch_displacement_by_mod
	extern _mov_res_to_arg1
	extern _mov_res_to_arg2	
	extern _mov_rm_to_arg1
	extern _get_host_addr_from_guest

	extern _set_imm_op_base
	extern _op_shift_base


	extern _load_rm_by_mod
	
_0xc0_shift:
	ret
_0xc1_shift:
	push rbp
	mov r8,0xc1
	call print
	add byte [_rip],1
	mov r8, _op_shift_base
	call _set_imm_op_base
	call _get_mod_op_rm
	add byte [_rip],1
	call _set_scale_index_base
	call _fetch_displacement_by_mod
	call _mov_rm_to_arg1
	call _load_arg1_by_mod
	call _fetch8_imm_set_to_arg2
	call [_context._imm_op]
	call _mov_rm_to_arg1
	call _mov_res_to_arg2
	call _store_or_assign_arg1_by_mod

	pop rbp	
	ret
_0xc2_ret:
	ret
	
_0xc3_ret:
	
	push rbp
	mov r8,0xc3
	call print
	mov r8,[_rsp]
	call print	
	mov rax,_rip
	mov [_context._internal_arg1],rax
	call _gen_pop
	mov r8,[_rsp]
	call print
	
	pop rbp
	ret
_0xc4_les:
	ret
_0xc5_lds:
	ret
	
_0xc6_mov:
	
	push rbp
	mov r8,0xc6
	call print
	add byte [_rip],1
	;; _rip are assumed to be on mod/reg/rm
	call _get_mod_reg_rm
	add byte [_rip],1
	;; set dflag as 0 which means register size will be 1byte.
	mov byte [_context._dflag],0x00

	;; _rip are assumed to be on next of mod/reg/rm to be fetched..
	call _set_scale_index_base
	;; displacement fetch (mod/reg/rm are no longer on the range of an eye)
	call _fetch_displacement_by_mod
	call _mov_rm_to_arg1
	;; fetch imm
	call _fetch8_imm_set_to_arg2
	call _store_or_assign_arg1_by_mod
	pop rbp
	ret
	
_0xc7_mov:
	push rbp
	mov r8,0xc7
	call print
	add byte [_rip],1
	;; _rip are assumed to be on mod/reg/rm
	call _get_mod_reg_rm
	add byte [_rip],1
	;; _rip are assumed to be on next of mod/reg/rm to be fetched..
	call _set_scale_index_base
	;; displacement fetch (mod/reg/rm are no longer on the range of an eye)

	
	call _fetch_displacement_by_mod
	
	
	call _mov_rm_to_arg1

	;; fetch imm
	call _fetch32_imm_set_to_arg2
	call _store_or_assign_arg1_by_mod
	pop rbp
	ret
_0xc8_enter:
	ret
_0xc9_leave:
	ret
_0xca_lret:
	ret
_0xcb_lret:
	ret
_0xcc_int3:
	ret
_0xcd_int:
	ret
_0xce_into:
	ret
_0xcf_iret:
	ret
