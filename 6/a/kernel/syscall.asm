;===========================================
; syscall.asm 系统调用相关
;===========================================

_NR_get_ticks		equ	0
_NR_sendrecv		equ	1
INT_VECTOR_SYSCALL	equ	0x80

global	get_ticks	; int get_ticks();
global	sendrecv	; void sendrecv(int func_type, int pid, MESSAGE* p_msg);

[SECTION .text]
[BITS 32]

get_ticks:
	mov	eax, _NR_get_ticks
	int	INT_VECTOR_SYSCALL
	ret
	
sendrecv:
	mov	ebx, [esp + 4]		; func_type
	mov	ecx, [esp + 8]		; pid
	mov	edx, [esp + 12]		; p_msg
	mov	eax, _NR_sendrecv
	int	INT_VECTOR_SYSCALL
	ret
	
