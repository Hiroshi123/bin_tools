
	default rel
	extern ex01
	section .rdata
r1:
	dq 0
	section .data
d1:
	dq 0

	section .text
	global f1
f1:
	lea rax,[ex01]
	mov rax,[d1]
	ret
f2:
	ret
	section .data
d3:
	db 0
d2:
	dq 0
