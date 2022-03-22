

	extern _rip

	extern _do01
	extern _do02
	extern _do_xmm_xmm
	extern _do_xmm_rm
	
	extern _gen_code
	extern _get_mod_reg_rm
	extern _get_mod_seg_rm
	extern _get_mod_xmm_rm
	extern _get_mod_xmm_xmm
	
	extern _set_scale_index_base
	extern _set_gen_code_base_by_mod
	extern _set_immidiate
	extern _set_gen_code_op_by_dflag

	extern _load_arg1_by_mod
	extern _load_rm_by_mod
	
	extern _store_or_assign_arg1_by_mod

	extern _set_dflag
	extern _fetch
	extern _and
	
	extern _mov_reg_to_arg1	
	extern _mov_reg_to_arg2
	extern _mov_rm_to_arg1	
	extern _mov_rm_to_arg2
	extern _set_reg_to_arg1	
	extern _set_reg_to_arg2
	extern _set_rm_to_arg1
	extern _set_rm_to_arg2

	extern _mov_res_to_arg1	
	extern _mov_res_to_arg2
	
	extern _set_arg1_rm_arg2_reg
	extern _set_arg1_reg_arg2_rm
	extern _get_cr_reg_rm

	extern print

	extern _context._arg1
	extern _context._arg2
	
	extern  _context._mod	
	extern  _context._reg
	extern  _context._rm
	extern _context._dflag	
	extern _context._opcode_table
	extern  _context._override
	extern  _context._override_reg
	extern  _context._data_prefix
	extern  _context._res
	extern  _context._imm_val
	extern  _context._repz
	extern _op_f6_f_base
	extern _op_fe_f_base
	extern _context._rex
	extern _context._arg1
	extern _context._arg2
	extern _context._res
	extern _context._imm_op
	extern _context._repz
	extern _context._repnz
	extern _context._is_rm_src
	extern _context._internal_arg1
	extern _context._internal_arg2
	extern _context._lock

	extern _fetch8_imm_set_to_arg2_with_sign
	
	extern print
	extern _rax
	extern _rcx
	extern _rbp
	extern _rsp
	extern _rip

	extern _sub
	
	extern _set_scale_index_base

	extern _mov_res_to_arg1
	extern _mov_res_to_arg2
	
	extern _load_arg1_by_mod
	extern _load_arg2_by_mod
	extern _load_rm_by_mod
	
	extern _store_or_assign_arg1_by_mod
	
	extern _get_mod_op_rm
	
	extern _assign
	
	extern _fetch
	extern _fetch8_imm_set_to_arg2
	extern _fetch32_imm_set_to_arg2
	extern _fetch_displacement_by_mod
	extern _store
	extern _store8
	extern _load
	extern _load8
	extern _dflag_len
	extern _select_reg

	extern _op01_f_base
	extern _set_imm_op_base
	extern _get_mod_fpu_rm

	extern _test
	extern _not
	extern _neg
	extern _eflags
	extern _set_dflag_as_1byte	
	
	extern _rax
	extern _rcx
	extern _rdx
	extern _rbx
	extern _rsp
	extern _rbp
	extern _rsi
	extern _rdi
	extern _r8
	extern _r9
	extern _r10
	extern _r11
	extern _r12
	extern _r13
	extern _r14
	extern _r15
	extern _rip

	extern _add
	extern _add8
	extern _add16
	extern _add32
	extern _add64

	extern _or
	extern _or8

	extern _xor
	extern _xor8
	extern _cmp
	extern _cmp8
	extern _ss
	extern _ds
	
	extern _fetch
	extern _fetch8
	extern _fetch16
	extern _fetch32
	extern _fetch_no64
	
	extern _assign
	extern _assign8
	
	extern _fetch_displacement_by_mod
	extern _inc_displacement

	extern _extend_opcode_table
	extern _exec_one
	extern _set_dflag_as_1byte
	extern _reset_higher8_access

	extern _op_shift_base
	extern _gen_push
	extern _gen_pop
