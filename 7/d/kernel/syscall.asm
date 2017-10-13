;===========================================
; syscall.asm 系统调用相关
;===========================================

_NR_get_ticks		equ	0
_NR_sendrecv		equ	1
_NR_disp_ldt		equ	2
_NR_getpid		equ	3
_NR_getppid		equ	4
INT_VECTOR_SYSCALL	equ	0x80

global	get_ticks	; int get_ticks();
global	sendrecv	; void sendrecv(int func_type, int pid, MESSAGE* p_msg);
global	disp_ldt	; void disp_ldt();
global	getpid		; u32 getpid();
global	getppid		; u32 getppid();

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

disp_ldt:
	mov	eax, _NR_disp_ldt
	int	INT_VECTOR_SYSCALL
	ret

getpid:
	mov	eax, _NR_getpid
	int	INT_VECTOR_SYSCALL
	ret

getppid:
	mov	eax, _NR_getppid
	int	INT_VECTOR_SYSCALL
	ret

