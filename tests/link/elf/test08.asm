
	section .text
	global f1
	global f2
	extern f3
f2:
	
	ret

f1:
	call f3
	ret
	
	