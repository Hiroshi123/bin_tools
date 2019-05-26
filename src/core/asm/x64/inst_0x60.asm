
	default rel

	section .text

	global _0x60_pusha
	global _0x61_popa
	global _0x62_bound
	global _0x63_movslS
	global _0x64_prefix_seg_fs

	global _0x65_prefix_seg_gs
	global _0x66_prefix_data
	global _0x67_prefix_addr
	
	global _0x68_push_iv
	global _0x69_imul
	global _0x6a_push_iv
	global _0x6b_imul
	global _0x6c_insS
	global _0x6d_insS
	global _0x6e_outsS
	global _0x6f_outsS
	
	extern _rax
	extern _rsp
	extern _rip

	extern _fs
	extern _gs
	
	extern print
	extern _exec_one
	
	extern _context._arg1
	extern _context._arg2

	extern _context._data_prefix
	extern _context._res
	
	extern _context._internal_arg1
	
	extern _context._override
	extern _context._override_reg

	extern _fetch
	extern _fetch8
	extern _fetch32

	extern _gen_push
	
_0x60_pusha:
	ret
	
_0x61_popa:
	ret

_0x62_bound:
	ret
	
_0x63_movslS:
	ret
	
_0x64_prefix_seg_fs:

	add dword [_rip],1
	mov byte [_context._override],0xff
	mov rax,_fs
	mov [_context._override_reg],rax
	jmp _exec_one
	
_0x65_prefix_seg_gs:	
	add dword [_rip],1
	mov byte [_context._override],0xff
	mov rax,_gs
	mov [_context._override_reg],rax
	jmp _exec_one
	
_0x66_prefix_data:

	push rbp
	mov byte [_context._data_prefix],0xff
	
	pop rbp
	;; mov cs,[fs:2]
	;; mov ds,rdx
	;; mov ds,edx
	;; mov [fs:edx+3],eax
	;; mov es,dx
	;; mov fs,dx
	;; mov gs,dx
	;; mov rax,[cs:2]
	;; mov rax,[ds:edx]
	;; lea rax,[es:edx]
	;; lea rax,[ss:edx]
	
	;; mov es:cx,15h
	ret

_0x67_prefix_addr:
	ret
	
_0x68_push_iv:
	push rbp
	add dword [_rip],0x1
	call _fetch32
	mov rax,[_context._res]
	mov [_context._internal_arg1],rax
	call _gen_push
	pop rbp
	ret
	
_0x69_imul:
	ret

_0x6a_push_iv:
	add dword [_rip],0x1
	call _fetch8
	mov rax,[_context._res]
	mov [_context._internal_arg1],rax
	call _gen_push
	ret

_0x6b_imul:
	ret

;;; ins,outs instruction
;;; which will deal with IO port with string instruction.
_0x6c_insS:
	ret

_0x6d_insS:
	ret

_0x6e_outsS:
	ret

_0x6f_outsS:
	ret

	
