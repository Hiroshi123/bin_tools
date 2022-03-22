
	
	extern _add
	extern _sub
	extern _and
	extern _or
	extern _xor
	extern _cmp

	;; f6/f7
	extern _test
	extern _not
	extern _neg
	extern _mul
	extern _imul
	extern _div
	extern _idiv
	
	;; fe/ff
	extern _inc
	extern _dec
	extern _call
	extern _jmp
	extern _push
	
	extern _rol
	extern _ror
	extern _shl
	extern _shr	
	
	extern _mod00_load
	extern _mod01_load
	extern _mod10_load
	extern _mod11_load

	extern _mod00_fetch_displacement
	extern _mod01_fetch_displacement
	extern _mod10_fetch_displacement
	extern _mod11_fetch_displacement
	
	extern _mod00_store
	extern _mod01_store
	extern _mod10_store
	extern _mod11_store
	
	extern _load_arg1_by_mod
	extern _load_arg2_by_mod

	extern _0x00_add
	extern _0x01_add
	extern _0x02_add
	extern _0x03_add
	extern _0x04_add
	extern _0x05_add

	extern _0x08_or
	extern _0x09_or
	extern _0x0a_or
	extern _0x0b_or
	extern _0x0c_or
	extern _0x0d_or
	extern _0x00_add
	extern _0x0f

	;; 0x10 adc
	extern _0x10_adc
	extern _0x11_adc
	extern _0x12_adc
	extern _0x13_adc
	extern _0x14_adc
	extern _0x15_adc
	extern _0x00_add
	extern _0x1f

	;; 0x18 sbb
	extern _0x18_sbb
	extern _0x19_sbb
	extern _0x1a_sbb
	extern _0x1b_sbb
	extern _0x1c_sbb
	extern _0x1d_sbb
	
	extern _0x20_and
	extern _0x21_and
	extern _0x22_and
	extern _0x23_and
	extern _0x24_and
	extern _0x25_and
	extern _0x26_prefix_seg_es
	
	extern _0x28_sub
	extern _0x29_sub
	extern _0x2a_sub
	extern _0x2b_sub
	extern _0x2c_sub
	extern _0x2d_sub
	extern _0x2e_prefix_seg_cs	
	extern _0x2f_bcd_das

	extern _0x30_xor
	extern _0x31_xor
	extern _0x32_xor
	extern _0x33_xor
	extern _0x34_xor
	extern _0x35_xor
	extern _0x36_prefix_seg_ss

	;; 0x38 cmp
	extern _0x38_cmp
	extern _0x39_cmp
	extern _0x3a_cmp
	extern _0x3b_cmp
	extern _0x3c_cmp
	extern _0x3d_cmp
	extern _0x3e_prefix_seg_ds
	
	extern _0x40_set_rex
	extern _0x41_set_rex
	extern _0x42_set_rex
	extern _0x43_set_rex
	extern _0x44_set_rex
	extern _0x45_set_rex
	extern _0x46_set_rex
	extern _0x47_set_rex
	;; 0x48
	extern _0x48_set_rex
	extern _0x49_set_rex
	extern _0x4a_set_rex
	extern _0x4b_set_rex
	extern _0x4c_set_rex
	extern _0x4d_set_rex
	extern _0x4e_set_rex
	extern _0x4f_set_rex
	;; 0x50
	extern _0x50_push
	extern _0x51_push
	extern _0x52_push
	extern _0x53_push
	extern _0x54_push
	extern _0x55_push
	extern _0x56_push
	extern _0x57_push
	;; 0x58
	extern _0x58_pop
	extern _0x59_pop
	extern _0x5a_pop
	extern _0x5b_pop
	extern _0x5c_pop
	extern _0x5d_pop
	extern _0x5e_pop
	extern _0x5f_pop

	;; 0x60
	extern _0x60_pusha
	extern _0x61_popa

	extern _0x62_bound
	extern _0x63_movslS
	extern _0x64_prefix_seg_fs
	extern _0x65_prefix_seg_gs
	extern _0x66_prefix_data
	extern _0x67_prefix_addr
	
	extern _0x68_push_iv
	extern _0x69_imul
	extern _0x6a_push_iv
	extern _0x6b_imul
	extern _0x6c_insS
	extern _0x6d_insS
	extern _0x6e_outsS
	extern _0x6f_outsS
	
	extern _0x70_jo
	extern _0x71_jno
	extern _0x72_jnae
	extern _0x73_jnc
	extern _0x74_je
	extern _0x75_jne
	extern _0x76_jna
	extern _0x77_ja
	extern _0x78_js
	extern _0x79_jns
	extern _0x7a_jpe
	extern _0x7b_jpo
	extern _0x7c_jnge
	extern _0x7d_jge
	extern _0x7e_jng
	extern _0x7f_jnle
	;;
	extern _0x80_arith_imm
	extern _0x81_arith_imm
	extern _0x82_arith_imm
	extern _0x83_arith_imm
	extern _0x84_test
	extern _0x85_test
	extern _0x86_xchg
	extern _0x87_xchg
	extern _0x88_mov
	extern _0x89_mov
	extern _0x8a_mov
	extern _0x8b_mov
	extern _0x8c_mov_seg
	extern _0x8d_lea
	extern _0x8e_mov_seg	
	extern _0x8f_pop_ev

	extern _0x90_nop
	extern _0x91_xchg_eax
	extern _0x92_xchg_eax
	extern _0x93_xchg_eax
	extern _0x94_xchg_eax
	extern _0x95_xchg_eax
	extern _0x96_xchg_eax
	extern _0x97_xchg_eax
	extern _0x98_convert
	extern _0x99_convert
	extern _0x9a_lcall
	extern _0x9b_fwait
	extern _0x9c_pushf
	extern _0x9d_popf
	extern _0x9e_sahf
	extern _0x9f_lahf

	extern _0xa0_mov
	extern _0xa1_mov
	extern _0xa2_mov
	extern _0xa3_mov
	extern _0xa4_movsS	
	extern _0xa5_movsS
	extern _0xa6_cmpsS	
	extern _0xa7_cmpsS
	extern _0xa8_test	
	extern _0xa9_test
	extern _0xaa_stosS
	extern _0xab_stosS
	extern _0xac_lodsS
	extern _0xad_lodsS
	extern _0xae_scasS
	extern _0xaf_scasS
	
	extern _0xb0_mov	
	extern _0xb1_mov	
	extern _0xb2_mov	
	extern _0xb3_mov	
	extern _0xb4_mov	
	extern _0xb5_mov	
	extern _0xb6_mov	
	extern _0xb7_mov
	extern _0xb8_mov	
	extern _0xb9_mov	
	extern _0xba_mov	
	extern _0xbb_mov	
	extern _0xbc_mov	
	extern _0xbd_mov	
	extern _0xbe_mov	
	extern _0xbf_mov

	extern _0xc0_shift
	extern _0xc1_shift	
	extern _0xc2_ret
	extern _0xc3_ret
	extern _0xc4_les
	extern _0xc5_lds
	extern _0xc6_mov
	extern _0xc7_mov
	extern _0xc8_enter
	extern _0xc9_leave	
	extern _0xca_lret
	extern _0xcb_lret
	extern _0xcc_int3
	extern _0xcd_int	
	extern _0xce_into	
	extern _0xcf_iret
	
	extern _0xd0_shift
	extern _0xd1_shift
	extern _0xd2_shift
	extern _0xd3_shift
	extern _0xd4_aam
	extern _0xd5_aad
	extern _0xd6_salc	
	extern _0xd7_xlat
	
	extern _0xd8_float
	extern _0xd9_float
	extern _0xda_float
	extern _0xdb_float
	extern _0xdc_float
	extern _0xdd_float
	extern _0xde_float
	extern _0xdf_float

	extern _0xe0_loopnz
	extern _0xe1_loopz
	extern _0xe2_loop
	extern _0xe3_jecxz
	extern _0xe4_port_io
	extern _0xe5_port_io
	extern _0xe6_port_io
	extern _0xe7_port_io

	extern _0xe8_call
	extern _0xe9_jmp
	extern _0xea_jmp
	extern _0xeb_jmp	
	
	extern _0xec_port_io
	extern _0xed_port_io	
	extern _0xee_port_io
	extern _0xef_port_io
	
	extern _0xf0_prefix_lock
	extern _0xf1_icebp
	extern _0xf2_prefix_repnz
	extern _0xf3_prefix_repz
	extern _0xf4_hlt
	extern _0xf5_cmc
	extern _0xf6_op
	extern _0xf7_op
	extern _0xf8_clc
	extern _0xf9_stc
	extern _0xfa_cli
	extern _0xfb_sti	
	extern _0xfc_cld
	extern _0xfd_std
	extern _0xfe_op
	extern _0xff_op

	extern _0x0f05_syscall
	
	extern _0x0f10_mov_xmm
	extern _0x0f11_mov_xmm
	extern _0x0f12_mov_xmm
	extern _0x0f13_mov_xmm
	extern _0x0f14_mov_xmm
	extern _0x0f15_mov_xmm
	extern _0x0f16_mov_xmm
	extern _0x0f17_mov_xmm

	extern _0x0f18_hint_nop
	extern _0x0f19_hint_nop
	extern _0x0f1a_hint_nop
	extern _0x0f1b_hint_nop
	extern _0x0f1c_hint_nop
	extern _0x0f1d_hint_nop
	extern _0x0f1e_hint_nop
	extern _0x0f1f_hint_nop
	extern _0x0f20_mov_crn
	extern _0x0f21_mov_drn
	extern _0x0f22_mov_crn
	extern _0x0f23_mov_drn

	extern _0x0f28_movaps
	extern _0x0f29_movaps
	extern _0x0f2a_convert_xmm
	extern _0x0f2b_mov_xmm
	extern _0x0f2c_convert_xmm
	extern _0x0f2d_convert_xmm
	extern _0x0f2e_ucmp_xmm
	extern _0x0f2f_cmp_xmm

	extern _0x0f40_cmovo
	extern _0x0f41_cmovno
	extern _0x0f42_cmovb
	extern _0x0f43_cmovnb
	extern _0x0f44_cmovz
	extern _0x0f45_cmovnz
	extern _0x0f46_cmovbe
	extern _0x0f47_cmovnbe
	extern _0x0f48_cmovs
	extern _0x0f49_cmovns
	extern _0x0f4a_cmovp
	extern _0x0f4b_cmovnp
	extern _0x0f4c_cmovl
	extern _0x0f4d_cmovnl
	extern _0x0f4e_cmovle
	extern _0x0f4f_cmovnle

	extern _0x0f50_movmskp
	extern _0x0f51_sqrt
	extern _0x0f52_rsqrt
	extern _0x0f53_rcp
	extern _0x0f54_rcp
	extern _0x0f55_andnp
	extern _0x0f56_orp
	extern _0x0f57_xorp
	extern _0x0f58_add
	extern _0x0f59_mul
	extern _0x0f5a_cvt2
	extern _0x0f5b_cvt2
	extern _0x0f5c_sub
	extern _0x0f5d_min
	extern _0x0f5e_div
	extern _0x0f5f_max

	extern _0x0f60_punpckl
	extern _0x0f61_punpckl
	extern _0x0f62_punpckl
	extern _0x0f63_packss
	extern _0x0f64_pcmpgt
	extern _0x0f65_pcmpgt
	extern _0x0f66_pcmpgt
	extern _0x0f67_packus
	extern _0x0f68_punpckh
	extern _0x0f69_punpckh
	extern _0x0f6a_punpckh
	extern _0x0f6b_packss
	extern _0x0f6c_punpckl
	extern _0x0f6d_punpckh
	extern _0x0f6e_mov
	extern _0x0f6f_mov

	extern _0x0f70_pshuf
	extern _0x0f71_pshift
	extern _0x0f72_pshift
	extern _0x0f73_pshift
	
	extern _0x0f74_pcmpeq
	extern _0x0f75_pcmpeq
	extern _0x0f76_pcmpeq
	extern _0x0f77_emms
	extern _0x0f78_vmread
	extern _0x0f79_vmwrite
	;; extern _0x0f7a
	;; extern _0x0f7b
	extern _0x0f7c_haddp
	extern _0x0f7d_hsubp
	extern _0x0f7e_mov
	extern _0x0f7f_mov

	extern _0x0f80_jo
	extern _0x0f81_jno
	extern _0x0f82_jnae
	extern _0x0f83_jnc
	extern _0x0f84_je
	extern _0x0f85_jne
	extern _0x0f86_jna
	extern _0x0f87_jnbe
	extern _0x0f88_js
	extern _0x0f89_jns
	extern _0x0f8a_jpe
	extern _0x0f8b_jpo
	extern _0x0f8c_jnl
	extern _0x0f8d_jng
	extern _0x0f8e_jng
	extern _0x0f8f_jnle

	extern _0x0f90_seto
	extern _0x0f91_setno
	extern _0x0f92_setb
	extern _0x0f93_setnb
	extern _0x0f94_sete
	extern _0x0f95_setne
	extern _0x0f96_setbe
	extern _0x0f97_setnbe
	extern _0x0f98_sets
	extern _0x0f99_setns
	extern _0x0f9a_setp
	extern _0x0f9b_setnp	
	extern _0x0f9c_setl
	extern _0x0f9d_setnl
	extern _0x0f9e_setle
	extern _0x0f9f_setnle
		
	extern _0x0fa0_pushfs
	extern _0x0fa1_popfs
	extern _0x0fa2_cpuid
	extern _0x0fa3_bt
	extern _0x0fa4_shld
	extern _0x0fa5_shld
	
	extern _0x0fa8_pushgs
	extern _0x0fa9_popgs
	extern _0x0faa_rsm
	extern _0x0fab_bts
	extern _0x0fac_shrd
	extern _0x0fad_shrd
	extern _0x0fae_fpu
	extern _0x0faf_imul

	extern _0x0fb0_cmpxchg
	extern _0x0fb1_cmpxchg
	extern _0x0fb2_lss
	extern _0x0fb3_btr
	extern _0x0fb4_lfs
	extern _0x0fb5_lgs
	
	extern _0x0fb6_movzbS
	extern _0x0fb7_movzwS
	extern _0x0fbe_movsbS
	extern _0x0fbf_movswS

	extern _0x0fc0_xadd
	extern _0x0fc1_xadd
	extern _0x0fc2_cmp
	extern _0x0fc3_movnti
	extern _0x0fc4_pinsrw
	extern _0x0fc5_pextrw	
	extern _0x0fc6_shufp
	extern _0x0fc7_cmpxchg_vm
	
	extern _0x0fc8_bswap
	extern _0x0fc9_bswap
	extern _0x0fca_bswap
	extern _0x0fcb_bswap
	extern _0x0fcc_bswap
	extern _0x0fcd_bswap
	extern _0x0fce_bswap
	extern _0x0fcf_bswap

	extern _0x0fd0_addsubp
	extern _0x0fd1_psrlw
	extern _0x0fd2_psrld
	extern _0x0fd3_psrlq
	extern _0x0fd4_paddq
	extern _0x0fd5_pmullw	
	extern _0x0fd6_mov
	extern _0x0fd7_pmovmskb
	
	extern _0x0fd8_psubus
	extern _0x0fd9_psubus
	extern _0x0fda_pminub
	extern _0x0fdb_pand
	extern _0x0fdc_paddusb
	extern _0x0fdd_paddusw
	extern _0x0fde_pmaxub
	extern _0x0fdf_pandn

	extern _0x0fe0_pavgb
	extern _0x0fe1_psraw
	extern _0x0fe2_psrad
	extern _0x0fe3_pavgw
	extern _0x0fe4_pmulhuw
	extern _0x0fe5_pmulhw
	extern _0x0fe6_cvt2
	extern _0x0fe7_movnt
	
	extern _0x0fe8_psubsb
	extern _0x0fe9_psubsw
	extern _0x0fea_pminsw
	extern _0x0feb_por
	extern _0x0fec_paddsb
	extern _0x0fed_paddsw
	extern _0x0fee_pmaxsw
	extern _0x0fef_pxor

	extern _0x0ff0_lddqu
	extern _0x0ff1_psllw
	extern _0x0ff2_pslld
	extern _0x0ff3_psllq
	extern _0x0ff4_pmuludq
	extern _0x0ff5_pmaddwd
	extern _0x0ff6_psadbw
	extern _0x0ff7_maskmov
	
	extern _0x0ff8_psubb
	extern _0x0ff9_psubw
	extern _0x0ffa_psubd
	extern _0x0ffb_psubq
	extern _0x0ffc_paddb
	extern _0x0ffd_paddw
	extern _0x0ffe_paddd
	extern _0x0fff

	;; following will be primitive instruction where x86-63 heads to.

	extern _fetch
	extern _fetch8
	extern _fetch16
	extern _fetch32
	extern _fetch64

	extern _load8
	extern _load16
	extern _load32
	extern _load64

	extern _store8
	extern _store16
	extern _store32
	extern _store64

	extern _assign8
	extern _assign16
	extern _assign32
	extern _assign64

	extern _add8
	extern _add16
	extern _add32
	extern _add64
	
	extern _sub8
	extern _sub16
	extern _sub32
	extern _sub64

	extern _and8
	extern _and16
	extern _and32
	extern _and64
	
	extern _or8
	extern _or16
	extern _or32
	extern _or64	

	extern _xor8
	extern _xor16
	extern _xor32
	extern _xor64

	extern _shl0
	extern _shl1
	extern _shl2
	extern _shl3
	extern _shl4
	extern _shl5
	extern _shl6
	extern _shl7
	extern _shl8	
	
	;; section .text
;; 	global _hello_world
	
;; _hello_world:
;; 	;; mov rax, 0x2000004 ; write
;; 	mov rax, 0x0000001 ; write
;; 	mov rdi, 1 ; stdout
;; 	mov rsi, msg1
;; 	mov rdx, msg1.len
;; 	syscall
;; 	ret

	


