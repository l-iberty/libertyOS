;===================================================
;　kernel.asm
;===================================================

GDT_SIZE		equ	16 * 8		; 16 个描述符, 每个 8 字节
IDT_SIZE		equ	256 * 8		; 256 个描述符, 每个 8 字节
SELECTOR_VIDEO		equ	8
SELECTOR_FLAT_C		equ	32
SELECTOR_FLAT_RW	equ	40

global	_start
global	IDT

extern	memcpy		; lib/klib.asm
extern	cstart		; kernel/start.c

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

