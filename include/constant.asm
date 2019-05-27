

;;; eflags carry flag
%define eflags_cf 0x0001
;;; eflags parity flag
%define eflags_pf 0x0004
;;; eflags adjust flag
%define eflags_af 0x0010
;;; eflags zero flag
%define eflags_zf 0x0040
;;; eflags sign flag
%define eflags_sf 0x0080
;;; eflags trap flag
%define eflags_tf 0x0100
;;; eflags interrupt enable flag
%define eflags_if 0x0200
;;; eflags direction flag
%define eflags_df 0x0400
;;; eflags overflow flag
%define eflags_of 0x0800
;;; eflags iopl
%define eflags_iopl 0x3000
;;; eflags nested task flag
%define eflags_nt 0x4000

;;; sign flag & overflow flag
%define eflags_sof 0x0880

;;; carry flag & zero flag
%define eflags_czf 0x0041

%ifidn __OUTPUT_FORMAT__, macho64
	%include "macho_syscall.asm"
%elifidn __OUTPUT_FORMAT__, elf64 
	%define SYS_write     0x1
	%define SYS_mmap	9
	%define SYS_clone	56
	%define SYS_exit	60
%endif

;; unistd.h
%define STDIN		0
%define STDOUT		1
%define STDERR		2

;; sched.h
%define CLONE_VM	0x00000100
%define CLONE_FS	0x00000200
%define CLONE_FILES	0x00000400
%define CLONE_SIGHAND	0x00000800
%define CLONE_PARENT	0x00008000
%define CLONE_THREAD	0x00010000
%define CLONE_IO	0x80000000

;; sys/mman.h
%define MAP_GROWSDOWN	0x0100
%define MAP_ANONYMOUS	0x0020
%define MAP_PRIVATE	0x0002
%define PROT_READ	0x1
%define PROT_WRITE	0x2
%define PROT_EXEC	0x4

%define THREAD_FLAGS \
	CLONE_VM|CLONE_FS|CLONE_FILES|CLONE_SIGHAND|CLONE_PARENT|CLONE_THREAD|CLONE_IO

%define STACK_SIZE	(4096 * 1024)

%define MAX_LINES	1000000	; number of output lines before exiting


;;; x86 trap and interrupt constants.
;;; Processor-defined:

%define T_DIVIDE         0      // divide error
%define T_DEBUG          1      // debug exception
%define T_NMI            2      // non-maskable interrupt
%define T_BRKPT          3      // breakpoint
%define T_OFLOW          4      // overflow
%define T_BOUND          5      // bounds check
%define T_ILLOP          6      // illegal opcode
%define T_DEVICE         7      // device not available
%define T_DBLFLT         8      // double fault
;;; %define T_COPROC      9	// reserved (not used since 486)
%define T_TSS           10      // invalid task switch segment
%define T_SEGNP         11      // segment not present
%define T_STACK         12      // stack exception
%define T_GPFLT         13      // general protection fault
%define T_PGFLT         14      // page fault
;;; %define T_RES        15      // reserved
%define T_FPERR         16      // floating point error
%define T_ALIGN         17      // aligment check
%define T_MCHK          18      // machine check
%define T_SIMDERR       19      // SIMD floating point error
	
%define T_IRQ0          32      // IRQ 0 corresponds to int T_IRQ

%define IRQ_TIMER        0
%define IRQ_KBD          1
%define IRQ_COM1         4
%define IRQ_IDE         14
%define IRQ_ERROR       19
%define IRQ_SPURIOUS    31
