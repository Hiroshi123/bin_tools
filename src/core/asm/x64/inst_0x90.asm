
	default rel	
	section .text
	global _0x90_nop
	global _0x91_xchg_eax
	global _0x92_xchg_eax
	global _0x93_xchg_eax
	global _0x94_xchg_eax
	global _0x95_xchg_eax
	global _0x96_xchg_eax
	global _0x97_xchg_eax
	global _0x98_convert
	global _0x99_convert
	global _0x9a_lcall
	global _0x9b_fwait
	global _0x9c_pushf
	global _0x9d_popf
	global _0x9e_sahf
	global _0x9f_lahf
	
	extern print
	extern _xchg
	
	extern _rip
	extern _set_dflag

	extern _context._arg1
	extern _context._arg2
	extern _context._internal_arg1	
	extern _context._internal_arg2
	
	extern _rax
	extern _select_reg

	extern _gen_push
	extern _gen_pop
	
	extern _eflags
	
_0x90_nop:
	add dword [_rip],0x01
	ret
_0x91_xchg_eax:
	add dword [_rip],0x01
	call _set_dflag
	call _select_reg
	mov [_context._arg2],r8
	mov rax,_rax
	mov [_context._arg1],rax
	call _xchg
	ret
_0x92_xchg_eax:
	add dword [_rip],0x01
	call _set_dflag
	call _select_reg
	mov [_context._arg2],r8
	mov rax,_rax
	mov [_context._arg1],rax
	call _xchg
	ret
_0x93_xchg_eax:
	add dword [_rip],0x01
	call _set_dflag
	call _select_reg
	mov [_context._arg2],r8
	mov rax,_rax
	mov [_context._arg1],rax
	call _xchg
	ret
_0x94_xchg_eax:
	add dword [_rip],0x01
	call _set_dflag
	call _select_reg
	mov [_context._arg2],r8
	mov rax,_rax
	mov [_context._arg1],rax
	call _xchg
	ret
_0x95_xchg_eax:
	add dword [_rip],0x01
	call _set_dflag
	call _select_reg
	mov [_context._arg2],r8
	mov rax,_rax
	mov [_context._arg1],rax
	call _xchg
	ret
_0x96_xchg_eax:
	add dword [_rip],0x01
	call _set_dflag
	call _select_reg
	mov [_context._arg2],r8
	mov rax,_rax
	mov [_context._arg1],rax
	call _xchg
	ret
_0x97_xchg_eax:
	add dword [_rip],0x01
	call _set_dflag
	call _select_reg
	mov [_context._arg2],r8
	mov rax,_rax
	mov [_context._arg1],rax
	call _xchg	
	ret

;;; conversion op
;;; get the sign bit of original register & set the value on every bits of another register.
;;; 1. get sign bit with masking by 0x10000000
;;; 2. if it is set, feed it on another register/if none, set them 0.

;;; [todo] you should prepare general shift extension combinating with primitive instruciton.

_0x98_convert:
	push rbp
	call _set_dflag
	
	;; mov rax,[_rax]
	;; mov [_context._arg2],rax
	;; mov [_context._arg2],0b01000000
	;; call _and
	;; call _mov_res_to_arg2
	add byte [_rip],0x01
	
	mov r8,0x98
	call print
	pop rbp
	ret

;;; CWD,CDQ
;;; cwd & cdq will store the higher half bits to another register(dx/dw)
;;; original value is still be kept on original regsiter.
_0x99_convert:
	
	ret
_0x9a_lcall:
	ret
_0x9b_fwait:
	ret
_0x9c_pushf:
	add dword [_rip],0x1
	mov rax,[_eflags]
	mov [_context._internal_arg1],rax
	call _gen_push
	ret
_0x9d_popf:
	add dword [_rip],0x1
	mov rax,[_eflags]
	mov [_context._internal_arg2],rax
	call _gen_pop
	ret

;;; Store AH into flags
;;; this operation is valid only for x86-32.
;;; And be careful not all of flag values are fed to eflags register.
;;; SF(7), ZF(6), AF(4), PF(2), and CF(0)
_0x9e_sahf:
	add dword [_rip],0x1
	mov rax,[_rax]
	mov [_eflags],rax
	ret

;;; Load flags into AH.
;;; this operation is valid only for x86-32.
_0x9f_lahf:
	add dword [_rip],0x1
	mov rax,0
	mov ah,[_eflags]
	mov [_rax],rax
	ret
