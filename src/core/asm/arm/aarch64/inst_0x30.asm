
;;; orr
;;; bitmask immidiate
;;; sf 0 1 1 | 0 0 1 0 |
;;; where sf = 0
__0x32_mov:
	ret


;;; bitfield insert
__0x33_bfi:
	ret


;;; sf 0 1 1 | 0 1 0 0 |
;;; where sf = 0
__0x34_cbz:
	ret

;;; sf 0 1 1 | 0 1 0 1 |
;;; where sf = 0
__0x35_cbnz:
	ret
	
;;; load register signed bytes
;;; size(0) 1 1 | 1 0 0 0 |
;;; where size = 0
__0x38_ldursb:
	ret

;;; load register signed bytes
;;; size(0) 1 1 | 1 0 0 1 |
;;; where sf = 0
__0x39_ldrsb:
	ret

;;; conditional compare negative
;;; sf 0 1 1 | 1 0 1 0 |
;;; where sf = 0
__0x3a_ccmn:
	ret
	
;;; load register signed bytes
;;; size(0) 1 1 | 1 1 0 0 |
;;; where sf = 0
__0x3c_ldr:
	ret

	
