
	
	default rel
	section .text

	global _fetch
	
	extern _load_base
	extern _store_base
	extern _fetch_base
	extern _add_base
	extern _sub_base
	extern _assign_base

	extern _check_on_iat
	extern _find_f_addr
	
	global __fetch8
	
	global _fetch
	global _fetch8
	global _fetch16
	global _fetch32
	global _fetch64

	global _load
	global _load8
	global _load16
	global _load32
	global _load64

	global _store
	global _store8
	global _store16
	global _store32
	global _store64

	global _assign
	global _assign8
	global _assign16
	global _assign32
	global _assign64

	global _add
	global _add8
	global _add16
	global _add32
	global _add64

	global _sub
	global _sub8
	global _sub16
	global _sub32
	global _sub64

	global _test
	global _not
	global _neg
	global _mul
	global _imul
	global _div
	global _idiv
	
	global _and
	global _or

	global _xor
	global _xor8
	global _xor16
	global _xor32
	global _xor64
	
	
	global _cmp

	global _rol
	global _ror
	global _shl
	global _shr

	global _inc
	global _dec
	global _call
	global _jmp
	global _push
	
	global _shl0
	global _shl1
	global _shl2
	global _shl3
	global _shl4
	global _shl5
	global _shl6
	global _shl7
	global _shl8	
	
	extern _rax
	extern _rcx
	extern _rdx
	extern _rbx
	extern _rsp
	extern _rbp
	extern _rsi
	extern _rdi
	extern _r8
	extern _r9
	extern _r10
	extern _r11
	extern _r12
	extern _r13
	extern _r14
	extern _r15

	extern _eflags
	extern _rip

	extern _debug._inst
	extern _debug._offset
	
	extern _context._dflag
	extern _context._rex
	extern _context._arg1
	extern _context._arg2
	extern _context._rm
	extern _context._reg	
	extern _context._res
	
	extern print
	extern get_diff_host_guest_addr
	extern _get_host_addr_from_guest

	extern _context._internal_arg1
	extern _gen_push
	
	extern _op_shl_base

%include "constant.asm"

;;; this fetch 8 does not increment instruction pointer aiming for debugging purpose.
__fetch8:
	push rbp
	mov qword [_context._res],0x00
	mov rax,[_rip]
	call _get_host_addr_from_guest
	mov al,[rax]
	mov [_context._res],al
	;; for debugging
	lea rdx,[_debug._inst]
	add dl,[_debug._offset]
	mov byte [rdx],al
	add byte [_debug._offset],1
	;; no incrementation
	pop rbp
	ret

_fetch:
	push rbp
	mov ax,0
	mov al,[_context._dflag]
	lea rdx,[_fetch_base]
	add dx,ax
	call [rdx]
	pop rbp
	ret

_fetch8:
	push rbp
	mov qword [_context._res],0x00
	mov rax,[_rip]
	call _get_host_addr_from_guest
	mov al,[rax]
	mov [_context._res],al
	;; for debugging
	lea rdx,[_debug._inst]
	add dl,[_debug._offset]
	mov [rdx],al
	add byte [_debug._offset],1
	;; increment
	add dword [_rip],1
	pop rbp
	ret

_fetch16:
	push rbp
	mov qword [_context._res],0x00
	mov rax,[_rip]
	call _get_host_addr_from_guest
	mov ax,[rax]
	mov [_context._res],ax
	;; for debugging
	lea rdx,[_debug._inst]
	add dl,[_debug._offset]
	mov [rdx],ax
	add byte [_debug._offset],2
	;; 
	add dword [_rip],2
	pop rbp
	ret

_fetch32:
	push rbp
	mov qword [_context._res],0x00	
	mov rax,[_rip]
	call _get_host_addr_from_guest
	mov eax,[rax]
	mov [_context._res],eax	
	;; for debugging
	lea rdx,[_debug._inst]
	add dl,[_debug._offset]
	mov [rdx],eax
	add byte [_debug._offset],4
	;; 
	add dword [_rip],4
	pop rbp
	ret

_fetch64:
	push rbp
	mov qword [_context._res],0x00
	mov rax,[_rip]
	call _get_host_addr_from_guest
	mov rax,[rax]
	mov [_context._res],rax
	;; for debugging
	lea rdx,[_debug._inst]
	add dl,[_debug._offset]
	mov [rdx],rax
	add byte [_debug._offset],8
	;; incrementation
	add dword [_rip],8
	pop rbp
	ret

	
;;;  you have	

_load:
	push rbp
	mov ax,0
	mov al,[_context._dflag]
	lea rdx,[_load_base]
	add dx,ax
	call [rdx]
	pop rbp
	ret
	
_load8:
	push rbp
	mov qword [_context._res],0x00
	mov rax,[_context._arg1]
	call _get_host_addr_from_guest	
	mov rdx,0
	mov dl,[rax]
	
	mov byte [_context._res],dl
	
	pop rbp
	ret
	
_load16:
	ret
	
_load32:
	push rbp
	mov rax,[_context._arg1]
	;; this is util function which will 
	call _get_host_addr_from_guest
	mov rax,[rax]
	mov [_context._res],rax
	pop rbp
	ret
	
_load64:
	push rbp
	mov rax,[_context._arg1]
	;; this is util function which will 
	call _get_host_addr_from_guest
	mov rax,[rax]
	mov [_context._res],rax
	pop rbp
	ret

_store:
	push rbp
	mov ax,0
	mov al,[_context._dflag]
	lea rdx,[_store_base]
	add dx,ax
	call [rdx]
	pop rbp
	ret
	
_store8:
	push rbp
	mov rax,[_context._arg1]
	call _get_host_addr_from_guest
	mov rdx,[_context._arg2]
	mov byte [rax],dl
	pop rbp
	ret
	
_store16:
	push rbp
	mov rax,[_context._arg1]
	call _get_host_addr_from_guest
	mov rdx,[_context._arg2]
	mov [rax],rdx
	pop rbp	
	ret
	
_store32:
	push rbp
	mov rax,[_context._arg1]
	call _get_host_addr_from_guest
	mov rdx,[_context._arg2]
	mov [rax],rdx
	pop rbp	
	ret
	
_store64:
	push rbp
	mov rax,[_context._arg1]
	call _get_host_addr_from_guest
	mov rdx,[_context._arg2]
	mov [rax],rdx
	pop rbp
	ret

;;; assign means setting a value on a register.
;;  assign is identical with store because
;;; in this virual condition, all registers are put on
;;; on a designated memory.
;;; Nevertheless, the distiguinshment is needed for
;;; measuring the count of two operations.

_assign:
	push rbp
	mov ax,0
;;; 
	mov r8,0x88
	call print

	lea rdx,[_assign_base]
	mov al,[_context._dflag]
	add dx,ax
	call [rdx]
	pop rbp
	ret
	
_assign8:
	push rbp
	mov rcx,0
	mov rax,[_context._arg1]
	mov cl,[_context._arg2]
	mov [rax],cl
	mov r8,0xb6
	call print
	
	mov r8,[_context._arg1]
	call print
	mov r8,[_context._rm]
	call print
	mov r8,[_context._arg2]
	call print
	
	pop rbp
	ret
	
_assign16:
	ret
	
_assign32:
	push rbp
	mov r8,[_context._arg1]
	call print
	mov r8,[_context._arg2]
	call print
	mov rax,[_context._arg1]
	mov ecx,[_context._arg2]
	mov [rax],ecx
	pop rbp
	ret
	
_assign64:
	push rbp
	mov rax,[_context._arg1]
	mov rcx,[_context._arg2]
	mov [rax],rcx
	pop rbp
	ret

;;; addition with signed

_add:
	push rbp
	mov ax,0
	mov al,[_context._dflag]
	lea rdx,[_add_base]
	add dx,ax
	call [rdx]
	pop rbp
	ret
	
_add8:
	mov qword [_context._res],0
	mov rax,[_context._arg1]
	mov bl,[_context._arg2]
	add al,bl
	mov [_context._res],rax
	;; [_context._res]
	mov r8,[_context._arg1]
	call print
	
	mov r8,[_context._res]
	call print
	ret

_add16:
	mov qword [_context._res],0
	mov al,[_context._arg1+6]
	mov bl,[_context._arg2+6]
	add al,bl
	mov [_context._res],ax
	ret
	
_add32:
	mov rax,[_context._arg1]
	mov edx,[_context._arg2]
	add eax,edx
	mov [_context._res],rax
	ret
	
_add64:
	mov rax,[_context._arg1]
	mov rdx,[_context._arg2]
	add rax,rdx
	mov [_context._res],rax
	ret

_sub:
	push rbp
	mov ax,0
	mov al,[_context._dflag]
	lea rdx,[_sub_base]
	add dx,ax
	call [rdx]
	pop rbp
	ret

_sub8:
	mov qword [_context._res],0
	mov rax,[_context._arg1]
	mov bl,[_context._arg2]
	sub al,bl
	mov [_context._res],rax
	ret
	
_sub16:
	ret
	
_sub32:
	push rbp
	mov rax,[_context._arg1]
	mov rdx,[_context._arg2]
	sub rax,rdx
	mov [_context._res],rax
	pop rbp
	ret
	
_sub64:
	push rbp
	mov rax,[_context._arg1]
	mov rdx,[_context._arg2]
	sub rax,rdx
	mov [_context._res],rax
	pop rbp
	ret

_and:
	ret

_or:
	ret

_xor:
	push rbp
	mov rax,[_context._arg1]
	mov rdx,[_context._arg2]
	xor rax,rdx
	mov [_context._res],rax
	pop rbp	
	ret

_xor8:
	ret

_xor16:
	ret

_xor32:
	ret

_xor64:
	ret

_cmp:
	push rbp
	mov rax,[_context._arg1]	
	mov rdx,[_context._arg2]
	;; sign , overflow , parity , zero , carry

	;; carry flag
	;; left most bits added.
	;; left most bits subtracted.

	;; overflow flag is turned on if 
	
	sub rax,rdx
	;; set zero flag if the computed result equals 0
	lea r15,[_cmp.done1]
	cmp eax,0
	je _set_zeroflags
.done1:
	
	mov r8,[_eflags]
	call print
	
	pop rbp
	ret
	
_set_zeroflags:
	mov qword [_eflags],eflags_zf
	;; or qword [_eflags],eflags_zf
	jmp r15
	
_update_eflags:
	mov qword [_eflags],0x00
	ret
	
_rol:
	ret
	
_ror:
	ret

_shr:
	ret
	
_shl:
	push rbp
	mov rax,[_context._arg1]	
	mov rdx,_op_shl_base
	mov rcx,[_context._arg2]
	shl rcx,0x03
	add rdx,rcx
	call [rdx]
	mov [_context._res],rax
	pop rbp
	ret
	
_shl0:
	ret

_shl1:
	shl rax,1	
	ret
	
_shl2:	
	shl rax,2
	ret
	
_shl3:
	shl rax,3
	ret
	
_shl4:
	shl rax,4
	ret
	
_shl5:
	shl rax,5
	ret
	
_shl6:	
	shl rax,6
	ret
	
_shl7:
	shl rax,7
	ret
	
_shl8:
	shl rax,8
	ret

_test:
	
	ret

_not:
	mov r8,0xff
	call print
	ret

_neg:
	ret

_mul:
	ret

_imul:
	ret

_div:
	ret

_idiv:
	ret
	

_inc:
	ret

_dec:
	ret

_call:
	push rbp
	;; addition of displacement & register is assumed to be set on
	;; [_context._arg1] already.
	;; you can get the value, and jump on it.
	
	mov rax,[_context._res]

	mov rdi,[_rip]
	mov rsi,rax
	
	call _check_on_iat

	mov rdi,[_rip]	
	mov rsi,[_context._res]	
	call _find_f_addr
	
	mov r8,rax
	call print
	
	;; before adding rax, you need to store rip to be returned on it.

	;; mov rdx,[_rip]
	;; mov [_rip],rax
	
	;; mov [_context._internal_arg1],rdx
	;; call _gen_push
	
	pop rbp
	
	ret
	
_jmp:
	ret

_push:
	ret
	