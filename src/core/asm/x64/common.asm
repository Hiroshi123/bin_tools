
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
	global _set_dflag
	global _set_dflag_as_1byte
	global _set_aflag
	
	global _set_eflags
	global setrip

	global _get_host_addr_from_guest
	global _set_imm_op_base
	global _set_scale_index_base
	global _set_base_reg
	global _select_reg

	global _get_cr_reg_rm
	
	extern _get_diff_host_guest_addr
	
	extern _opcode_table
	extern _debug._offset
	extern _context
	extern _context._opcode
	extern _context._opcode_table

	extern _context._override
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
	extern _context._imm_op
	extern _context._sib_displacement
	extern _context._data_prefix
	extern _context._addr_prefix
	extern _context._rip_rel
	extern _context._internal_arg1

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

	extern _cr0
	
	extern print
	
	extern _sub
	extern _add
	extern _or
	extern _shl
	extern __fetch8	
	extern _fetch8
	extern _fetch32
	
	extern _add32	
	
	extern _0xe8_call

	extern _op_shift_base

	extern _mov_res_to_arg2

	extern _sib_no_fetch_displacement
	extern _sib_fetch8_displacement
	extern _sib_fetch32_displacement
	extern _context._sib
	
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
	;; ;; for debuging, offset should be reset
	;; mov byte [_debug._offset],0
	mov r8,0x1
	call print

	;; debugging purpose to be stopped when not yet implemented.
	call __fetch8
	mov r8,0x2
	call print

	mov rbx,0x00
	mov bl,[_context._res]
	mov [_context._opcode],bl
	mov rax,[_context._opcode_table]
	;; shl rbx,0x03
	imul rbx,0x08
	adc rax,rbx
	call [rax]
	;; add byte [_rip],0x01

	;; following should be reset.
	;; rex prefix(0x4X) / data prefix (0x66) / addr prefix (0x67)
	mov byte [_context._rex],0x00
	mov byte [_context._data_prefix],0x00
	mov byte [_context._addr_prefix],0x00
	mov rax,_opcode_table
	mov [_context._opcode_table],rax
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
	
;;; ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	
;;; if mod == 00/01/10 && rm == 100(rsp)
;;; fetch8 at least
;;; fetch8 or 32 additionally.

;;; scale index base

compute_scale_index:
	
	push rbp
	mov dl,[_context._rm]
	;; scale(shift)
	and dl,0b11000000
	cmp dl,0b00000000
	je compute_scale_index.done2
	rol dl,0x02

	mov qword [_context._arg2],0
	mov [_context._arg2],dl

	;; get base of index register
	lea r12,[compute_scale_index.done1]
	mov r8b,[_context._rex]
	and r8b,0b00000010
	cmp r8b,0b00000010
	je _set_base_reg_ex
	jmp _set_base_reg
.done1:

	;; get kind of index register
	mov dx,0
	mov al,[_context._res]
	mov dl,al
	and dl,0b00111000
	add r8w,dx
	mov r8,[r8]
	mov [_context._arg1],r8
	call _shl
	call _mov_res_to_arg2	
	pop rbp
	ret

	;; [CAUTION] undocumented feature.
	;; if scale == 00, it does not mean, index is set as it is but index ends up with 0.
	;; no matter what is set on index.
.done2:
	mov qword [_context._arg2],0
	pop rbp
	ret

compute_base:

	push rbp
	mov r8,0x99
	call print
	
	;; get base of base register(rax/r8)
	lea r12,[compute_base.done1]
	mov r8b,[_context._rex]
	and r8b,0b00000001
	cmp r8b,0b00000001
	je _set_base_reg_ex
	jmp _set_base_reg
.done1:
	;; get kind of base register
	mov dx,0
	mov dl,[_context._rm]
	and dl,0b00000111
	shl dl,0x03
	add r8w,dx
	mov r8,[r8]
	
	mov rax,_sib_no_fetch_displacement
	mov [_context._sib_displacement],rax

	;; undocumented feature(if rm is not 101, follow the value of mod)
	;; cmp dl,0b00101000
	;; jne compute_base.done4

	;; mod == 01
	mov cl,[_context._mod]
	and cl,0b00001000
	cmp cl,0b00001000
	je compute_base.done2
	;; mod == 10
	mov cl,[_context._mod]
	and cl,0b00010000
	je compute_base.done3
	;; mod == 00
	;; if kind is 101(rbp), fetch displacement32
	cmp dl,0b00101000
	je compute_base.done3
	jmp compute_base.done4	
.done2:
	mov rax,_sib_fetch8_displacement	
	mov [_context._sib_displacement],rax
	jmp compute_base.done4
.done3:
	mov rax,_sib_fetch32_displacement
	mov [_context._sib_displacement],rax
	jmp compute_base.done4	
	
	;; in this case, mod == 00 but, you need to retrieve 32bit displacement
	;; on the subsequent function "fetch_displacement".
	;; if mod == 01/10, let it be as things are worked out with doing nothing.
	
	;; call [compute_base.displacement00]
	
	;; set the value on [_context._arg1] to be added with the result of scale times index.
	
.done4:
	mov [_context._arg1],r8
	mov r8,[_context._arg1]
	call print
	pop rbp
	ret

_set_scale_index_base:

	push rbp
	mov rax,0
	mov al,[_context._mod]
	mov rdx,[_context._rm]
	
	cmp al,0b00011000
	je _set_scale_index_base.done1

	lea rcx, [_rsp]
	
	cmp rdx,rcx
	jne _set_scale_index_base.done1	
	
	mov r8,rcx
	call print

	mov r8,rdx
	call print
	
	call _fetch8
	mov al,[_context._res]
	mov [_context._rm],al
	call compute_scale_index
	call compute_base
	call _add32
	mov rdx, [_context._res]
	mov [_context._rm],rdx
	
	mov byte [_context._sib],0xff

	pop rbp

	ret
	
.done1:
	pop rbp
	ret
	
;;; if mod == 00 && rm == 101(rbp)
;;; fetch32
_set_rip_offset:

	push rbp

	mov r8b,[_context._mod]
	mov r9b,[_context._rm]
	
	cmp r8b,0b00000000
	jne _set_rip_offset.done1
	cmp r9b,0b00101000
	jne _set_rip_offset.done1
	ret
.done1:	
	ret
	
_set_immidiate:
	ret
	
;;; ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	
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

;;; this takes argument r8 which is not good.
;;; needs to be retried from opcode later on.
_set_imm_op_base:
	mov [_context._imm_op], r8
	ret
	
_set_imm_op:
	push rbp
	mov rdx,[_context._imm_op]
	mov r9,0x00
	mov r9b,al
	and r9b,0b00111000
	add dx,r9w
	mov rdx,[rdx]
	mov [_context._imm_op], rdx
	pop rbp
	ret
	
_get_mod_op_rm:
	push rbp
	;; fetched mod/reg/rm data is set on al(1byte)
	call _fetch8
	mov al,[_context._res]
	;; rex prefix is set on bl(1byte)
	mov bl,[_context._rex]
	
	call _set_mod
	call _set_imm_op
	
	call _set_rm
	call _set_dflag
	call _set_aflag
	
	pop rbp
	ret
	
_get_mod_reg_rm:
	push rbp
	;; fetched mod/reg/rm data is set on al(1byte)
	call _fetch8
	mov al,[_context._res]
	;; mov byte al,[rax]
	;; mov rax,[_rip]	
	;; rex prefix is set on bl(1byte)
	mov bl,[_context._rex]

	call _set_mod
	call _set_reg
	call _set_rm
	call _set_dflag
	call _set_aflag
	
	pop rbp
	ret

_get_cr_reg_rm:
	push rbp
	;; fetched mod/reg/rm data is set on al(1byte)
	call _fetch8
	mov al,[_context._res]
	mov bl,[_context._rex]

	call _set_cr_reg
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
	
;;; ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
_set_cr_reg:
	push rbp
	;; 1. get base of reg
	;; simply feeds address of cr0
	mov r8,_cr0
	;; 2.get kind of reg
	mov r9w,0x00
	mov r9b,al
	and r9b,0b00111000
	;; you do not need to shift as reg kind should
	;; be multipled by 8 regarding the size of each register.
	;; shr r9b,3
	add r8w,r9w
	;; 3.get reg size
	;; call set_register_size
	;; finally fill the vaue on to the memory of reg.
	mov [_context._reg],r8
	pop rbp
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
	je _set_base_reg_ex
	jmp _set_base_reg
	;; 2.get kind of reg
.done1:
	mov r9w,0x00
	mov r9b,al
	and r9b,0b00111000
	;; you do not need to shift as reg kind should
	;; be multipled by 8 regarding the size of each register.
	;; shr r9b,3
	add r8w,r9w
	;; 3.get reg size
	;; call set_register_size
	;; finally fill the vaue on to the memory of reg.
	mov [_context._reg],r8
	pop rbp
	ret
	
_set_base_reg:	
	mov r8,_rax
	jmp r12 
	
_set_base_reg_ex:
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
	jne _set_base_reg
	jmp _set_base_reg_ex
.done1:
	;; 2. get kind of rm
	mov r9w,0x00
	mov r9b,al
	and r9b,0b00000111
	;; you need to shift left 3times considering size of each register.
	shl r9b,3
	add r8w,r9w
	;; 3. get size of rm
	;; call set_register_size
	mov [_context._rm],r8
	;; RIP offset check (mod == 00 & rm == 101)
	cmp r9b,0b00101000
	jne _set_rm.done2
	mov r9b,al
	and r9b,0b11000000
	cmp r9b,0b00000000
	jne _set_rm.done2
	mov byte [_context._rip_rel],0xff
.done2:
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

_set_dflag_as_1byte:
	mov byte [_context._dflag],0x0
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

;;; ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

_select_reg:
	lea r12,[_select_reg.done1]
	mov r8b,[_context._rex]
	and r8b,0b00000001
	cmp r8b,0b00000001
	jne _set_base_reg
	jmp _set_base_reg_ex
.done1:
	;; 2. get kind of register
	mov r9,0x00
	mov r9b,[_context._opcode]
	;; only rightest 3bit is needed for register selection.
	and r9b,0b00000111
	;; you need to shift left 3times considering size of each register.
	shl r9b,3
	add r8b,r9b
	ret
	
;;; this is just a wrapper of get_diff_host_guest_addr
;;; which basically calculates diff of host & guest addr.

_get_host_addr_from_guest:
	push rbp
	mov [_context._internal_arg1],rax
	mov rdi,rax
	call _get_diff_host_guest_addr
	add rax,[_context._internal_arg1]
	pop rbp
	ret
	
