
	default rel
	section .text

	;; scale index base
	global _set_scale_index_base
	
	global _load_arg1_by_mod
	global _load_arg2_by_mod

	global _mod00_do1
	global _mod01_do1
	global _mod10_do1
	global _mod11_do1

	global _mod00_do2
	global _mod01_do2
	global _mod10_do2
	global _mod11_do2
	
	;; fetch immidiate
	global _fetch_immidiate
	
	;; store or assign
	global _store_or_assign_arg1_by_mod
	global _store_arg1_by_mod
	global _assign_arg1_by_mod
	
	global _set_arg1_rm_arg2_reg
	global _set_arg1_reg_arg2_rm

	global _mov_rm_to_arg1
	global _mov_rm_to_arg2

	global _mov_reg_to_arg1
	global _mov_reg_to_arg2

	global _set_rm_to_arg1
	global _set_rm_to_arg2

	global _set_reg_to_arg1
	global _set_reg_to_arg2

	global _mov_res_to_arg1
	global _mov_res_to_arg2
	
	
	extern _mod_f_base01
	extern _mod_f_base02
	extern _context._mod
	extern _context._rm
	extern _context._reg
	extern _context._res

	extern _context._dflag
	extern _context._aflag
	extern _context._arg1
	extern _context._arg2

	extern _rax
	
	extern _load
	extern _load8
	extern _load16
	extern _load32
	extern _load64
	
	extern _fetch
	extern _fetch8
	extern _fetch16
	extern _fetch32
	extern _fetch64

	extern _store
	extern _store8
	extern _store16
	extern _store32
	extern _store64

	extern _assign
	extern _assign8
	extern _assign16
	extern _assign32
	extern _assign64

	extern _add
	extern _add8
	extern _add16
	extern _add32
	extern _add64

	extern _sub
	extern _sub8
	extern _sub16
	extern _sub32
	extern _sub64
	
	extern print
	
_set_scale_index_base:
	;; mov qword [_context._rm],0	
	ret

_set_rm_to_arg1:
	mov rax,[_context._rm]
	mov rax,[rax]
	mov [_context._arg1],rax	
	ret

_set_rm_to_arg2:
	mov rax,[_context._rm]
	mov rax,[rax]
	mov [_context._arg2],rax	
	ret

_set_reg_to_arg1:
	mov rax,[_context._reg]
	mov rax,[rax]
	mov [_context._arg1],rax	
	ret

_set_reg_to_arg2:
	mov rax,[_context._reg]
	mov rax,[rax]
	mov [_context._arg2],rax
	ret
	
;;; ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
_mov_rm_to_arg1:
	mov rax,[_context._rm]
	mov [_context._arg1],rax	
	ret

_mov_rm_to_arg2:
	mov rax,[_context._rm]
	mov [_context._arg2],rax	
	ret

_mov_reg_to_arg1:
	mov rax,[_context._reg]
	mov [_context._arg1],rax	
	ret

_mov_reg_to_arg2:
	mov rax,[_context._reg]
	mov [_context._arg2],rax	
	ret

_mov_res_to_arg1:
	mov rax,[_context._res]
	mov [_context._arg1],rax	
	ret

_mov_res_to_arg2:
	mov rax,[_context._res]
	mov [_context._arg2],rax	
	ret

_mod00_do1:
	push rbp
	;; call load
	call _set_rm_to_arg1
	call _load
	pop rbp
	ret

_mod01_do1:
	;; fetch
	;; load

	call _fetch8
	call _set_rm_to_arg1
	call _mov_res_to_arg2
	call _add
	call _mov_res_to_arg1
	call _load

	ret
	
_mod10_do1:
	;; fetch
	;; load
	call _fetch32
	call _set_rm_to_arg1
	call _mov_res_to_arg2
	call _add
	call _mov_res_to_arg1
	call _load
	ret
	
_mod11_do1:
	ret

_load_arg2_by_mod:
	push rbp
	mov ax,0
	lea rdx,[_mod_f_base01]
	mov al,[_context._mod]
	add dx,ax
	call [rdx]
	call _set_rm_to_arg2
	mov r8,[_context._arg2]
	call print
	pop rbp
	ret
	
_load_arg1_by_mod:
	push rbp
	mov ax,0
	lea rdx,[_mod_f_base01]
	mov al ,[_context._mod]
	add dx,ax
	call [rdx]
	call _set_rm_to_arg1
	mov r8,_rax
	call print
	mov r8,[_context._arg1]
	call print
	pop rbp
	ret
	
;;; ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

_mod00_do2:
	jmp _load
	
_mod01_do2:
	;; fetch
	;; load
	push rbp
	pop rbp
	ret
	
_mod10_do2:
	;; fetch
	;; load	
	push rbp
	call _fetch32
	call _add32
	call _load
	pop rbp
	ret
	
_mod11_do2:
	call _assign
	mov r8,0x57
	call print	
	ret

_store_or_assign_arg1_by_mod:
	push rbp
	mov ax,0
	lea rdx,[_mod_f_base02]
	mov al,[_context._mod]
	add dx,ax
	call [rdx]
	pop rbp
	ret

_store_arg1_by_mod:
	ret

_assign_arg1_by_mod:	
	ret
	

_set_arg1_rm_arg2_reg:
	mov rax,_context._rm
	mov qword [_context._arg1],rax
	mov rax,_context._reg
	mov qword [_context._arg2],rax
	ret
	
_set_arg1_reg_arg2_rm:
	mov rax,_context._reg
	mov [_context._arg1],rax
	mov rax,_context._rm
	mov [_context._arg2],rax
	ret
	

	
