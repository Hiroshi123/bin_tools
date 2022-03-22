
;;; 1 immlo(2) 1 | 0 0 0 0 | 0 N
;;; where immlo = 0x00
__0x90_adrp:
	ret

;;; add (immediate)
;;; sf 0 0 1 | 0 0 0 1 |  |
;;; where sf = 1(64bit)
__0x91_add:
	ret
	
;;; and (immediate)
;;; sf 0 0 1 | 0 0 1 0 |  |
;;; where sf = 1(64bit)
__0x92_and:
	ret

;;; sbfiz (signed Bitfield Insert in Zero)
;;; sxtw
;;; sf 0 0 1 | 0 0 1 1 |  |
;;; where sf = 1(64bit)
__0x93_sbfiz:
	ret
	
;;; 1 0 0 1 | 0 1 ... |
__0x97_bl:
	ret
	
;;; sf 0 0 1 | 1 0 1 0 | 0 1 0 0 |
;;; where sf = 1
__0x9a_csel:
	ret
	
;;; sf 0 0 1 | 1 0 1 0 | 1 1 1 0 |
;;; where sf = 1
__0x9a_sdiv:
	ret

;;; 
;;; 1 0 0 1 | 1 0 1 1 | 1 1 0 .. |
__0x9b_umulh:
	ret

;;; floating
;;; sf 0 0 1 | 1 1 1 0 | type(2) 0 0 0 1 |
;;; where sf = 1
__0x9e_scvtf:

	ret
