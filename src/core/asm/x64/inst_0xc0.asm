
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
	extern _rip
	extern _rsp

	extern _get_host_addr_from_guest
	extern _context._internal_arg1
	extern _gen_pop
	
_0xc0_shift:
	ret
_0xc1_shift:
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
	ret
_0xc7_mov:
	mov r8,0xc7
	call print
	add byte [_rip],1
	
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
