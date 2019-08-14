
	default rel
	section .data

	extern _0x00_unreachable
	extern _0x01_nop
	extern _0x02_block
	extern _0x03_loop
	extern _0x04_if
	extern _0x05_else
	extern _0x0b_end
	extern _0x0c_br
	extern _0x0d_br_if
	extern _0x0e_br_table
	extern _0x0f_return
	extern _0x10_call
	extern _0x11_indirect
	extern _0x1a_drop
	extern _0x1b_select
	extern _0x20_get_local
	extern _0x21_set_local
	extern _0x22_tee_local
	extern _0x23_get_global
	extern _0x24_set_global

	extern _0x28_i32_load
	extern _0x29_i64_load
	extern _0x2a_f32_load
	extern _0x2b_f64_load
	extern _0x2c_i32_load8_s
	extern _0x2d_i32_load8_u
	extern _0x2e_i32_load16_s
	extern _0x2f_i32_load16_u
	extern _0x30_i64_load8_s
	extern _0x31_i64_load8_u
	extern _0x32_i64_load16_s
	extern _0x33_i64_load16_u
	extern _0x34_i64_load32_s
	extern _0x35_i64_load32_u
	extern _0x36_i32_store
	extern _0x37_i64_store
	extern _0x38_f32_store
	extern _0x39_f64_store
	extern _0x3a_i32_store8
	extern _0x3b_i32_store16
	extern _0x3c_i64_store8
	extern _0x3d_i64_store16
	extern _0x3e_i64_store32
	extern _0x3f_current_memory
	extern _0x40_grow_memory
	;; Constant
	extern _0x41_i32_const
	extern _0x42_i64_const
	extern _0x43_f32_const
	extern _0x44_f64_const

	extern _0x45
	extern _0x46
	extern _0x47
	extern _0x48
	extern _0x49
	extern _0x4a
	extern _0x4b
	extern _0x4c
	extern _0x4d
	extern _0x4e
	extern _0x4f
	extern _0x50
	extern _0x51
	extern _0x52
	extern _0x53
	extern _0x54
	extern _0x55
	extern _0x56
	extern _0x57
	extern _0x58
	extern _0x59
	extern _0x5a
	extern _0x5b
	extern _0x5c
	extern _0x5d
	extern _0x5e
	extern _0x5f
	extern _0x60
	extern _0x61
	extern _0x62
	extern _0x63
	extern _0x64
	extern _0x65
	extern _0x66
	
	extern _0x67_clz
	extern _0x68_ctz
	extern _0x69_popcnt
	extern _0x6a_add
	extern _0x6b_sub	

	extern _0x00

	extern __0x00_none
	extern __0x01_type
	extern __0x02_import
	extern __0x03_function
	extern __0x04_table
	extern __0x05_memory
	extern __0x06_global
	extern __0x07_export
	extern __0x08_start
	extern __0x09_element
	extern __0x0a_code
	extern __0x0b_data
	
	global _inp
	global _outp
	global _symtabp
	global _codep
	global _section_table
	global _opcode_table
	global _stack_depth
	global _localv_base_reg
	
;;; 
_inp:
	dq 0
_outp:
	dq 0
_symtabp:
	dq 0
;;; this contains pointer to memory that you are going to write the translatin result.
_codep:
	dq 0
;;; (current) stack depth is used for indicating which register is used for next operation.
;;; For instance, if the current depth is 0, get_local will set the variable on %rax on x86-64.
_stack_depth:
	db 0
;;; stack depth limit is the maximum depth of stack which generaly means
;;; numebr of registers that an architecture has.
_stack_depth_limit:
	db 0

;;; indirect-memory refrerence from a register is often used to access memory on local variable,
;;; For instacne, this is %rbx on x86-64.
_localv_base_reg:
	db 0
	
;; _x86_op_semantics:
;; 	dq _0x00
;; 	dq _0x01
;; 	dq _0x02

_section_table:
	dq __0x00_none
	dq __0x01_type
	dq __0x02_import
	dq __0x03_function
	dq __0x04_table
	dq __0x05_memory
	dq __0x06_global
	dq __0x07_export
	dq __0x08_start
	dq __0x09_element
	dq __0x0a_code
	dq __0x0b_data

_opcode_table:
	;; Conditional flow operator
	dq _0x00
	;; dq _0x00_unreachable
	dq _0x01_nop
	dq _0x02_block
	dq _0x03_loop
	dq _0x04_if
	dq _0x05_else
	dq _0x00
	dq _0x00
	dq _0x00
	dq _0x00
	dq _0x00
	dq _0x0b_end
	dq _0x0c_br
	dq _0x0d_br_if
	dq _0x0e_br_table
	dq _0x0f_return
	;; Call operator
	dq _0x10_call
	dq _0x11_indirect
	;;
	dq _0x00
	dq _0x00
	dq _0x00
	dq _0x00
	dq _0x00
	dq _0x00
	dq _0x00
	dq _0x00
	dq _0x1a_drop
	dq _0x1b_select
	dq _0x00
	dq _0x00
	dq _0x00	
	dq _0x00
	;;  Variable Access
	dq _0x20_get_local
	dq _0x21_set_local
	dq _0x22_tee_local
	dq _0x23_get_global
	dq _0x24_set_global
	dq _0x00
	dq _0x00
	dq _0x00
	;; Memory related
	dq _0x28_i32_load
	dq _0x29_i64_load
	dq _0x2a_f32_load
	dq _0x2b_f64_load
	dq _0x2c_i32_load8_s
	dq _0x2d_i32_load8_u
	dq _0x2e_i32_load16_s
	dq _0x2f_i32_load16_u
	dq _0x30_i64_load8_s
	dq _0x31_i64_load8_u
	dq _0x32_i64_load16_s
	dq _0x33_i64_load16_u
	dq _0x34_i64_load32_s
	dq _0x35_i64_load32_u
	dq _0x36_i32_store
	dq _0x37_i64_store
	dq _0x38_f32_store
	dq _0x39_f64_store
	dq _0x3a_i32_store8
	dq _0x3b_i32_store16
	dq _0x3c_i64_store8
	dq _0x3d_i64_store16
	dq _0x3e_i64_store32
	dq _0x3f_current_memory
	dq _0x40_grow_memory
	;; Constant
	dq _0x41_i32_const
	dq _0x42_i64_const
	dq _0x43_f32_const
	dq _0x44_f64_const
	;; Comparison
	dq _0x45
	dq _0x46
	dq _0x47
	dq _0x48
	dq _0x49
	dq _0x4a
	dq _0x4b
	dq _0x4c
	dq _0x4d
	dq _0x4e
	dq _0x4f
	dq _0x50
	dq _0x51
	dq _0x52
	dq _0x53
	dq _0x54
	dq _0x55
	dq _0x56
	dq _0x57
	dq _0x58
	dq _0x59
	dq _0x5a
	dq _0x5b
	dq _0x5c
	dq _0x5d
	dq _0x5e
	dq _0x5f
	dq _0x60
	dq _0x61
	dq _0x62
	dq _0x63
	dq _0x64
	dq _0x65
	dq _0x66
	;; Numeric Operator
	dq _0x67_clz
	dq _0x68_ctz
	dq _0x69_popcnt
	dq _0x6a_add
	dq _0x6b_sub
	
	
