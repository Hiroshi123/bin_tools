
	default rel	
	section .text

	global _0x0f10_mov_xmm
	global _0x0f11_mov_xmm
	global _0x0f12_mov_xmm
	global _0x0f13_mov_xmm
	global _0x0f14_mov_xmm
	global _0x0f15_mov_xmm
	;; movhps
	global _0x0f16_mov_xmm
	global _0x0f17_mov_xmm
	
	global _0x0f18_hint_nop
	global _0x0f19_hint_nop
	global _0x0f1a_hint_nop
	global _0x0f1b_hint_nop
	global _0x0f1c_hint_nop
	global _0x0f1d_hint_nop
	global _0x0f1e_hint_nop
	global _0x0f1f_hint_nop

%include "extern_for_inst.asm"

_0x66_movupd:
	ret

_do:	
	add dword [_rip],1
	call _get_mod_xmm_rm
	call _set_scale_index_base
	call _fetch_displacement_by_mod
	;; db 0xeb,0xfe
	ret

_0xf2_movsd:
	jmp _do
_0xf3_movss:
	jmp _do

_movups:
	ret
	
;;; 66/f2/f3/
;;; movss(32)/movsd(64) is reg is not xmm register.
;;; movupps(d) will be m128
_0x0f10_mov_xmm:
_0x0f11_mov_xmm:
	;;
	jmp _do
	
	cmp byte [_context._data_prefix],0
	jne _0x66_movupd
	;; 
	cmp byte [_context._repz],0
	jne _0xf2_movsd
	;; 
	cmp byte [_context._repnz],0
	jne _0xf3_movss
	jmp _movups
	
;; _0x0f11_mov_xmm:
;; 	db 0xeb,0xfe
_0x0f12_mov_xmm:
	db 0xeb,0xfe
_0x0f13_mov_xmm:
	db 0xeb,0xfe

;;; 0f14/0f15 is unpcklp(unpack and interleave)
;;; TODO needs to check UNPCKLPS(64bit) use non-xmm register
_0x0f14_mov_xmm:
	jmp _do_xmm_xmm
	db 0xeb,0xfe
	
_0x0f15_mov_xmm:
	jmp _do
	db 0xeb,0xfe

;;; this is also move
_0x0f16_mov_xmm:
	db 0xeb,0xfe
_0x0f17_mov_xmm:
	db 0xeb,0xfe

;;; 0x18 mightbe pretchnta(0/1/2/3) which prefetch data into caches
_0x0f18_hint_nop:
	ret
_0x0f19_hint_nop:
	ret
_0x0f1a_hint_nop:
	ret
_0x0f1b_hint_nop:
	ret
_0x0f1c_hint_nop:
	ret
_0x0f1d_hint_nop:
	ret
_0x0f1e_hint_nop:
	ret
_0x0f1f_hint_nop:
	
	;; this is a bit complicated
	;; meanwhile fetch 5byte
	;; call _fetch8
	add dword [_rip],1
	mov rax,[_rip]
	mov byte al,[rax]
	cmp byte al,0x00
	je  _0x0f1f_hint_nop.b2
	cmp byte al,0x80
	jb  _0x0f1f_hint_nop.b1
	jg  _0x0f1f_hint_nop.b00
	cmp byte [_context._data_prefix],0xff
	jne _0x0f1f_hint_nop.b0
.b00:
	add dword [_rip],1	
.b0:
	add dword [_rip],5
	ret
	
.b1:
	cmp byte al,0x40
	je _0x0f1f_hint_nop.b11
	add dword [_rip],1
.b11:
	add dword [_rip],2
	ret
.b2:
	add dword [_rip],1
	ret
	
