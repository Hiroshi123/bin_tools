

;;; add (immediate)
;;; sf 0 0 1 | 0 0 0 1 |
;;; where sf = 0(32bit)
__0x11_add:
	ret

;;; sf 0 0 1 | 0 0 1 0 | 0 N
;;; where sf = 0 (32bit)
__0x12_and:
	ret

;;; sf 0 0 1 | 0 0 1 1 | 0 N
;;; where sf = 0 (32bit)
__0x13_asr:
	ret

;;; sbfiz (signed Bitfield Insert in Zero)
;;; sxth(b)
;;; sf 0 0 1 | 0 0 1 1 |  |
;;; where sf = 0(32bit)
__0x13_sbfiz:
	ret

	
;;; 0 0 0 1 | 0 1 ..|
__0x14_b:
	ret


;;; sf 0 0 1 | 1 0 1 0 | 0 1 0 0 |
;;; where sf = 0
__0x1a_csel:
	ret
	
;;; sf 0 0 1 | 1 0 1 0 | 1 1 1 0 |
;;; where sf = 0
__0x1a_sdiv:
	ret

;;; arithmetic shift register
;;; sf 0 0 1 | 1 0 1 0 | 1 1 0 ..
;;; might be asrv
__0x1a_asr:
	ret

;;; fadd/fmov/fmul/
__0x1e_float:	
	ret

;;; subs
;;; sf 0 0 1 | 1 1 1 0 | type(2) 0 0 0 1 |
;;; where sf = 0
__0x1e_scvtf:
	ret


