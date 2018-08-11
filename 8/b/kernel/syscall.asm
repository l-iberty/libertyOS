;===========================================
; syscall.asm 系统调用相关
;===========================================

_NR_get_ticks		equ	0
_NR_sendrecv		equ	1
_NR_getpid		equ	2
_NR_getppid		equ	3
_NR_printk		equ	4
_NR_sem_init		equ	5
_NR_sem_post		equ	6
_NR_sem_wait		equ	7

INT_VECTOR_SYSCALL	equ	0x80

global	get_ticks	; int	get_ticks();
global	sendrecv	; void	sendrecv(int func_type, int pid, MESSAGE* p_msg);
global	getpid		; u32	getpid();
global	getppid		; u32	getppid();
global	printk		; void	printk(const char* sz);
global	sem_init	; int	sem_init(SEMAPHORE* p_sem, int value);
global	sem_post	; int	sem_post(SEMAPHORE* p_sem);
global	sem_wait	; int	sem_wait(SEMAPHORE* p_sem);


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

getpid:
	mov	eax, _NR_getpid
	int	INT_VECTOR_SYSCALL
	ret

getppid:
	mov	eax, _NR_getppid
	int	INT_VECTOR_SYSCALL
	ret

printk:
	mov	ebx, [esp + 4]		; char* sz
	mov	eax, _NR_printk
	int	INT_VECTOR_SYSCALL
	ret

sem_init:
	mov	ebx, [esp + 4]		; p_sem
	mov	ecx, [esp + 8]		; value
	mov	eax, _NR_sem_init
	int	INT_VECTOR_SYSCALL
	ret

sem_post:
	mov	ebx, [esp + 4]		; p_sem
	mov	eax, _NR_sem_post
	int	INT_VECTOR_SYSCALL
	ret

sem_wait:
	mov	ebx, [esp + 4]		; p_sem
	mov	eax, _NR_sem_wait
	int	INT_VECTOR_SYSCALL
	ret


	
