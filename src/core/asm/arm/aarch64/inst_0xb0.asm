

;;; 1 immlo(2) 1 | 0 0 0 0 | 0 N
;;; where immlo = 0x01
__0xb0_adrp:
	ret

;;; orr
;;; bitmask immidiate
;;; sf 0 1 1 | 0 0 1 0 |
;;; where sf = 1
__0xb2_mov:
	ret


;;; sf 0 1 1 | 0 1 0 0 |
;;; where sf = 1
__0xb4_cbz:
	ret

;;; sf 0 1 1 | 0 1 0 1 |
;;; where sf = 1
__0xb5_cbnz:

	ret
	
	
;;; 1 x 1 1 | 1 0 0 0
__0xb8_ldr_str:

	ret
	
;;; 1 x 1 1 | 1 0 0 1
__0xb9_ldr_str:
	ret

;;; conditional compare negative
;;; sf 0 1 1 | 1 0 1 0 |
;;; where sf = 1
__0xba_ccmn:
	ret

;;; 1 x 1 1 | 1 1 0 1
__0xbc_ldr_str:

	ret

;;; 1 x 1 1 | 1 1 0 1
;;; where x(size) = 1
	
__0xbd_ldr_str:

	ret

	
