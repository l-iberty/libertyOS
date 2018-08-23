;===================================================
;　kernel.asm
;===================================================

%include "const.inc"

GDT_SIZE		equ	256 * 8		; 256 个描述符, 每个 8 字节
IDT_SIZE		equ	256 * 8		; 256 个描述符, 每个 8 字节
SELECTOR_VIDEO		equ	8
SELECTOR_FLATC		equ	16
SELECTOR_FLATRW		equ	24
SELECTOR_TSS		equ	32

extern	p_current_proc
extern	re_enter
extern	syscall_table
extern	irq_table
extern	exception_table
extern	memcpy
extern	print
extern	cstart
extern	kernel_main

global	_start
global	GDT
global	IDT
global	TSS

global	divide_error
global	debug_exception
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
global	mcu_fault
global	align_check
global	mechine
global	simd_error

global	hwint_0
global	hwint_1
global	hwint_2
global	hwint_3
global	hwint_4
global	hwint_5
global	hwint_6
global	hwint_7
global	hwint_8
global	hwint_9
global	hwint_10
global	hwint_11
global	hwint_12
global	hwint_13
global	hwint_14
global	hwint_15

global	proc_begin

global	sys_call

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

TSS:
		dd	0			; PREV_TSS
TSS_ESP0	dd	0			; ESP0
		dd	SELECTOR_FLATRW		; SS0
		dd	0			; ESP1
		dd	0			; SS1
		dd	0			; ESP2
		dd	0			; SS2
		dd	0			; CR3
		dd	0			; EIP
		dd	0			; EFLAGS
		dd	0			; EAX
		dd	0			; ECX
		dd	0			; EDX
		dd	0			; EBX
		dd	0			; ESP
		dd	0			; EBP
		dd	0			; ESI
		dd	0			; EDI
		dd	0			; ES
		dd	0			; CS
		dd	0			; SS
		dd	0			; DS
		dd	0			; FS
		dd	0			; GS
		dd	0			; LDT 选择子
		dw	0			; 保留
		dw	TSSLen			; I/O　许可位图基地址大于或等于 TSS 界限, 则表示没有 I/O 许可位图

TSSLen	equ	$ - TSS

;-------------------------------------------------------------------------------------

[SECTION .bss]
StackSpace	resb	2 * 1024	; 2K
BottomOfStack:	; 栈底


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
	mov	ax, SELECTOR_FLATRW
	mov	ds, ax
	mov	es, ax

	; 切换栈
	mov	ss, ax
	mov	esp, BottomOfStack
	
	mov	ax, SELECTOR_VIDEO
	mov	gs, ax

	; 刷新 CS 描述符高速缓存器
	jmp	SELECTOR_FLATC:LABEL
LABEL:
	call	cstart
	lidt	[IdtPtr]
	
	mov	ax, SELECTOR_TSS
	ltr	ax
	
	jmp	kernel_main

	hlt

; 0 ~ 19 异常处理例程 -------------------------------------------------------------------
%macro exception 1
	pushad
	push	ds
	push	es
	push	fs
	push	gs
	
	mov	ax, ss
	mov	ds, ax
	mov	es, ax
	mov	fs, ax
	
	lea	eax, [esp + ERR_CODE_OFFSET]
	
	inc	dword [re_enter]
	cmp	dword [re_enter], 0	; re_enter = 0 则没有发生中断重入
	jne	.reenter
	mov	esp, BottomOfStack	; 中断重入未发生, 切换到内核栈
	push	proc_begin
	jmp	.no_reenter
.reenter:				; 中断重入发生时esp处在内核栈, 无需切换
	push	proc_begin_reenter
.no_reenter:
	push	dword [eax + 12]	; eflags
	push	dword [eax + 8]		; cs
	push	dword [eax + 4]		; eip
	push	dword [eax]		; err_code
	push	%1			; vecno
	call	[exception_table + %1 * 4]
	add	esp, 20
	ret
%endmacro

;---------------------------------------------------------------------------

divide_error:
	push		0	; no err_code, the same below
 	exception	0
 	
debug_exception:
	push		0
	exception	1
	
nmi:
	push		0
	exception	2
	
breakpoint:
	push		0
	exception	3
	
overflow:
	push		0
	exception	4
	
out_of_bound:
	push		0
	exception	5
	
invalid_opcode:
	push		0
	exception	6
	
no_math_coprocessor:
	push		0
	exception	7
	
double_fault:
	exception	8
	
coprocessor_error:
	push		0
	exception	9
	
invalid_tss:
	exception	10
	
not_present:
	exception	11
	
stack_error:
	exception	12
	
general_protect:
	exception	13
	
page_fault:
	exception	14

mcu_fault:
	push		0
	exception	16

align_check:
	exception	17

mechine:
	push		0
	exception	18

simd_error:
	push		0
	exception	19


; 8259A 中断处理例程 ----------------------------------------------------

; master -------------------------------------------------------------------
%macro hwint_master 1
	push	0 ; err_code
	pushad
	push	ds
	push	es
	push	fs
	push	gs
	
	mov	ax, ss
	mov	ds, ax
	mov	es, ax
	mov	fs, ax
	
	in	al, INT_M_CTLMASK	; `. 屏蔽当前中断
	or	al, (1 << %1)		;  | out_byte(INT_M_CTLMASK,
	out	INT_M_CTLMASK, al	; /	in_byte(INT_M_CTLMASK) | (1 << %1));
	
	mov	al, EOI			; `.
	out	INT_M_CTL, al		; / 向主8259A发送 EOI
	
	inc	dword [re_enter]
	cmp	dword [re_enter], 0	; re_enter = 0 则没有发生中断重入
	jne	.reenter
	mov	esp, BottomOfStack	; 中断重入未发生, 切换到内核栈
	push	proc_begin
	jmp	.no_reenter
.reenter:
	push	proc_begin_reenter
.no_reenter:
	sti
	push	%1
	call	[irq_table + %1 * 4]
	add	esp, 4
	cli
	in	al, INT_M_CTLMASK	; `. 恢复接收当前中断
	and	al, ~(1 << %1)		;  | out_byte(INT_M_CTLMASK,
	out	INT_M_CTLMASK, al	; /	in_byte(INT_M_CTLMASK) & ~(1 << %1));
	ret
%endmacro

;---------------------------------------------------------------------------

hwint_0:				; 时钟
	hwint_master	0
	
hwint_1:				; 键盘
	hwint_master	1

hwint_2:
	hwint_master	2

hwint_3:
	hwint_master	3

hwint_4:
	hwint_master	4

hwint_5:
	hwint_master	5

hwint_6:
	hwint_master	6

hwint_7:
	hwint_master	7

; slave -------------------------------------------------------------------
%macro hwint_slave 1
	push	0 ; err_code
	pushad
	push	ds
	push	es
	push	fs
	push	gs
	
	mov	ax, ss
	mov	ds, ax
	mov	es, ax
	mov	fs, ax
	
	in	al, INT_S_CTLMASK	; `. 屏蔽当前中断
	or	al, (1 << (%1 - 8))	;  | out_byte(INT_S_CTLMASK,
	out	INT_S_CTLMASK, al	; /	in_byte(INT_S_CTLMASK) | (1 << (%1 - 8)));
	
	mov	al, EOI 	; `.
	out	INT_M_CTL, al	; / 向主8259A发送 EOI
	out	INT_S_CTL, al	; 向从8259A发送 EOI
	
	inc	dword [re_enter]
	cmp	dword [re_enter], 0	; re_enter = 0 则没有发生中断重入
	jne	.reenter
	mov	esp, BottomOfStack	; 中断重入未发生, 切换到内核栈
	push	proc_begin
	jmp	.no_reenter
.reenter:
	push	proc_begin_reenter
.no_reenter:
	sti
	push	%1
	call	[irq_table + %1 * 4]
	add	esp, 4
	cli
	in	al, INT_S_CTLMASK	; `. 恢复接收当前中断
	and	al, ~(1 << (%1 - 8))	;  | out_byte(INT_S_CTLMASK,
	out	INT_S_CTLMASK, al	; /	in_byte(INT_S_CTLMASK) & ~(1 << (%1 - 8)));
	ret
%endmacro

;---------------------------------------------------------------------------

hwint_8:
	hwint_slave	8
	
hwint_9:
	hwint_slave	9

hwint_10:
	hwint_slave	10

hwint_11:
	hwint_slave	11

hwint_12:
	hwint_slave	12

hwint_13:
	hwint_slave	13

hwint_14:				; AT 温盘
	hwint_slave	14

hwint_15:
	hwint_slave	15

;---------------------------------------------------------------------------

sys_call:
	push	0 ; err_code
	pushad
	push	ds
	push	es
	push	fs
	push	gs
	
	mov	esi, eax	; save eax
	
	mov	ax, ss
	mov	ds, ax
	mov	es, ax
	mov	fs, ax
	
	mov	eax, esi	; resume eax
	inc	dword [re_enter]
	cmp	dword [re_enter], 0	; re_enter = 0 则没有发生中断重入
	jne	.reenter
	mov	esp, BottomOfStack	; 中断重入未发生, 切换到内核栈
	push	proc_begin
	push	dword [p_current_proc]
	jmp	.no_reenter
.reenter:				; 中断重入发生时esp处在内核栈, 无需切换
	push	proc_begin_reenter
	push	dword [p_current_proc]
.no_reenter:
	sti
	push	edx
	push	ecx
	push	ebx
	call	[syscall_table + eax * 4]
	add	esp, 12
	cli
	
	pop	esi			; esi <- [p_current_proc]
	mov	[esi + EAX_OFFSET], eax	; return value
	ret
	
;---------------------------------------------------------------------------

proc_begin:
	mov	esp, [p_current_proc]
	lldt	[esp + LDT_SEL_OFFSET]
	mov	eax, [esp + PAGE_DIR_BASE_OFFSET]
	mov	cr3, eax
	lea	eax, [esp + STACK_BUTTOM_OFFSET]
	mov	dword [TSS_ESP0], eax
proc_begin_reenter:
	dec	dword [re_enter]
	pop	gs
	pop	fs
	pop	es
	pop	ds
	popad
	add	esp, 4 ; skip `err_code'
	iretd

; struct stack_frame  结构体内各个字段相对结构体首部的偏移
K_ESP_OFFSET		equ	28
EAX_OFFSET		equ	44
ERR_CODE_OFFSET		equ	48
SS_OFFSET		equ	68
STACK_BUTTOM_OFFSET	equ	72
LDT_SEL_OFFSET		equ	72
PAGE_DIR_BASE_OFFSET	equ	108
