
	default rel
	section .text

	global _0x0fa0_pushfs
	global _0x0fa1_popfs
	global _0x0fa2_cpuid
	global _0x0fa3_bt
	global _0x0fa4_shld
	global _0x0fa5_shld
	
	global _0x0fa8_pushgs
	global _0x0fa9_popgs
	global _0x0faa_rsm
	global _0x0fab_bts
	global _0x0fac_shrd
	global _0x0fad_shrd
	global _0x0fae_fpu
	global _0x0faf_imul

%include "extern_for_inst.asm"

_0x0fa0_pushfs:	
	db 0xeb,0xfe
_0x0fa1_popfs:	
	db 0xeb,0xfe
_0x0fa2_cpuid:
	add dword [_rip],1
	ret
	db 0xeb,0xfe
;;; bittest
_0x0fa3_bt:
	db 0xeb,0xfe
_0x0fa4_shld:	
	db 0xeb,0xfe
_0x0fa5_shld:	
	db 0xeb,0xfe

_0x0fa8_pushgs:
	db 0xeb,0xfe
_0x0fa9_popgs:
	db 0xeb,0xfe

;; resume from system management mode
_0x0faa_rsm:
	db 0xeb,0xfe

;;; bit test and set
_0x0fab_bts:
	db 0xeb,0xfe

_0x0fac_shrd:	
	db 0xeb,0xfe
_0x0fad_shrd:	
	db 0xeb,0xfe

;;; 0xfe is not limited for fxsave but much more
;;; 0 fxsave
;;; 1 fxrstor
;;; 2 ldmxcsr
;;; 3 stmxcsr
;;; 4 xsave
;;; 5 lfence(xrstor)
;;; 6 mfence
;;; 7 lfence(clflush)
_0x0fae_fpu:
	;; call _get_mod_fpu_rm
	jmp _do01
	db 0xeb,0xfe
	
_0x0faf_imul:	
	db 0xeb,0xfe
