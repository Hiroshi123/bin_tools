
	default rel
	section .text
	global _0xf0_prefix_lock
	global _0xf1_icebp
	global _0xf2_prefix_repnz
	global _0xf3_prefix_repz
	global _0xf4_hlt
	global _0xf5_cmc
	global _0xf6_op
	global _0xf7_op
	global _0xf8_clc
	global _0xf9_stc
	global _0xfa_cli
	global _0xfb_sti	
	global _0xfc_cld
	global _0xfd_std
	global _0xfe_op
	global _0xff_op

%include "extern_for_inst.asm"
	
%include "constant.asm"

;;; next instruction of lock will be done wihtout interrupt.
;;; essential functionality is asserting or locking bus.
;;; read/modify/write instruction often consumed more than 1 cycle, and
;;; allows another instruction comes into the memory btweeen read and write.
_0xf0_prefix_lock:
	add dword [_rip],1
	mov byte [_context._lock],1
	jmp _exec_one

;;; exits to external debugger??
_0xf1_icebp:
	add r8,0xf1
	call print
	ret

_0xf2_prefix_repnz:
	add dword [_rip],1
	mov byte [_context._repz],0xff
	jmp _exec_one
	ret
	
_0xf3_prefix_repz:
	add dword [_rip],1
	mov byte [_context._repnz],0xff
	jmp _exec_one	
	ret	
;;; privileged instruction which is allowed only for stage on ring 0.
_0xf4_hlt:
	ret
;;; complement carry flag
_0xf5_cmc:
	ret

;;; f6/f7/fe/ff cannot set reg by itself as is is substituted for seleciton of opcode.
;;; It means reg is assumed from the opcode.

;;; test/not/neg/mul/imul/div/idiv
;;; in case of f6/f7, there is no 
;;; test is the only op which requires fetching imm32.
;;; not / neg needs only 1 operand.
;;; mul/imul/div/idiv requires 2 operand.
;;; register which was supposed to be indicated by reg
;;; is fixed on al/ax/eax/rax.

_0xf6_op:
	add dword [_rip],1
	mov r8,_op_f6_f_base
	call _set_imm_op_base
	call _get_mod_op_rm
	call _set_dflag_as_1byte
	call _set_scale_index_base
	call _fetch_displacement_by_mod
	call _compensate_reg_f6f7	
	ret

;;; test/not/neg/mul/imul/div/idiv
_0xf7_op:
	add dword [_rip],1
	;; tmp
	mov r8,_op_f6_f_base
	call _set_imm_op_base
	call _get_mod_op_rm

	call _set_scale_index_base
	call _fetch_displacement_by_mod
	call _compensate_reg_f6f7
	
	ret
	
	call _load_rm_by_mod
	;; note that the register that "load by mod" is put
	;; depends on opcode, which are determined by following.
	call _compensate_reg_f6f7
	
	call [_context._imm_op]
	
	call _mov_rm_to_arg1	
	call _mov_res_to_arg2
	call _store_or_assign_arg1_by_mod
	
	pop rbp
	ret
	
;;; if reg == test, fetch 32bit
;;; if 
_compensate_reg_f6f7:
	mov rax,[_context._imm_op]
	mov rax,[rax]
	mov rdx,_test
	cmp rax,rdx
	je _compensate_reg_f6f7._test
	mov rdx,_not	
	cmp rax,rdx
	je _compensate_reg_f6f7._not_neg
	mov rdx,_neg
	cmp rax,rdx
	je _compensate_reg_f6f7._not_neg
	;; else, set rax to reg
	call _mov_res_to_arg2
	mov  rdx,_rax
	mov  [_context._reg],rdx
	call _mov_reg_to_arg1
	ret
	
._test:
	;; call _mov_res_to_arg1
	call _fetch
	mov rax,[_context._res]
	mov [_context._imm_val],rax
	;; call _fetch32_imm_set_to_arg2
	ret
._not_neg:
	call _mov_res_to_arg1
	ret

;;; clear carry flag
;;; 
_0xf8_clc:
	mov r8,0xf8
	call print

	ret
;;; set carry flag 
_0xf9_stc:
	ret
;;; clear interrupt flag
;;; valid if cpl is less than iopl.
;;; it is used for before accessing port-mapped io memory
;;; to prevent inconsistant access.
_0xfa_cli:
	add dword [_rip],1	
	mov r8,0xfa
	call print	
	ret
;;; set interrupt flag
_0xfb_sti:
	add dword [_rip],1
	mov rax,eflags_df
	or [_eflags],rax
	ret
;;; clear direction flag
_0xfc_cld:
	add dword [_rip],1
	mov rax,eflags_df
	not rax
	and [_eflags],rax
	ret

;;; set direction flag
_0xfd_std:
	mov r8,0xfd
	call print
	ret

;;; inc/dec/call/jmp/push
;;; this fe/ff does not require any immidiate fetch.
;;; it does not have reg specification as all of them does require just 1 operand.
;;; Nevertheless note that you need to load displacement
;;; if mod == 00 / 01 / 10, you need to load

_0xfe_op:
	add byte [_rip],1
	;; tmp
	mov r8,_op_fe_f_base
	call _set_imm_op_base
	call _get_mod_op_rm
	call _set_dflag_as_1byte
	call _set_scale_index_base
	call _fetch_displacement_by_mod
	ret
	
	call _load_rm_by_mod
	call _mov_res_to_arg1	
	;; call _fetch8_imm_set_to_arg2

	call [_context._imm_op]
	
	call _mov_rm_to_arg1	
	call _mov_res_to_arg2
	call _store_or_assign_arg1_by_mod
	
	mov r8,0xfe
	call print

	pop rbp
	ret

;;; inc/dec/call/jmp/push
;;; 
_0xff_op:
	add dword [_rip],1
	;; tmp
	mov r8,_op_fe_f_base
	call _set_imm_op_base
	call _get_mod_op_rm
	call _set_scale_index_base
	;; 
	call _fetch_displacement_by_mod
	ret
	
	;; if mod == 00 or 01 or 10, you need memory access.
	call _load_rm_by_mod
	call _mov_res_to_arg1
	;; call _mov_rm_to_arg1
	call [_context._imm_op]

	;; storing will be done by
	
	;; call _mov_rm_to_arg1	
	;; call _mov_res_to_arg2
	;; call _store_or_assign_arg1_by_mod
	
	mov r8,0xff
	call print

	pop rbp
	ret

	
