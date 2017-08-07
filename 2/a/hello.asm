[SECTION .data]

strHello	db 'Hello, World', 0Ah
STRLEN		equ	$ - strHello

[SECTION .text]

global	_start	; 导出 _start 这个入口，以便让链接器识别

_start:
	mov	edx, STRLEN
	mov	ecx, strHello
	mov	ebx, 1
	mov	eax, 4
	int	0x80
	
	mov	ebx, 0
	mov	eax, 1
	int	0x80
