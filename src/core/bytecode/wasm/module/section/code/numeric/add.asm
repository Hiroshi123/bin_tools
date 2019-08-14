
	section .text
	default rel

	global _0x6a_add

	extern _inp
	extern _outp
	extern _codep
	extern _stack_depth	
	extern print
	extern _localv_base_reg

;; 0x83 0xc0 (2bit|3bit|3bit|)
;;; 0x83,0xc0
;;; const is something that you are able to push the
_0x6a_add:	
	mov r8,0x02
	call print
	
	mov al,0x01
	call _write_on_codep
	
	mov al,0xc0
	mov cl,0x00
	mov dl,0x01
	call _gen_mod_reg_rm
	call _write_on_codep	
	;; mov byte [_codep],_al
	;; add dword [_codep],0x01
	
	sub dword [_stack_depth],0x01	
	;; al,cl,dl
	ret

_write_on_codep:
	mov r9,[_codep]
	mov byte [r9],al
	add byte [_codep],0x01
	ret

;;; mod_reg_rm
_gen_mod_reg_rm:
	or al,cl
	or al,dl
	ret

;;; mod_op_rm
_gen__reg_rm:
	
	ret
	
