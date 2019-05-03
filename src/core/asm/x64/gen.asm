
	default rel
	section .text

	;; scale index base

	;; this is intented to be done after fetch displacement.
	global _load_arg1_by_mod
	global _load_arg2_by_mod
	global _load_rm_by_mod

	global _mod00_fetch_displacement
	global _mod01_fetch_displacement
	global _mod10_fetch_displacement
	global _mod11_fetch_displacement

	global _mod00_load
	global _mod01_load
	global _mod10_load
	global _mod11_load
	
	global _mod00_store
	global _mod01_store
	global _mod10_store
	global _mod11_store
	;; fetch displacement
	global _fetch_displacement_by_mod
	
	;; fetch immidiate
	global _fetch8_imm_set_to_arg2:
	global _fetch32_imm_set_to_arg2:

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

	global _sib_no_fetch_displacement	
	global _sib_fetch8_displacement
	global _sib_fetch32_displacement
	
	extern _mod_load_base
	extern _mod_fetch_base
	extern _mod_store_base
	
	extern _context._mod
	extern _context._rm
	extern _context._reg
	extern _context._res
	extern _context._sib
	extern _context._sib_displacement
	
	extern _context._dflag
	extern _context._aflag
	extern _context._arg1
	extern _context._arg2
	extern _context._rip_rel

	extern _rax
	extern _rip
	
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
	
;;; ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

_mod00_load:
	push rbp
	;; call load
	call _mov_rm_to_arg1
	call _load
	pop rbp
	ret

_mod01_load:
	push rbp
	;; call load
	call _mov_rm_to_arg1
	mov r8,0x33
	call print	
	
	call _load
	pop rbp
	ret

_mod10_load:
	push rbp
	;; call load
	call _mov_rm_to_arg1
	call _load
	pop rbp
	ret

_mod11_load:
	push rbp
	mov rax,[_context._rm]
	mov rax,[rax]
	mov [_context._res],rax
	pop rbp
	ret

;;; ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

_load_arg2_by_mod:
	push rbp
	mov ax,0
	lea rdx,[_mod_load_base]
	mov al,[_context._mod]
	add dx,ax
	call [rdx]
	call _mov_res_to_arg2
	pop rbp
	ret
	
_load_arg1_by_mod:
	push rbp
	mov ax,0
	lea rdx,[_mod_load_base]
	mov al ,[_context._mod]
	add dx,ax
	call [rdx]
	call _mov_res_to_arg1
	pop rbp
	ret

_load_rm_by_mod:
	push rbp

	mov ax,0
	lea rdx,[_mod_load_base]
	mov al ,[_context._mod]
	add dx,ax
	call [rdx]
	
	pop rbp
	ret

;;; ;;;;;;;;;;;;;;;;;;;;;;;;;;

;;; ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	
;;; this function is agnostic about the existance of
;;; subsequent loading. If mod is 00/01/10, it involes memory
;;; access, but not all of the case needs to issue actual "load" op
;;; as store simply ignores no matter what is stored on the memory that
;;; it is supposed to store.

_mod00_fetch_displacement:
	ret

_mod01_fetch_displacement:
	;; fetch
	push rbp
	call _fetch8
	add byte [_rip],1
	call _set_rm_to_arg1
	call _mov_res_to_arg2
	call _add8
	mov rax,[_context._res]
	mov [_context._rm],rax
	
	pop rbp
	ret
	
_mod10_fetch_displacement:
	;; fetch
	;; load
	push rbp
	call _fetch32
	add byte [_rip],4
	call _set_rm_to_arg1
	call _mov_res_to_arg2
	call _add
	mov rax,[_context._res]
	mov [_context._rm],rax
	pop rbp
	ret

_mod11_fetch_displacement:
	ret

;;; 1. SIB. if rm was computed by scale index base, normal mod does not directly
;;;    imply size of displacement.
;;; Displacement 8bit will be introduced if base == 101 && mod == 01
;;; Displacement 32bit will be introduced if base == 101 && mod == 00/10

_sib_no_fetch_displacement:
	ret
	
_sib_fetch8_displacement:
	push rbp
	call _fetch8
	add byte [_rip],1
	call _mov_rm_to_arg1
	call _mov_res_to_arg2
	call _add8
	mov rax,[_context._res]
	mov [_context._rm],rax
	
	pop rbp
	ret
	
_sib_fetch32_displacement:
	push rbp
	call _fetch32
	add byte [_rip],4
	call _mov_rm_to_arg1
	call _mov_res_to_arg2
	call _add
	mov rax,[_context._res]
	mov [_context._rm],rax
	pop rbp
	ret
	
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;; 
;;; This is the fetch phase of displacemnet depending on the value of mod.
;;; size of code which will be fetched are determined by mod itself.
;;; if mod == 01, it will fetch 8bit.
;;; if mod == 10, it will fetch 32bit.
;;; Note that after being fetched, rip will be stepped up according to the size of the fetch.
;;; after being fetched, the result will be again stored back to [_context._rm] since
;;; displacement calculation takes roles of updates of rm which are a bit different from general calculation..

;;; Here, two things needs to be considered.

;;; 2. RIP offset

_fetch_displacement_by_mod:
	
	push rbp
	;; SIB check
	mov byte al,[_context._sib]
	cmp al,0xff
	je _fetch_displacement_by_mod.sib_displacement
	;; RIP relative offset check
	mov byte al,[_context._rip_rel]
	cmp al,0xff
	je _fetch_displacement_by_mod.rip_relative_offset
	
	mov ax,0
	lea rdx,[_mod_fetch_base]
	mov al ,[_context._mod]
	add dx,ax
	call [rdx]
	
	pop rbp
	ret
	
.sib_displacement:
	call [_context._sib_displacement]
	;; initialization
	mov byte [_context._sib],0x00
	pop rbp
	ret

.rip_relative_offset:
	mov byte [_context._rip_rel],0x00
	call _fetch32
	add byte [_rip],4
	mov r8,[_context._res]
	call print
	;; call _mov_rm_to_arg1
	;; mov rax,[_rip]
	mov rax,[_rip]	
	mov [_context._arg1],rax
	call _mov_res_to_arg2
	call _add64
	mov r8,[_context._arg1]
	call print
	mov r8,[_context._res]
	call print
	mov rax,[_context._res]
	mov [_context._rm],rax
	mov r8,0x78
	call print
	pop rbp
	ret
	
;;; ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

_mod00_store:
	push rbp
	call _store
	pop rbp
	ret	
		
_mod01_store:
	push rbp
	call _store
	pop rbp
	ret
	
_mod10_store:
	push rbp
	call _store
	pop rbp
	ret	
	
_mod11_store:
	push rbp
	call _assign
	pop rbp
	ret

_store_or_assign_arg1_by_mod:
	push rbp
	mov ax,0
	lea rdx,[_mod_store_base]
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
	
_fetch8_imm_set_to_arg2:
	push rbp
	call _fetch8
	add byte [_rip],0x01
	call _mov_res_to_arg2
	pop rbp
	ret
	
_fetch32_imm_set_to_arg2:
	push rbp
	call _fetch32
	add byte [_rip],0x04
	call _mov_res_to_arg2
	pop rbp
	ret
	