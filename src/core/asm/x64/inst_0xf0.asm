
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

	extern print
	
_0xf0_prefix_lock:
	
	add r8,0xf0
	call print

	ret
_0xf1_icebp:
	add r8,0xf1
	call print

	ret
_0xf2_prefix_repnz:
	mov r8,0xf2
	call print

	ret
_0xf3_prefix_repz:
	ret
_0xf4_hlt:
	ret
_0xf5_cmc:
	ret
_0xf6_op:
	mov r8,0xf6
	call print

	ret
_0xf7_op:
	ret
_0xf8_clc:
	mov r8,0xf8
	call print

	ret
_0xf9_stc:
	ret
_0xfa_cli:
	mov r8,0xfa
	call print

	ret
_0xfb_sti:
	mov r8,0xfb
	call print
	ret
_0xfc_cld:
	mov r8,0xfc
	call print
	ret
_0xfd_std:
	mov r8,0xfd
	call print
	ret
_0xfe_op:
	mov r8,0xfe
	call print

	ret
_0xff_op:
	mov r8,0xf0
	call print
	ret

	
