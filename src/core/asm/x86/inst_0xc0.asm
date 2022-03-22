
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

%include "extern_for_inst.asm"

_0xc0_shift:
	add dword [_rip],1
	mov r8, _op_shift_base
	call _set_imm_op_base
	call _get_mod_op_rm
	call _set_dflag_as_1byte
	call _set_scale_index_base
	call _fetch_displacement_by_mod
	call _fetch8
	mov rax,[_context._res]
	mov [_context._imm_val],rax	
	ret
	
_0xc1_shift:
	add dword [_rip],1
	mov r8, _op_shift_base
	call _set_imm_op_base
	call _get_mod_op_rm
	call _set_dflag
	call _set_scale_index_base
	call _fetch_displacement_by_mod
	call _fetch8
	mov rax,[_context._res]
	mov [_context._imm_val],rax
	ret

	call _mov_rm_to_arg1
	call _load_arg1_by_mod
	call _fetch8_imm_set_to_arg2
	call [_context._imm_op]
	call _mov_rm_to_arg1
	call _mov_res_to_arg2
	call _store_or_assign_arg1_by_mod
	ret

_0xc2_ret:
	add dword [_rip],1
	call _fetch16
	mov rax,[_context._res]
	mov [_context._imm_val],rax
	ret
	
_0xc3_ret:
	add dword [_rip],1
	ret
	mov rax,_rip
	mov [_context._internal_arg2],rax
	call _gen_pop
	ret
;;; VEX prefixes
_0xc4_les:
	ret
;;; VEX prefixes
_0xc5_lds:
	ret
	
_0xc6_mov:	
	;; push rbp
	add dword [_rip],1
	;; _rip are assumed to be on mod/reg/rm
	call _get_mod_reg_rm
	;; set dflag as 0 which means register size will be 1byte.
	mov byte [_context._dflag],0x00
	;; _rip are assumed to be on next of mod/reg/rm to be fetched..
	call _set_scale_index_base
	;; displacement fetch (mod/reg/rm are no longer on the range of an eye)
	call _fetch_displacement_by_mod
	;; fetch imm
	call _fetch8
	mov rax,[_context._res]
	mov [_context._imm_val],rax
	ret
	
	call _fetch8_imm_set_to_arg2
	call _mov_rm_to_arg1
	call _store_or_assign_arg1_by_mod
	;; pop rbp
	ret
	
_0xc7_mov:
	;; push rbp
	
	add dword [_rip],1
	;; _rip are assumed to be on mod/reg/rm
	call _get_mod_reg_rm
	;; call _set_dflag
	
	;; _rip are assumed to be on next of mod/reg/rm to be fetched..
	call _set_scale_index_base
	;; displacement fetch (mod/reg/rm are no longer on the range of an eye)
	call _fetch_displacement_by_mod
	call _fetch32
	mov rax,[_context._res]
	mov [_context._imm_val],rax
	ret

	call _mov_rm_to_arg1
	call _store_or_assign_arg1_by_mod
	pop rbp
	ret

;;; enter is combination of
;;; 1.push rbp
;;; 2.mov rbp,rsp
;;; 3.sub rsp, N

_0xc8_enter:
	add dword [_rip],0x1
	;; push
	call _select_reg
	mov r8,[r8]
	mov [_context._internal_arg1],r8
	call _gen_push
	;; mov rbp,rsp
	mov rax,_rbp
	mov [_context._arg1],rax
	mov rax,[_rsp]
	mov [_context._arg2],rax
	call _assign
	;; sub rsp,N
	call _fetch8
	call _mov_res_to_arg2
	mov rax,[_rsp]
	mov [_context._arg1],rax	
	call _sub
	ret

;;; 1.mov esp, ebp
;;; 2.pop ebp
_0xc9_leave:
	add dword [_rip],0x1
	ret
	
	call _select_reg
	mov rax,_rsp
	mov [_context._arg1],rax
	mov rax,[_rbp]
	mov [_context._arg2],rax
	call _assign
	mov r8,[r8]
	mov [_context._internal_arg1],r8
	call _gen_pop
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
