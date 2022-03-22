

;;; 1 immlo(2) 1 | 0 0 0 0 | 0 N
;;; where immlo = 0x11
__0xf0_adrp:
	ret

	
;;; subs
;;; sf 1 1 1 | 0 0 0 1 |
;;; where sf = 1
__0xf1_cmp:

	ret
	
;;; 1 x 1 1 | 1 0 0 0
__0xf8_ldr_str:

	ret
	
;;; 1 x 1 1 | 1 0 0 1
__0xf9_ldr_str:

	ret

;;; conditional cmp
;;; sf 1 1 1 | 1 0 1 0 |
;;; where sf = 1
__0xfa_ccmp:
	ret

;;; 1 x 1 1 | 1 1 0 1
__0xfc_ldr_str:

	ret

;;; 1 x 1 1 | 1 1 0 1
;;; where x(size) = 1
	
__0xfd_ldr_str:

	ret
	
	

