
	global __get_ntdll_handle_64
	global __get_ntdll_handle_64_end
	global __enumerate_import_table_2
	global __enumerate_import_table_2_end
	global __enumerate_export_table_2
	global __enumerate_export_table_2_end
	section .text

;;; ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

__get_ntdll_handle_64:
;;;  mov rax, [fs:0x30] for 32bit
	mov rax,[gs:0x60 ]
	mov rax,[rax+0x18]
	mov rax,[rax+0x10]
	mov rax,[rax+0x0 ]
	mov rax,[rax+0x30]
	ret

__get_ntdll_handle_64_end:
	nop

;;; ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

__enumerate_import_table_2:
	call _get_ldr_data_table_entry_1
	mov rcx,[rax+0x30]
	call _get_import_entry_from_handle_1
	;;  let 3rd 2nd
	mov r8,rdx
	;;  set 2nd argument
	mov rdx,rax
	call _enumerate_import_table_3
	;;  mov rax,[rax]	
	ret

_get_import_entry_from_handle_1:
	mov rax,rcx
	mov rbx,0
	mov ebx,[rax+0x3c]
	add ebx,0x90
	;;  add rax,0x18 + 0x70
	mov ebx,[rax+rbx]
	add rax,rbx
	ret
	
_get_ldr_data_table_entry_1:
	mov rax,[gs:0x60]
;;;  access peb_led_data
	mov rax,[rax+0x18]
;;;  in load order module list
	mov rax,[rax+0x10]
	jmp _get_ldr_data_table_entry_1._l1
	._l0:
	mov rax,[rax+0x0]
	dec rcx
	._l1:
	cmp rcx,0
	jne _get_ldr_data_table_entry_1._l0
	ret
	
enu_get_ldr_data_table_entry_1:

	mov rax,[gs:0x60]
	;;  access peb_led_data
	mov rax,[rax+0x18]
	;;  in load order module list
	mov rax,[rax+0x10]
	jmp _get_ldr_data_table_entry_1._l1
._l0:
	mov rax,[rax+0x0]
	dec rcx
._l1:
	cmp rcx,0
	jne _get_ldr_data_table_entry_1._l0
	ret

;;; ; 1st :: base address
;;; ; 2nd ;; import entry
;;; ; 3rd :: pointer to callback
_enumerate_import_table_3:
;;;  base of module
	mov r13,rcx
;;;  heads of import table
	mov r14,rdx
;;;  call back address
	mov r15,r8
;;; ; ;;;;;;;;; start to read each value of import descripor
	._l0:
;;;  import name table entry
	mov rdi,0
	mov edi,[r14+0x00]
;;;  add module base
	add rdi,r13
;;;  import address table entry
	mov rsi,0
	mov esi,[r14+0x10]
;;;  add module base
	add rsi,r13
	mov rbx,0
	mov ebx,[r14+0x0c]
	mov r12,rbx
	add r12,r13
	._l1:
;;;  import name table
	mov r8,[rdi]
	add r8,r13
	add r8,0x2
;;;  import address table
	mov r9,[rsi]
	add r9,r13
	add r9,0x2

;;;  if import address table & import name table points to same address,
;;;  it means this is the end of an import address & name table on
;;;  an image import descriptor.
	cmp r8,r9
	je _enumerate_import_table_3._l2
	mov rdx,r12
	mov rcx,r14

	sub rsp,0x20
	call r15
	add rsp,0x20
;;;  increment import address table & import name table
	add qword rdi,0x8
	add qword rsi,0x8
	jmp _enumerate_import_table_3._l1
	._l2:
	add r14,0x14
;;;  the end of import descriptor table is null
	cmp qword [r14],0
	jne _enumerate_import_table_3._l0
	._l3:
	ret

;;; ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

__enumerate_import_table_2_end:

__enumerate_export_table_2:
	call _get_ldr_data_table_entry_1
	mov rcx,[rax+0x30]
	call _get_export_entry_from_handle_1
;;;  let 3rd 2nd
	mov r8,rdx
;;;  set 2nd argument
	mov rdx,rax
	call _enumerate_export_table_3
	ret

_get_export_entry_from_handle_1:
	mov rax,rcx
	mov rbx,0
	mov ebx,[rax+0x3c]
	add ebx,0x88
	;; add rax,0x18 + 0x70
	mov ebx,[rax+rbx]
	add rax,rbx
	ret

_enumerate_export_table_3:
;;;  r13 == module_base
	mov rsi,rcx
;;;  head of entry directory
	mov rdi,rdx
	mov rax,0
;;;  entry directory heads + 0x20 is the first export name table
	mov eax,[rdi+0x20]
	add rax,rsi
;;;  r12 == index
	mov r12,0
;;;  r15 [pointer to callback]
	mov r15,r8
;;;  head of export name table
	mov r14,rax
;;;  r13 == number of entry on name index table
	mov rbx,0
	mov ebx,[rdi+0x18]
	mov r13,rbx
	._l0:
;;;  1st argument
	mov rcx,rsi
;;;  2nd arugment
	mov rdx,rdi
;;;  3rd argument[function address]
	mov r8,r12
	call _get_faddr_by_index_3

;;;  as callee volatile rdi & rsi,
;;;  original value needs to be reset here.
	mov rdi,rdx
	mov rsi,rcx

	;;  2nd arg
	mov rdx,rax

	mov rbx,0
	mov ebx,[r14+r12*4]
	add rbx,rsi

	;;  1st arg
	mov rcx,rbx

	sub rsp,0x20
	call r15
	add rsp,0x20

	add r12,1
	cmp r12,r13
	jne _enumerate_export_table_3._l0
	mov rax,rbx
	ret

;;; ; 3 argument
;;; ; 1st[rcx] :: base address of the module
;;; ; 2nd[rdx] :: base address of export entry directory
;;; ; 3rd[r8 ] :: function index
;;; ; return value[rax] :: function address
_get_faddr_by_index_3:
	mov rdi,0
	mov rsi,0
;;;  directory entry + 0x1c (address of functions,[4byte])
	mov edi,[rdx+0x1c]
;;;  directory entry + 0x24 (address of ordinals [4byte])
	mov esi,[rdx+0x24]
;;;  add module base as they are relative.
	add rdi,rcx
	add rsi,rcx
;;;  from function index to function ordinal
	shl r8,1
	mov rbx,0
	mov bx,[rsi+r8]
;;;  from function ordinal to function address
	shl rbx,2
	mov rax,0
	mov eax,[rdi+rbx]
	add rax,rcx
;;;
	ret

__enumerate_export_table_2_end:	