
	default rel
	
	section .text

	global _0x0fb0_cmpxchg
	global _0x0fb1_cmpxchg
	global _0x0fb2_lss
	global _0x0fb3_btr
	global _0x0fb4_lfs
	global _0x0fb5_lgs
	
	global _0x0fb6_movzbS
	global _0x0fb7_movzwS

	global _0x0fb8_popcnt

	;; global _0x0fb9
	
	global _0x0fba_bitop
	global _0x0fbb_btc

	global _0x0fbc_bsf
	global _0x0fbd_bsr
	
	global _0x0fbe_movsbS
	global _0x0fbf_movswS
	
	extern print

	extern _rip
	extern _mov_res_to_arg1
	extern _mov_res_to_arg2

	extern _get_mod_reg_rm
	extern _mov_reg_to_arg1
	extern _mov_reg_to_arg2
	extern _mov_rm_to_arg1
	extern _mov_rm_to_arg2

	extern _load_rm_by_mod

	extern _context._rm
	extern _context._dflag
	extern _assign
	
	extern _fetch_displacement_by_mod
	extern _set_scale_index_base
	
;;; casting from 1byte to larger bytes
;;; the bits which was not given from another register will be fed as value0.
_0x0fb6_movzbS:

	push rbp
	add byte [_rip],1	
	call _get_mod_reg_rm
	add byte [_rip],1
	;; it loads just 1byte which means dflag must be 0
	mov byte [_context._dflag],0x00	
	call _set_scale_index_base	
	call _fetch_displacement_by_mod
	call _load_rm_by_mod
	call _mov_res_to_arg2
	call _mov_reg_to_arg1
	call _assign
	pop rbp
	ret

;;; [TODO] needs to be sign extended..
_0x0fbe_movsbS:

	push rbp
	add byte [_rip],1	
	call _get_mod_reg_rm
	add byte [_rip],1
	;; it loads just 1byte which means dflag must be 0
	mov byte [_context._dflag],0x00	
	call _set_scale_index_base	
	call _fetch_displacement_by_mod
	call _load_rm_by_mod
	call _mov_res_to_arg2
	call _mov_reg_to_arg1
	call _assign
	pop rbp

	ret	
	