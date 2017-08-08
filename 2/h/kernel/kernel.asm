;===================================================
;　kernel.asm
;===================================================

GDT_SIZE		equ	16 * 8		; 16 个描述符, 每个 8 字节
IDT_SIZE		equ	256 * 8		; 256 个描述符, 每个 8 字节
SELECTOR_VIDEO		equ	8
SELECTOR_FLAT_C		equ	32
SELECTOR_FLAT_RW	equ	40

extern	memcpy		; lib/klib.asm
extern	cstart		; kernel/start.c
extern	excep_handler	; kernel/start.c

global	_start
global	IDT


global	divide_error
global	reserved
global	nmi
global	breakpoint
global	overflow
global	out_of_bound
global	invalid_opcode
global	no_math_coprocessor
global	double_fault
global	coprocessor_error
global	invalid_tss
global	not_present
global	stack_error
global	general_protect
global	page_fault
global	intel_reserved
global	mcu_fault
global	align_check
global	mechine
global	simd_error


[SECTION .data]
;-------------------------------------------------------------------------------------
GDT:	times	GDT_SIZE db 0

GdtPtr:	dw	0		; GDT 界限
	dd	0		; GDT 线性基地址
;-------------------------------------------------------------------------------------

IDT:	times	IDT_SIZE db 0

IdtPtr:	dw	IDT_SIZE - 1	; IDT 界限
	dd	IDT		; IDT 线性基地址
;-------------------------------------------------------------------------------------

err	times 5 db 0

[SECTION .bss]
StackSpace	resb	2 * 1024	; 2K
ButtomOfStack:	; 栈底


[SECTION .text]
_start:
	; 切换 GDT
	sgdt	[GdtPtr]

	mov	ax, word [GdtPtr]	; ax <- gdt_limit
	inc	ax			; gdt_size
	movzx	eax, ax
	push	eax			; len
	push	dword [GdtPtr + 2]	; pSrc (gdt_base)
	push	dword GDT		; pDst
	call	memcpy
	add	esp, 12

	mov	word [GdtPtr], GDT_SIZE - 1
	mov	dword [GdtPtr + 2], GDT
	lgdt	[GdtPtr]

	; 刷新段选择器的描述符高速缓存器
	mov	ax, SELECTOR_FLAT_RW
	mov	ds, ax
	mov	es, ax

	; 切换栈
	mov	ss, ax
	mov	esp, ButtomOfStack
	
	mov	ax, SELECTOR_VIDEO
	mov	gs, ax

	; 刷新 CS 描述符高速缓存器
	jmp	SELECTOR_FLAT_C:LABEL
LABEL:
	call	cstart
	lidt	[IdtPtr]
	
	ud2

	hlt

; 中断异常处理例程 -------------------------------------------------------------------

divide_error:
	push	0			; "err_code = 0" means that no error code actually.
	push	0			; vecno = 0
	jmp	Call_excep_handler

reserved:
	push	0			; no error code
	push	1			; vecno = 1
	jmp	Call_excep_handler


nmi:
	push	0			; no error code
	push	2			; vecno = 2
	jmp	Call_excep_handler


breakpoint:
	push	0			; no error code
	push	3			; vecno = 3
	jmp	Call_excep_handler


overflow:
	push	0			; no error code
	push	4			; vecno = 4
	jmp	Call_excep_handler


out_of_bound:
	push	0			; no error code
	push	5			; vecno = 5
	jmp	Call_excep_handler


invalid_opcode:
	push	0			; no error code
	push	6			; vecno = 6
	jmp	Call_excep_handler

no_math_coprocessor:
	push	0			; no error code
	push	7			; vecno = 7
	jmp	Call_excep_handler


double_fault:
	push	0			; no error code
	push	8			; vecno = 8
	jmp	Call_excep_handler


coprocessor_error:
	push	0			; no error code
	push	9			; vecno = 9
	jmp	Call_excep_handler


invalid_tss:
	push	0			; no error code
	push	10			; vecno = 10
	jmp	Call_excep_handler


not_present:
	push	0			; no error code
	push	11			; vecno = 11
	jmp	Call_excep_handler


stack_error:
	push	0			; no error code
	push	12			; vecno = 12
	jmp	Call_excep_handler


general_protect:
	push	13			; vecno = 13
	jmp	Call_excep_handler

page_fault:
	push	0			; no error code
	push	14			; vecno = 14
	jmp	Call_excep_handler


intel_reserved:
	push	0			; no error code
	push	15			; vecno = 15
	jmp	Call_excep_handler


mcu_fault:
	push	0			; no error code
	push	16			; vecno = 16
	jmp	Call_excep_handler


align_check:
	push	0			; no error code
	push	17			; vecno = 17
	jmp	Call_excep_handler


mechine:
	push	0			; no error code
	push	18			; vecno = 18
	jmp	Call_excep_handler


simd_error:
	push	0			; no error code
	push	19			; vecno = 19
	jmp	Call_excep_handler


Call_excep_handler:
	; 此时的栈:
;	|  vecno   |	
;	| err_code |
;	|   eip    |
;	|   cs     |
;	|  eflags  |
;
	call	excep_handler
	add	esp, 8			; 此时栈的内容为 eip, cs, eflags, 若要从中断返回, 需要 iretd
	hlt
