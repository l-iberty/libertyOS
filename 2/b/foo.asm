extern choose		; int choose(int a, int b)

[section .data]

num1		dd	3
num2		dd	4

[section .text]

global _start		; 导出 _start　让链接器识别
global myprint		; 导出函数

_start:
	push	dword [num2]
	push	dword [num1]
	call	choose
	add	esp, 8

	mov	ebx, 0
	mov	eax, 1	; sys_exit
	int	0x80

; void myprint(char* msg, int len)
myprint:
	mov	edx, [esp + 8]	; len
	mov	ecx, [esp + 4]	; msg
	mov	ebx, 1
	mov	eax, 4		; sys_write
	int	0x80
	ret
