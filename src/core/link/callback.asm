	
	global callback_arg3
	section .text
	
callback_arg3:
	push rbp
	sub rsp,0x10
	call r8
	add rsp,0x10
	pop rbp
	ret
	