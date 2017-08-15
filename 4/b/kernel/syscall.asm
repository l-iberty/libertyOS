;===========================================
; syscall.asm 系统调用相关
;===========================================

_NR_get_ticks		equ	0
INT_VECTOR_SYSCALL	equ	0x80

global	get_ticks

[SECTION .text]
[BITS 32]

get_ticks:
	mov	eax, _NR_get_ticks
	int	INT_VECTOR_SYSCALL
	ret
