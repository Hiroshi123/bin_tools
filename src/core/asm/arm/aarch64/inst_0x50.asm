

;;; branch with condition
;;; sf 1 0 1 | 0 0 0 1|
;;; where sf = 0(32bit)
__0x51_b_sub:
	ret


;;; branch with condition
;;; 0 1 0 1 | 0 1 ..|
__0x54_b_con:
	ret

;;; ldr literal
;;; 0 1 0 1 | 0 1 ..|
__0x58_b_ldr:
	ret

;;; conditional select inverts
;;; cinv csetm
;;; sf 1 0 1 | 1 0 1 0 | 1 0 0 |
;;; where sf = 0
__0x5a_csinv:
	ret


	