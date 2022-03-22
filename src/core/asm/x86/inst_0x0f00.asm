
	default rel
	section .text
	global _0x0f00_ltr
	global _0x0f01_lgdt_lidt
	global _0x0f05_syscall

%include "extern_for_inst.asm"

;;; sldt (local discriptor table)
;;; str (store task register)
;;; lldt 
;;; ltr 
;;; verr verify a segment for reading
;;; verw verify a segment for writing
  
;;; load task register.
;;; On global descriter table, 
_0x0f00_ltr:
	add dword [_rip],0x01
	call _get_mod_reg_rm
	
	ret

;;; if the value which sits on the equivalent position of reg
;;; will be 2(_rdx), load_gdt, else if 3(_rbx), load_idt,
;;; otherwise, error.

;;; There are two kinds of descriptor table which needs to be configured when CPU is initialized.

;;; 1. Global Descriptor table
;;; mod & rm will be used for setting the head address of descriptor table.
;;; 64*6(KCODE/KDATA/UCODE/UDATA/TSS)
;;; Note that TSS (Task State Segment) will be initialized by ltr(0f00) on every time that task segment is updated
;;; such as system call or context switch.
;;; 
;;; 2. Interrupt Descriptor table
;;; Interrupt descriptor table is initialized by each CPU.
;;; in order to let hardware interrupt work out properly,
;;; initialization of lapic on each CPU and ioapic which will distribute each hardware interrupt
;;; which had been delieved another IC is necessary.
;;; BIOS will do that and map mp configuration table which put register of ioapic & ioapic.

_0x0f01_lgdt_lidt:
	add dword [_rip],0x01
	call _get_mod_reg_rm
	mov rax,[_context._reg]
	cmp rax,_rdx
	je _load_gdt
	cmp rax,_rbx
	je _load_idt
	ret

;;; segment descriptor will be loaded
_load_gdt:
	ret

;;; gate descriptor will be loaded
_load_idt:
	ret

_0x0f05_syscall:
	add dword [_rip],0x01
	;; db 0xeb,0xfe
	ret
	
