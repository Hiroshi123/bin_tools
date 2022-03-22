
	default rel

	section .text
	global _0xa0_mov
	global _0xa1_mov
	global _0xa2_mov
	global _0xa3_mov
	global _0xa4_movsS	
	global _0xa5_movsS
	global _0xa6_cmpsS	
	global _0xa7_cmpsS
	global _0xa8_test	
	global _0xa9_test
	global _0xaa_stosS
	global _0xab_stosS
	global _0xac_lodsS
	global _0xad_lodsS
	global _0xae_scasS
	global _0xaf_scasS

%include "extern_for_inst.asm"

;;; this A0/A1 loads seg+off to AL/AX/EAX/RAX.
_0xa0_mov:
	add dword [_rip],1
	call _fetch8
	mov rax,[_context._res]
	mov [_context._imm_val],rax
	mov rax,_rax
	mov [_context._rm],rax	
	mov byte al,0b00011000
	mov byte [_context._mod],al
	ret
_0xa1_mov:
	add dword [_rip],1
	call _fetch32
	mov rax,[_context._res]
	mov [_context._imm_val],rax
	mov rax,_rax
	mov [_context._rm],rax
	mov byte al,0b00011000
	mov byte [_context._mod],al
	ret
	
;;; this A0/A1 stores AL/AX/EAX/RAX to seg+off.
_0xa2_mov:
	add dword [_rip],1
	mov byte [_context._is_rm_src],0xff
	call _fetch8
	mov rax,[_context._res]
	mov [_context._imm_val],rax
	mov rax,_rax
	mov [_context._rm],rax	
	mov byte al,0b00011000
	mov byte [_context._mod],al
	ret
_0xa3_mov:
	add dword [_rip],1
	mov byte [_context._is_rm_src],0xff
	call _fetch32
	mov rax,[_context._res]
	mov [_context._imm_val],rax
	mov rax,_rax
	mov [_context._rm],rax
	mov byte al,0b00011000
	mov byte [_context._mod],al
	ret

_0xa4_movsS:
	add dword [_rip],1
._b1:
	;; load value of address which rsi indicates
	mov rsi,[_rsi]
	mov [_context._arg1],rsi
	call _load8
	call _mov_res_to_arg2
	mov rdi,[_rdi]
	mov [_context._arg1],rdi
	call _store8	
	;; update source & destination memory
	call _update_rdi_rsi
	;; check rep prefix & should loop back.
	mov r8,_0xa4_movsS._b1
	call _check_rep
	;; if the r8 is set as 0, do not jump but step one up.
	jmp r8
	ret
	
_0xa5_movsS:
	add dword [_rip],1
	ret
	
	call _set_dflag
._b1:
	;; load value of address which rsi indicates
	mov rsi,[_rsi]
	mov [_context._arg1],rsi
	call _load
	call _mov_res_to_arg2
	mov rdi,[_rdi]
	mov [_context._arg1],rdi
	call _store
	;; update source & destination memory
	call _update_rdi_rsi
	;; check rep prefix & should loop back.
	mov r8,_0xa5_movsS._b1
	call _check_rep
	;; if the r8 is set as 0, do not jump but step one up.
	jmp r8
	ret
	
_0xa6_cmpsS:
	add dword [_rip],1
	ret
._b1:
	;; load value of address which rsi indicates
	mov rsi,[_rsi]
	mov [_context._arg1],rsi
	call _load8
	call _mov_res_to_arg2
	mov rdi,[_rdi]
	mov [_context._arg2],rdi
	call _load8
	call _mov_res_to_arg1
	call _cmp8
	;; update source & destination memory
	call _update_rdi_rsi
	;; check rep prefix & should loop back.
	mov  al,[_context._res]
	test al,al
	;; if the result of cmp is not equal,
	;; then stop it.
	jne  _0xa6_cmpsS._b2
	mov r8,_0xa6_cmpsS._b1
	call _check_rep
	;; if the r8 is set as 0, do not jump but step one up.
	jmp r8
._b2:
	ret
	
_0xa7_cmpsS:
	add dword [_rip],1
	call _set_dflag
._b1:
	;; load value of address which rsi indicates
	mov rsi,[_rsi]
	mov [_context._arg1],rsi
	call _load
	call _mov_res_to_arg2
	mov rdi,[_rdi]
	mov [_context._arg2],rdi
	call _load
	call _mov_res_to_arg1
	call _cmp
	;; update source & destination memory
	call _update_rdi_rsi
	;; check rep prefix & should loop back.
	mov  al,[_context._res]
	test al,al
	;; if the result of cmp is not equal,
	;; then stop it.
	jne  _0xa7_cmpsS._b2
	mov r8,_0xa7_cmpsS._b1
	call _check_rep
	;; if the r8 is set as 0, do not jump but step one up.
	jmp r8
._b2:
	ret

;;; this test also requires imm and fixed al
_0xa8_test:
	add dword [_rip],1
	call _set_dflag_as_1byte
	mov rax,_rax
	mov [_context._rm],rax
	call _fetch8
	mov rax,0
	mov al,[_context._res]
	mov qword [_context._imm_val],rax
	ret

;;; this test also requires imm and fixed ax/eax
_0xa9_test:
	add dword [_rip],1
	mov byte [_context._mod],0b00011000
	call _set_dflag
	mov rax,_rax
	mov [_context._rm],rax
	call _fetch32
	mov eax,[_context._res]
	mov qword [_context._imm_val],rax
	;; mov [_context._rm],0x1
	ret

_0xaa_stosS:
	add dword [_rip],1
	;; call _set_dflag_as_1byte
._b1:
	;; store value
	mov rax,[_rax]
	mov rdi,[_rdi]
	mov [_context._arg1],rdi
	mov [_context._arg2],rax
	call _store8
	;; update destination memory
	call _update_rdi
	;; if rcx is still above 0, go on.
	mov r8,_0xaa_stosS._b1
	call _check_rep
	;; if the r8 is set as 0, do not jump but step one up.
	jmp r8
	ret

_0xab_stosS:
	add dword [_rip],1
	call _set_dflag
	call _select_reg
	ret
._done1:
	;; store value
	mov rax,[_rax]
	mov rdi,[_rdi]
	mov [_context._arg1],rdi
	mov [_context._arg2],rax
	call _store
	;; update destination memory
	call _update_rdi
	;; if rcx is still above 0, go on.
	mov r8,_0xab_stosS._done1
	call _check_rep
	;; if the r8 is set as 0, do not jump but step one up.
	jmp r8
	ret

_0xac_lodsS:
	add dword [_rip],1
._b1:
	;; load value of address which rsi indicates
	mov rsi,[_rsi]
	mov [_context._arg1],rsi
	call _load8
	call _mov_res_to_arg2
	mov rdi,_rdi
	mov [_context._arg1],rdi
	call _assign8
	;; update source & destination memory
	call _update_rsi
	;; check rep prefix & should loop back.
	mov r8,_0xac_lodsS._b1
	call _check_rep
	;; if the r8 is set as 0, do not jump but step one up.
	jmp r8
	ret

_0xad_lodsS:
	add dword [_rip],1
	call _set_dflag
._b1:
	;; load value of address which rsi indicates
	mov rsi,[_rsi]
	mov [_context._arg1],rsi
	call _load
	call _mov_res_to_arg2
	mov rdi,_rdi
	mov [_context._arg1],rdi
	call _assign
	;; update source & destination memory
	call _update_rsi
	;; check rep prefix & should loop back.
	mov r8,_0xad_lodsS._b1
	call _check_rep
	;; if the r8 is set as 0, do not jump but step one up.
	jmp r8
	ret
	
_0xae_scasS:
	add dword [_rip],1
._b1:
	;; put value of rax on arg2
	mov rax,[_rax]
	mov [_context._arg1],rax
	call _mov_res_to_arg2
	mov rdi,[_rdi]
	mov [_context._arg2],rdi
	call _load8
	call _mov_res_to_arg1
	call _cmp8
	;; update source & destination memory
	call _update_rdi_rsi
	;; check rep prefix & should loop back.
	mov  al,[_context._res]
	test al,al
	;; if the result of cmp is not equal,
	;; then stop it.
	jne  _0xae_scasS._b2
	mov r8,_0xae_scasS._b1
	call _check_rep
	;; if the r8 is set as 0, do not jump but step one up.
	jmp r8
._b2:
	ret

_0xaf_scasS:
	add dword [_rip],1
	call _set_dflag
._b1:
	;; put value of rax on arg2
	mov rax,[_rax]
	mov [_context._arg1],rax
	call _mov_res_to_arg2
	mov rdi,[_rdi]
	mov [_context._arg2],rdi
	call _load
	call _mov_res_to_arg1
	call _cmp
	;; update source & destination memory
	call _update_rdi_rsi
	;; check rep prefix & should loop back.
	mov  al,[_context._res]
	test al,al
	;; if the result of cmp is not equal,
	;; then stop it.
	jne  _0xaf_scasS._b2
	mov r8,_0xaf_scasS._b1
	call _check_rep
	;; if the r8 is set as 0, do not jump but step one up.
	jmp r8
._b2:
	ret

;;; there are 3types of functions for string operation to update rdi,rsi.
;;; movs/cmps -> _update_rdi_rsi
;;; stos/scas -> _update_rdi
;;; lods      -> _update_rsi
_update_rdi_rsi:
	mov byte al,[_context._repz]
	cmp al,0xff
	je _update_rdi._dec
._inc:
	lea rax,[_dflag_len]	
	add [_rdi],rax
	add [_rsi],rax
	ret
._dec:
	lea rax,[_dflag_len]	
	sub [_rdi],rax
	sub [_rsi],rax
	ret
;;; ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

_update_rdi:
	mov byte al,[_context._repz]
	cmp al,0xff
	je _update_rdi._dec
._inc:
	lea rax,[_dflag_len]	
	add [_rdi],rax
	ret
._dec:
	lea rax,[_dflag_len]	
	sub [_rdi],rax
	ret
;;; ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
_update_rsi:
	mov byte al,[_context._repz]
	cmp al,0xff
	je _update_rsi._dec
._inc:
	lea rax,[_dflag_len]	
	add [_rsi],rax
	ret
._dec:
	lea rax,[_dflag_len]	
	sub [_rsi],rax
	ret

;;; this function assumes that r8 is already set where you wanna return
;;; after jumping.
;;; if rep prefix is not set or rcx is decremented down to 0, the value on r8
;;; will be changed to 0 that says no more jump back.
_check_rep:
	mov byte al,[_context._repz]
	cmp al,0xff
	je  _check_rep._b1
	mov r8,0
	ret
._b1:
	sub byte [_rcx],0x01
	mov rcx,[_rcx]
	test rcx,rcx
	;; after decrementation of rcx, check rcx.
	;; if it equals 0, then set r8 0 so that it does not come
	;; to another loop.
	jne _check_rep._b2
	mov r8,0
._b2:
	ret
	
