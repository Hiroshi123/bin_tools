
;;; 1 immlo(2) 1 | 0 0 0 0 | 0 N
;;; where immlo = 0x10
__d0_adrp:
	ret

;;; sf 1 0 1 | 0 0 0 1 | 
;;; where fs = 0x0
__d1_sub:
	ret

;;; sf 1 0 1 | 0 0 1 1 | 0 N
;;; where sf = 1
__d3_lsl:
	ret

;;; 1 1 0 1 | 0 1 0 0 | 0 0 0 ..
__d4_svc:
	ret

;;; 1 1 0 1 | 0 1 1 0 |
__d6_ret:
	ret
	