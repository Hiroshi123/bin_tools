
;;; subs
;;; sf 1 1 1 | 0 0 0 1 |
;;; where sf = 0
__0x71_cmp:

	ret

;;; ands (immediate)
;;; tst
;;; sf 1 1 1 | 0 0 1 0 | 0 N
__0x72_ands:
	
	ret

;;; load register halfword
;;; size(2) 1 1 | 1 0 0 0 | 0 1 0 ..
;;; size = 0x01
__0x78_ldrh:
	
	ret

;;; conditional cmp
;;; sf 1 1 1 | 1 0 1 0 |
;;; where sf = 0
__0x7a_ccmp:
	ret


;;; load register
;;; size(0) 1 1 | 1 1 0 0 |
;;; where size = 0x01
__0x7c_ldr:
	ret


