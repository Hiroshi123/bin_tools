
	default rel
	section .text

	global _get_host_rax
	global _set_host_rax
	global _exec
	global _exec_one	
	global _set_rip
	global _set_rsp
	global _get_rip
	global _get_rsp
	global _get_host_rsp
	
	global _init_regs
	global _gen_code
	global _get_mod_reg_rm
	global _get_mod_op_rm
	
	global _set_eflags
	global setrip

	global _get_host_addr_from_guest
	extern get_diff_host_guest_addr
	
	extern _opcode_table
	extern _context
	extern _context._rex
	extern _context._mod
	extern _context._reg
	extern _context._rm
	extern _context._scale
	extern _context._index
	extern _context._base
	extern _context._dflag
	extern _context._aflag
	extern _context._arg1
	extern _context._arg2	
	extern _context._res
	
	extern _context._data_prefix
	extern _context._addr_prefix

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
	extern _rip
	extern _eflags	

	extern print
	
	extern _op01_f_base

	extern _sub
	extern _add
	extern _or

	extern _0xe8_call
	
;;; ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

_get_host_rsp:
	mov rax,rsp
	ret

_get_rip:
	mov rax,[_rip]
	ret
_get_rsp:
	mov rax,[_rsp]
	ret

_set_rip:
	mov [_rip],rdi
	ret
_set_rsp:
	mov [_rsp],rdi
	ret
	
_init_regs:
	ret

_exec_one:
	push rbp
	lea rax,[_opcode_table]
	mov r8,rax
	call print

	mov rbx,0x00
	mov rdx,[_rip]
	mov bl,[rdx]	
	;; shl rbx,0x03
	imul rbx,0x08

	adc rax,rbx
	
	mov r8,0x11
	call print
	mov r8,[rax]
	call print

	mov r8,_0xe8_call
	call print	

	call [rax]
	;; add byte [_rip],0x01
	mov r8,0xff
	call print

	pop rbp
	ret
	
_exec:
	push rbp
	mov rax,_opcode_table
	mov rbx,0x00
	mov rdx,[_rip]
	mov bl,[rdx]
	imul rbx,0x08
	add rax,rbx
	call [rax]
	add dword [_rip],0x01
	;; and al,0x10
	;; mov r8,0xcd2145d2
	;; call print
	;; add rax,[rdi]
	;; add rax,0x00
	;; add [_rax],rax
	;; mov qword [print_str],rax
	;; call _write
	pop rbp
	ret	

	
_get_host_rax:
	mov rax,rax
	ret
	
_set_host_rax:
	mov rax,rdi
	ret

_asm_add:	
	enter 0,0
	mov rax, rdi
	mov ebx, esi
	add eax, ebx
	leave
	ret

;;; what needs to be done here is
;;; 1. set absolute address
;;; 2. if mod == 00, set base address to load
;;; 3. if mod == 01 or 10, set base address to displacement
;;; 4. if mod == 11, set base address to f( if you have immidiate or SIB)

;;; 
;;; SIB
;;; mod should be just fed the size of displacement if it has....
;;; displacement
;;; immidiate(operation dependent..)
;;; f
;;; store/assign will be dependent to mod...

;;; no matter you have displacement or not,
;;; base index will be set to before loading.

;;; add [eax],eax (01,00)
;;; add eax,[eax] (03,00)

;;; load ::  mov eax,[eax]
;;; store :: mov [eax],eax

;;; set base address

;;; arg1 or arg2

;;; if arg1 && mod00, then
;;; it should be load & store????
;;; if arg2 && mod00,
;;; load & assign

;;; arg1 && mod00
;;; store 

;;; arg2 && mod00
;;; load & mov

_mod00_do:
	ret
	
_mod01_do:
	ret
	
_mod10_do:
	ret
	
_mod11_do:
	ret


_set_scale_index_base:
	ret

_set_immidiate:
	ret

	
;; _gen_code:
;; 	push rbp 
;; 	mov r8,[_context._gen_code_base]
;; 	mov rax,[r8]
;; 	call [rax]
;; 	add r8,32
;; 	pop rbp
;; 	ret
;; 	jmp _gen_code
	

;; _hello_pika:
;; 	mov rax, 0x2000004 ; write
;; 	mov rdi, 1 ; stdout
;; 	mov rsi, msg2
;; 	mov rdx, msg2.len
;; 	syscall
;; 	ret
	
;; _write:
;; 	mov rax, 0x2000004 ; write
;; 	mov rdi, 1 ; stdout
;; 	mov rsi, print_0x
;; 	mov rdx, 1
;; 	syscall
;; 	ret
	
_set_op01_f_base:
	push rbp
	mov rdx,_op01_f_base
	mov r9,0x00
	mov r9b,al
	and r9b,0b00111000	
	add dx,r9w
	mov rdx,[rdx]
	mov [_context._reg], rdx
	pop rbp
	ret
	
_get_mod_op_rm:
	push rbp
	;; fetched mod/reg/rm data is set on al(1byte)
	mov rax,[_rip]
	mov byte al,[rax]
	;; rex prefix is set on bl(1byte)
	mov bl,[_context._rex]
	
	call _set_mod
	call _set_op01_f_base
	
	call _set_rm
	call _set_dflag
	call _set_aflag
	
	pop rbp
	ret
	
_get_mod_reg_rm:
	push rbp
	;; fetched mod/reg/rm data is set on al(1byte)
	mov rax,[_rip]
	mov byte al,[rax]
	;; rex prefix is set on bl(1byte)
	mov bl,[_context._rex]

	call _set_mod
	call _set_reg
	call _set_rm
	call _set_dflag
	call _set_aflag

	pop rbp
	ret

_set_mod:
	mov r8b,al
	and r8b, 0b11000000
	;; shift should be 3 step right as it should set 8bit between each steps.
	shr r8b, 0x03
	mov [_context._mod],r8b
	ret
	
;; set an offset of register
_set_reg:
	push rbp
	mov r8,0x00
	;; 1. get base of reg
	mov r8b,bl
	lea r12,[_set_reg.done1]
	and r8b,0b00000100
	cmp r8b,0b00000100
	je set_base_reg_ex
	jmp set_base_reg
	;; 2.get kind of reg
.done1:
	mov r9b,al
	and r9b,0b00111000
	;; you do not need to shift as reg kind should
	;; be multipled by 8 regarding the size of each register.
	;; shr r9b,3
	add r8b,r9b
	;; 3.get reg size
	;; call set_register_size
	;; finally fill the vaue on to the memory of reg.
	mov [_context._reg],r8
	pop rbp
	ret
	
set_base_reg:	
	mov r8,_rax
	jmp r12 
	
set_base_reg_ex:
	mov r8,_r8
	jmp r12
	
set_register_size:
	mov r10b,0x00
	;; check rex_prefix is set
	and bl,0b00001000
	cmp bl,0b00001000
	je  set_register_size.done2
	and byte [_context._data_prefix],0xff
	jna  set_register_size.done1
	mov r10b,0x2
.done1:
	add r10b,0x4
.done2:
	add r8b,r10b
	ret
	
;; set an offset of r/m
_set_rm:
	push rbp
	mov r8,0x00
	;; 1. get base of rm
	mov r8b,bl
	lea r12,[_set_rm.done1]	
	and r8b,0b00000001
	cmp r8b,0b00000001
	jne set_base_reg
	jmp set_base_reg_ex
.done1:
	;; 2. get kind of rm
	mov r9b,al
	and r9b,0b00000111
	;; you need to shift left 3times considering size of each register.
	shl r9b,3
	add r8b,r9b
	;; 3. get size of rm
	;; call set_register_size
	mov [_context._rm],r8
	pop rbp
	ret

_set_dflag:
	;; check rex_prefix is set
	mov bl,[_context._rex]
	and bl,0b00001000	
	cmp bl,0b00001000
	je  _set_dflag.done1
	mov dl,[_context._data_prefix]
	and dl,0xff
	cmp bl,0xff
	jne _set_dflag.done2
	mov byte [_context._dflag],0x8
	ret
.done1:	
	mov byte [_context._dflag],0x18
	ret
.done2:
	mov byte [_context._dflag],0x10
	ret
	
_set_aflag:
	;; aflags
	mov dl,[_context._addr_prefix]
	and dl,0xff
	cmp bl,0xff	
	jne _set_aflag.done1	
	and byte [_context._aflag],0x00
.done1:
	and byte [_context._aflag],0x08
	ret

_get_reg_base:
	mov rax,_rax
	mov rax,_r8

_get_rm_base:	
	mov rax,_r8
	mov rax,_r8

;;; this is just a wrapper of get_diff_host_guest_addr
;;; which basically calculates diff of host & guest addr.

_get_host_addr_from_guest:
	push rbp
	mov rdi,rax
	call get_diff_host_guest_addr
	add rax,[_context._arg1]
	pop rbp
	ret
	
;;; jmp instruction

_0x80_jo:
	call _set_eflags	
	jo setrip
	ret
	
_0x81_jno:
	call _set_eflags
	jno setrip
	ret

_0x82_jnae:
	call _set_eflags
	jnae setrip
	ret

_0x83_jnc:
	call _set_eflags
	jnc setrip
	ret

_0x84_je:
	call _set_eflags
	je setrip
	ret
	
_0x85_jne:
	call _set_eflags
	jne setrip
	ret

_0x86_jna:
	call _set_eflags
	jna setrip
	ret

_0x87_jnbe:
	call _set_eflags
	jnbe setrip
	ret

_0x88_js:
	call _set_eflags
	js setrip
	ret

_0x89_jns:
	call _set_eflags
	jns setrip
	ret

_0x8a_jpe:
	call _set_eflags
	jpe setrip
	ret

_0x8b_jpo:
	call _set_eflags
	jpo setrip
	ret

_0x8c_jnl:
	call _set_eflags
	jnl setrip
	ret

_0x8d_jng:
	call _set_eflags
	jng setrip
	ret

_0x8e_jng:
	call _set_eflags
	jng setrip
	ret

_0x8f_jnle:
	call _set_eflags
	jnle setrip
	ret

_set_eflags:
	push qword [_eflags]
	popf
	ret

setrip:
	add [_rip],rdi
	ret
