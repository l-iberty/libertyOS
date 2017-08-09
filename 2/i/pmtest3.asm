; ==========================================
; pmtest3.asm
; 编译方法：nasm pmtest3.asm -o pmtest3.com
; ==========================================

%include	"pm.inc"	; 常量, 宏, 以及一些说明
		org 0100h
		jmp LABEL_START

PageDirBase	equ	200000h		; 页目录起始物理地址 2M
PageTblBase	equ	201000h		; 页表起始物理地址 2M + 4K

LinearAddrDemo	equ	700000h
ProcFoo		equ	700000h
ProcBar		equ	701000h
ProcPagingDemo	equ	702000h

; //////////////////////////////////////////////////////////////////////////////////////////////

; GDT
[SECTION .gdt]
;					Base		Limit		Attr
LABEL_GDT:		Descriptor	0,		0,		0		; 空描述符, 处理器的要求
LABEL_DESC_NORMAL:	Descriptor	0,		0FFFFh,		DA_D32		; Normal 描述符
LABEL_DESC_CODE32:	Descriptor	0,		SegCode32Len-1,	DA_C32|DA_D32	; 32-bit 代码段描述符
LABEL_DESC_CODE16:	Descriptor	0,		0FFFFh,		DA_C16		; 16-bit 位代码段描述符
LABEL_DESC_VIDEO:	Descriptor	0B8000h,	0FFFFh,		DA_D32+DPL_3	; 32-bit 位视频段描述符
LABEL_DESC_STACK:	Descriptor	0,		0,		DA_S32_L	; 32-bit 位堆栈段描述符
LABEL_DESC_DATA:	Descriptor	0,		DataLen-1,	DA_D32		; 32-bit 位数据段描述符
LABEL_DESC_LDT:		Descriptor	0,		LdtLen-1,	DA_LDT		; LDT 描述符 (保护模式下需要使用 GDT 对其寻址 )
LABEL_DESC_CGATE_DEST:	Descriptor	0,		CGateCodeLen-1, DA_C32		; 调用门目标代码段描述符
LABEL_DESC_STACK3:	Descriptor	0,		0,		DA_S32_L+DPL_3	; 32-bit ring3 堆栈段描述符
LABEL_DESC_CODE3:	Descriptor	0,		SegCode3Len-1,	DA_C32+DPL_3	; 32-bit ring3 代码段描述符
LABEL_DESC_TSS:		Descriptor	0,		TSSLen-1,	DA_TSS		; TSS 描述符
LABEL_DESC_PAGE_DIR:	Descriptor	PageDirBase,	4095,		DA_D32		; 页目录描述符
LABEL_DESC_PAGE_TBL:	Descriptor	PageTblBase,	1023,		DA_D32|DA_G_4K	; 页表描述符 (4K 粒度)
LABEL_DESC_FLAT_C:	Descriptor	0,		1FFFh,		DA_C32|DA_G_4K
LABEL_DESC_FLAT_RW:	Descriptor	0,		1FFFh,		DA_D32|DA_G_4K
; 门
;				Selector		Offset	Argc	Attr
LABEL_DESC_CGATE:	Gate	Selector_CGate_Dest,	0,	4,	DA_CGATE+DPL_3	; 调用门

GdtLen	equ	$ - LABEL_GDT	; GDT　长度
GdtPtr:	dw	GdtLen - 1	; GDT　界限
	dd	0		; GDT线性基地址

; GDT　选择子, 16 bits
Selector_Normal		equ	LABEL_DESC_NORMAL	- LABEL_GDT
Selector_Code32		equ	LABEL_DESC_CODE32	- LABEL_GDT
Selector_Code16		equ	LABEL_DESC_CODE16	- LABEL_GDT
Selector_Video		equ	LABEL_DESC_VIDEO	- LABEL_GDT
Selector_Stack		equ	LABEL_DESC_STACK	- LABEL_GDT
Selector_Data		equ	LABEL_DESC_DATA		- LABEL_GDT
Selector_Ldt		equ	LABEL_DESC_LDT		- LABEL_GDT		; 通过该选择子寻址 LDT
Selector_CGate_Dest	equ	LABEL_DESC_CGATE_DEST 	- LABEL_GDT		; 调用门目标代码段选择子
Selector_CGate		equ	LABEL_DESC_CGATE	- LABEL_GDT+SA_RPL_3	; 调用门选择子
Selector_Stack3		equ	LABEL_DESC_STACK3	- LABEL_GDT+SA_RPL_3
Selector_Code3		equ	LABEL_DESC_CODE3	- LABEL_GDT+SA_RPL_3
Selector_TSS		equ	LABEL_DESC_TSS		- LABEL_GDT
Selector_PageDir	equ	LABEL_DESC_PAGE_DIR	- LABEL_GDT
Selector_PageTbl	equ	LABEL_DESC_PAGE_TBL	- LABEL_GDT
Selector_FlatC		equ	LABEL_DESC_FLAT_C	- LABEL_GDT
Selector_FlatRW		equ	LABEL_DESC_FLAT_RW	- LABEL_GDT

; ///////////////////////////////// End of [SECTION .gdt] /////////////////////////////////


; LDT
[SECTION .ldt]
ALIGN 32	; 使用 LDT 时已处于 32-bit 保护模式下
LABEL_LDT:
;					Base	Limit		Attr
LABEL_LDT_DESC_CODE32:	Descriptor	0,	CodeLLen-1,	DA_C32
LABEL_LDT_DESC_DATA:	Descriptor	0,	DataLLen-1,	DA_D32

LdtLen	equ	$ - LABEL_LDT

; LDT 选择子, 16 bits
Selector_Code32L	equ	LABEL_LDT_DESC_CODE32	- LABEL_LDT + SA_TI_LDT
Selector_DataL		equ	LABEL_LDT_DESC_DATA	- LABEL_LDT + SA_TI_LDT

; ///////////////////////////////// End of [SECTION .ldt] /////////////////////////////////


; IDT
[SECTION .idt]
ALIGN 32
LABEL_IDT:
%rep 13
;			Selector		Offset		Argc	Attr
		Gate	Selector_Code32,	IntHandler,	0,	DA_IGATE
%endrep

.#GP:		Gate	Selector_Code32,	GPHandler,	0,	DA_IGATE

%rep (32-14)
		Gate	Selector_Code32,	IntHandler,	0,	DA_IGATE
%endrep

.CLOCK:		Gate	Selector_Code32,	ClockHandler,	0,	DA_IGATE

%rep (256-33)
		Gate	Selector_Code32,	IntHandler,	0,	DA_IGATE+DPL_3
%endrep

IdtLen	equ	$ - LABEL_IDT	; IDT 长度
IdtPtr:	dw	IdtLen - 1	; IDT 界限
	dd	0		; IDT 线性基地址

; ///////////////////////////////// End of [SECTION .idt] /////////////////////////////////


; 堆栈段
[SECTION .stack]
ALIGN 32
[BITS 32]
LABEL_SEG_STACK:
	times 512 db 0
ButtomOfStack	equ $ - LABEL_SEG_STACK

; ///////////////////////////////// End of [SECTION .stack] /////////////////////////////////


; ring3 堆栈段
[SECTION .stack3]
ALIGN 32
[BITS 32]
LABEL_SEG_STACK3:
	times 512 db 0
ButtomOfStack3	equ $ - LABEL_SEG_STACK3

; ///////////////////////////////// End of [SECTION .stack3] /////////////////////////////////


; TSS 段
[SECTION .tss]
ALIGN 32
[BITS 32]
LABEL_SEG_TSS:
	dd	0				; PREV_TSS
	dd	ButtomOfStack			; ESP0
	dd	Selector_Stack			; SS0
	dd	0				; ESP1
	dd	0				; SS1
	dd	0				; ESP2
	dd	0				; SS2
	dd	0				; CR3
	dd	0				; EIP
	dd	0				; EFLAGS
	dd	0				; EAX
	dd	0				; ECX
	dd	0				; EDX
	dd	0				; EBX
	dd	0				; ESP
	dd	0				; EBP
	dd	0				; ESI
	dd	0				; EDI
	dd	0				; ES
	dd	0				; CS
	dd	0				; SS
	dd	0				; DS
	dd	0				; FS
	dd	0				; GS
	dd	0				; LDT 选择子
	dw	0				; 保留
	dw	$ - LABEL_SEG_TSS + 2		; I/O　许可映射区基地址
	db	0FFh				; I/O 许可映射区结束标志

TSSLen	equ	$ - LABEL_SEG_TSS

; ///////////////////////////////// End of [SECTION .tss] /////////////////////////////////

; 数据段
[SECTION .data]
ALIGN 32
[BITS 32]
LABEL_SEG_DATA:
szTest:			db 'In Protection Mode', 0
Offset_szTest		equ szTest - LABEL_SEG_DATA

szGPError:		db '#GP', 0
Offset_szGPError	equ szGPError - LABEL_SEG_DATA

SPValueInRealMode	dw 0

DataLen	equ	$ - LABEL_SEG_DATA

; ///////////////////////////////// End of [SECTION .data] /////////////////////////////////


[SECTION .c16_r] ; 实模式下的 16-bit 代码段
[BITS 16]
LABEL_START:
	mov	ax, cs
	mov	ds, ax
	mov	es, ax
	mov	ss, ax
	mov	sp, 0100h

	mov	[SPValueInRealMode], sp			; 保存实模式下的sp
	mov	[LABEL_BACK_TO_REAL + 3], ax

	; 初始化 32-bit 代码段描述符中的"段基址", 参照描述符格式
	xor	eax, eax
	mov	ax, cs
	shl	eax, 4
	add	eax, LABEL_SEG_CODE32			; 段地址 << 4 + 偏移地址 = 线性地址
	mov	word [LABEL_DESC_CODE32 + 2], ax	; ax = 线性基地址的低16 bits, 填入描述符的"段基址1"
	shr	eax, 16					; ax = 线性基地址的高16 bits
	mov	byte [LABEL_DESC_CODE32 + 4], al	; 段基址2
	mov	byte [LABEL_DESC_CODE32 + 7], ah	; 段基址3

	; 初始化 16-bit 代码段描述符
	xor	eax, eax
	mov	ax, cs
	shl	eax, 4
	add	eax, LABEL_SEG_CODE16
	mov	word [LABEL_DESC_CODE16 + 2], ax
	shr	eax, 16
	mov	byte [LABEL_DESC_CODE16 + 4], al
	mov	byte [LABEL_DESC_CODE16 + 7], ah

	; 初始化堆栈段描述符
	xor	eax, eax
	mov	ax, ds
	shl	eax, 4
	add	eax, LABEL_SEG_STACK
	mov	word [LABEL_DESC_STACK + 2], ax
	shr	eax, 16
	mov	byte [LABEL_DESC_STACK + 4], al
	mov	byte [LABEL_DESC_STACK + 7], ah

	; 初始化数据段描述符
	xor	eax, eax
	mov	ax, ds
	shl	eax, 4
	add	eax, LABEL_SEG_DATA
	mov	word [LABEL_DESC_DATA + 2], ax
	shr	eax, 16
	mov	byte [LABEL_DESC_DATA + 4], al
	mov	byte [LABEL_DESC_DATA + 7], ah

	; 初始化 GDT 中的 LDT 描述符
	xor	eax, eax
	mov	ax, ds
	shl	eax, 4
	add	eax, LABEL_LDT
	mov	word [LABEL_DESC_LDT + 2], ax
	shr	eax, 16
	mov	byte [LABEL_DESC_LDT + 4], al
	mov	byte [LABEL_DESC_LDT + 7], ah

	; 初始化 LDT 中的 32-bit 代码段描述符
	xor	eax, eax
	mov	ax, cs
	shl	eax, 4
	add	eax, LABEL_LDT_SEG_CODE32
	mov	word [LABEL_LDT_DESC_CODE32 + 2], ax
	shr	eax, 16
	mov	byte [LABEL_LDT_DESC_CODE32 + 4], al
	mov	byte [LABEL_LDT_DESC_CODE32 + 7], ah

	; 初始化 LDT 中的数据段描述符
	xor	eax, eax
	mov	ax, ds
	shl	eax, 4
	add	eax, LABEL_LDT_SEG_DATA
	mov	word [LABEL_LDT_DESC_DATA + 2], ax
	shr	eax, 16
	mov	byte [LABEL_LDT_DESC_DATA + 4], al
	mov	byte [LABEL_LDT_DESC_DATA + 7], ah

	; 初始化调用门的目标代码段描述符
	xor	eax, eax
	mov	ax, cs
	shl	eax, 4
	add	eax, LABEL_SEG_CGATE_DEST
	mov	word [LABEL_DESC_CGATE_DEST + 2], ax
	shr	eax, 16
	mov	byte [LABEL_DESC_CGATE_DEST + 4], al
	mov	byte [LABEL_DESC_CGATE_DEST + 7], ah

	; 初始化 ring3 堆栈段描述符
	xor	eax, eax
	mov	ax, ds
	shl	eax, 4
	add	eax, LABEL_SEG_STACK3
	mov	word [LABEL_DESC_STACK3 + 2], ax
	shr	eax, 16
	mov	byte [LABEL_DESC_STACK3 + 4], al
	mov	byte [LABEL_DESC_STACK3 + 7], ah

	; 初始化 ring3 代码段描述符
	xor	eax, eax
	mov	ax, cs
	shl	eax, 4
	add	eax, LABEL_SEG_CODE3
	mov	word [LABEL_DESC_CODE3 + 2], ax
	shr	eax, 16
	mov	byte [LABEL_DESC_CODE3 + 4], al
	mov	byte [LABEL_DESC_CODE3 + 7], ah

	; 初始化 TSS 描述符
	xor	eax, eax
	mov	ax, ds
	shl	eax, 4
	add	eax, LABEL_SEG_TSS
	mov	word [LABEL_DESC_TSS + 2], ax
	shr	eax, 16
	mov	byte [LABEL_DESC_TSS + 4], al
	mov	byte [LABEL_DESC_TSS + 7], ah
	
	; 为加载 GDTR 做准备
	xor	eax, eax
	mov	ax, ds
	shl	eax, 4
	add	eax, LABEL_GDT			; eax <- GDT线性基地址
	mov	dword [GdtPtr + 2], eax		; [GdtPtr + 2] <- GDT线性基地址

	; 为加载 IDTR 做准备
	xor	eax, eax
	mov	ax, ds
	shl	eax, 4
	add	eax, LABEL_IDT			; eax <- IDT线性基地址
	mov	dword [IdtPtr + 2], eax		; [GdtPtr + 2] <- IDT线性基地址

	; 加载 GDTR
	lgdt	[GdtPtr]

	; 打开　A20 地址线
	in	al, 92h
	or	al, 0000_0010b
	out	92h, al

	; 关中断
	cli

	; 加载 IDTR
	lidt	[IdtPtr]

	; 控制寄存器 cr0 的PE位置1
	mov	eax, cr0
	or	eax, 1
	mov	cr0, eax

	; 进入保护模式
	jmp	dword Selector_Code32:0
	; dword 关键字强制处理器将'0'解释为32位偏移量; 否则, 如果偏移量是一个超过16位的数, 高16位会丢失.
	; jmp 指令将 Selector_Code32 加载到代码段选择器 cs, 并从 GDT 中取出对应的描述符, 加载到cs描述符
	; 高速缓存器; 同时, 把指令中给出的32位偏移量传送到 eip, 处理器便从新的地方取得指令并执行.

;++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
LABEL_REAL_ENTRY:		; 从保护模式跳回实模式后就跳到了这里
	mov	ax, cs		; cs 已被恢复为实模式下的原值
	mov	ds, ax
	mov	es, ax
	mov	ss, ax
	mov	sp, [SPValueInRealMode]

	; 关闭　A20 地址线
	in	al, 92h
	and	al, 1111_1101b
	out	92h, al

	; 开中断
	sti

	mov	ax, 0B800h
	mov	gs, ax
	mov	di, (80 * 13 + 0) * 2	; 屏幕第 13 行, 第 0 列
	mov	si, _szMsg
	call	DispMsg
	
	mov	ax, 4c00h
	int	21h

_szMsg:	db 'In Real Mode', 0

;---------------------------------------------------------------
; DispMsg: 实模式下在 gs:di 处显示字符串
; ds:si 指向字符串第一字节
;---------------------------------------------------------------
DispMsg:
	push	bx
	mov	ah, 0Ch			; 0000: 黑底    1100: 红字
	xor	bx, bx
	mov	si, _szMsg
.a:
	mov	al, [si+bx]
	cmp	al, 0
	jz	.b
	mov	[gs:edi], ax
	add	edi, 2
	inc	si
	jmp	.a
.b:
	pop	bx
	ret
;----------------------------------------------------------------


; ///////////////////////////////// End of [SECTION .c16_r] /////////////////////////////////

[SECTION .c32] ; 32-bit 代码段, 由实模式跳入
[BITS 32]
LABEL_SEG_CODE32:
	mov	ax, Selector_Video	; 选择子为 16 bits, 无需使用 eax
	mov	gs, ax

	mov	ax, Selector_Stack	; 堆栈段选择子
	mov	ss, ax
	mov	esp, ButtomOfStack

	mov	ax, Selector_Data	; 数据段选择子
	mov	ds, ax

	; 显示一个字符串
	mov	edi, (80 * 8 + 0) * 2	; 屏幕第 8 行, 第 0 列
	mov	esi, Offset_szTest
	call	DispStr

;-----------测试分页机制-----------
	call	PagingDemo
;---------------------------------

;-----------测试中断--------------
	call	Init8259A
;---------------------------------

	; 加载 LDT
	;mov	ax, Selector_Ldt
	;lldt	ax

	; 跳入局部任务
	;jmp	Selector_Code32L:0

	;jmp	Selector_Code16:0

	mov	ax, Selector_TSS
	ltr	ax

	; 跳入 ring3
	; 假想有个调用者，他通过调用门从 ring3 转移到这里(ring0),　现在调用门要返回，
	; 而"返回"的目标代码段即"调用者"的 ring0 代码段. 以下 push 操作的栈就是 "调用者"
	; 使用"调用门"时切换到的新栈(ring0).
	push	Selector_Stack3		; "调用者"的 ss
	push	ButtomOfStack3		; "调用者"的 esp
;	| 参数1	|
;	| 参数2	|
;	   ...				; "调用门"的参数, 本例 argc=0
;	| 参数n	|
	push	Selector_Code3		; "调用者"的 cs
	push	0			; "调用者"的 eip
	retf

;--------------------------------------------------------------------------
; SetupPaging	启动分页机制
;--------------------------------------------------------------------------
SetupPaging:
	; 初始化页目录, 填写 1024 个页表的物理基地址 ( 页目录大小为 4K )
	mov	ax, Selector_PageDir
	mov	es, ax
	xor	edi, edi		; es:edi -> 页目录的物理基地址
	xor	eax, eax
	mov	eax, PageTblBase | PG_P | PG_RWW | PG_US1
	mov	ecx, 1024		; 页目录存放 1024 个页表的物理基地址
	cld
.init_pagedir:
	stosd				; STOSD: Store EAX at address ES:EDI
	add	eax, 4096		; 所有页表在内存中连续存放
	loop	.init_pagedir

	; 初始化页表, 共 1024 个页表, 每个页表存放 1024 个页的物理基地址 ( 页表总大小为 4M )
	mov	ax, Selector_PageTbl
	mov	es, ax
	xor	edi, edi		; es:edi -> 第一个页表的物理基地址
	xor	eax, eax
	mov	eax, PG_P | PG_RWW | PG_US1
	mov	ecx, 1024 * 1024	; 1024 * 1024 个页表项
	cld
.init_pagetbl:
	stosd
	add	eax, 4096		; 每个物理页 4K
	loop	.init_pagetbl

	; 共 1024 * 1024 个页表项, 每个页表项指向一块 4K 大小的物理内存，
	; 因此页表映射了 4G 内存空间.

	; 在 CR3 (Page Directory Base Register) 登记页目录物理基地址
	mov	eax, PageDirBase
	mov	cr3, eax

	; 将 CR0 最高位(PG)置1, 启动分页机制
	mov	eax, cr0
	or	eax, 80000000h
	mov	cr0, eax

	ret
;--------------------------------------------------------------------------


;--------------------------------------------------------------------------
; PagingDemo
;--------------------------------------------------------------------------
PagingDemo:
	mov	ax, Selector_FlatRW
	mov	es, ax
	mov	ax, Selector_Code32
	mov	ds, ax

	push	LenPagingDemo
	push	OffsetPagingDemoProc
	push	ProcPagingDemo
	call	CopyMemory
	add	esp, 12

	push	LenFoo
	push	OffsetFoo
	push	ProcFoo
	call	CopyMemory
	add	esp, 12

	push	LenBar
	push	OffsetBar
	push	ProcBar
	call	CopyMemory
	add	esp, 12

	call	SetupPaging

	call	Selector_FlatC:ProcPagingDemo
	call	PageSwitch
	call	Selector_FlatC:ProcPagingDemo
	
	ret
;--------------------------------------------------------------------------


;--------------------------------------------------------------------------
; PageSwitch
;--------------------------------------------------------------------------
PageSwitch:
	mov	ax, Selector_FlatRW
	mov	ds, ax

	; 改变 LinearAddrDemo 对应的物理地址
	mov	eax, LinearAddrDemo
	shr	eax, 22			; 取线性地址高 10 位 -> 页目录项的索引
	mov	ebx, 4096		; 每个页表大小为 4K
	mul	ebx			; (EDX,EAX) <- (EAX)*(SRC)
	mov	ecx, eax
	mov	eax, LinearAddrDemo
	shr	eax, 12
	and	eax, 3FFh		; 取线性地址中间 10 位 -> 页表项的索引
	mov	ebx, 4			; 每个页表项大小为 4B
	mul	ebx
	add	eax, ecx		; (EAX)=页表项相对 PageTblBase 的偏移
	add	eax, PageTblBase

	; 显示原页表项
	push	eax
	mov	eax, [eax]
	mov	edi, (80 * 1 + 0) * 2	; 屏幕第 1 行, 第 0 列
	call	Disp_DWORD
	pop	eax

	mov	dword [eax], ProcBar | PG_P | PG_RWW | PG_US1

	; 显示新页表项
	push	eax
	mov	eax, [eax]
	mov	edi, (80 * 1 + 15) * 2	; 屏幕第 1 行, 第 15 列
	call	Disp_DWORD
	pop	eax

	; 重写 cr3, 刷新 TLB (Translation Lookaside Buffer)
	mov	eax, PageDirBase
	mov	cr3, eax

	ret
;--------------------------------------------------------------------------


;--------------------------------------------------------------------------
PagingDemoProc:
OffsetPagingDemoProc	equ	PagingDemoProc - $$
	mov	eax, LinearAddrDemo
	call	eax
	retf
LenPagingDemo		equ	$ - PagingDemoProc

Foo:
OffsetFoo		equ	Foo - $$
	mov	ah, 0Ch				; 0000: 黑底    1100: 红字
	mov	al, 'F'
	mov	[gs:((80 * 17 + 0) * 2)], ax	; 屏幕第 17 行, 第 0 列。
	mov	al, 'o'
	mov	[gs:((80 * 17 + 1) * 2)], ax	; 屏幕第 17 行, 第 1 列。
	mov	[gs:((80 * 17 + 2) * 2)], ax	; 屏幕第 17 行, 第 2 列。
	ret
LenFoo			equ	$ - Foo

Bar:
OffsetBar		equ	Bar - $$
	mov	ah, 0Ch				; 0000: 黑底    1100: 红字
	mov	al, 'B'
	mov	[gs:((80 * 18 + 0) * 2)], ax	; 屏幕第 18 行, 第 0 列。
	mov	al, 'a'
	mov	[gs:((80 * 18 + 1) * 2)], ax	; 屏幕第 18 行, 第 1 列。
	mov	al, 'r'
	mov	[gs:((80 * 18 + 2) * 2)], ax	; 屏幕第 18 行, 第 2 列。
	ret
LenBar			equ	$ - Bar
;--------------------------------------------------------------------------


;--------------------------------------------------------------------------
; Init8259A
;--------------------------------------------------------------------------
Init8259A:
	mov	al, 00010001b
	out	020h, al	; 主8259A, ICW1.
	call	io_delay

	out	0A0h, al	; 从8259A, ICW1.
	call	io_delay

	mov	al, 020h	; IRQ0 对应中断向量 0x20
	out	021h, al	; 主8259A, ICW2.
	call	io_delay

	mov	al, 028h	; IRQ8 对应中断向量 0x28
	out	0A1h, al	; 从8259A, ICW2.
	call	io_delay

	mov	al, 00000100b	; 主片的 2 号引脚级联到从片
	out	021h, al	; 主8259A, ICW3.
	call	io_delay

	mov	al, 00000100b	; 从片级联到主片的 2 号引脚
	out	0A1h, al	; 从8259A, ICW3.
	call	io_delay

	mov	al, 00000001b	; AEOI=0 非自动结束方式
	out	021h, al	; 主8259A, ICW4.
	call	io_delay

	out	0A1h, al	; 从8259, ICW4.
	call	io_delay

	mov	al, 11111111b	; 屏蔽主8259A所有中断
	;mov	al, 11111110b	; 仅仅开启定时器中断
	out	021h, al	; 主8259A, OCW1.
	call	io_delay

	mov	al, 11111111b	; 屏蔽从8259A所有中断
	out	0A1h, al	; 从8259A, OCW1.
	call	io_delay

	ret
;--------------------------------------------------------------------------


;--------------------------------------------------------------------------
; io_delay
;--------------------------------------------------------------------------
io_delay:
	nop
	nop
	nop
	nop
	ret
;--------------------------------------------------------------------------


;--------------------------------------------------------------------------
; _IntHandler 中断处理过程
;--------------------------------------------------------------------------
_IntHandler:
IntHandler	equ	_IntHandler - $$
	mov	edi, (80 * 20 + 0) * 2	; 屏幕第 5 行, 第 0 列
	mov	ah, 0Ch			; 0000 黑底, 1100 红字
	mov	al, '!'
	mov	[gs:edi], ax

	; 打印栈
	mov	edi, (80 * 0 + 30) * 2	; 屏幕第 0 行, 第 30 列
	mov	ecx, 16			; 打印 16 个栈单元
	call	PrintStack

	iretd
;--------------------------------------------------------------------------


;--------------------------------------------------------------------------
; _ClockHandler 时钟中断处理过程
;--------------------------------------------------------------------------
_ClockHandler:
ClockHandler	equ	_ClockHandler - $$
	mov	ah, 0Ch			; 0000 黑底, 1100 红字
	mov	al, '#'
	mov	[gs:edi], ax
	add	edi, 2

	mov	al, 20h
	out	20h, al	; 向主8259A发送 EOI

	iretd
;--------------------------------------------------------------------------


;--------------------------------------------------------------------------
; _GPHandler #GP(常规保护异常)处理过程
;--------------------------------------------------------------------------
_GPHandler:
GPHandler	equ _GPHandler - $$
	mov	ax, Selector_Data
	mov	ds, ax

	mov	esi, Offset_szGPError
	mov	edi, (80 * 0 + 60) * 2	; 屏幕第 0 行, 第 60 列
	call	DispStr

	; 打印栈
	add	edi, 160 - 6		; 回退 3 个字符并换行
	mov	ecx, 16
	call	PrintStack

	jmp	$
;--------------------------------------------------------------------------

%include	"lib32.inc"		; 库函数

SegCode32Len	equ	$ - LABEL_SEG_CODE32

; ///////////////////////////////// End of [SECTION .c32] /////////////////////////////////


[SECTION c16_p] ; 保护模式下的 16-bit 代码段
ALIGN	32
[BITS	16]
LABEL_SEG_CODE16:
	mov	ax, Selector_Normal	; Selector_Normal 指定的段的线性基地址为 0
	mov	ds, ax
	mov	es, ax
	mov	fs, ax
	mov	gs, ax
	mov	ss, ax

	; 清 cr0 PE位
	mov	eax, cr0
	and	al, 11111110b
	mov	cr0, eax

	; 跳回实模式
LABEL_BACK_TO_REAL:
	jmp	0:LABEL_REAL_ENTRY	; 段地址在程序开始处被设置成正确的值, 即跳入保护模式前实模式下的 cs 原值
					; 同时, jmp 指令会修改 cs 为实模式下的原值

SegCode16Len	equ	$ - LABEL_SEG_CODE16
; 不可定义该代码段的界限为 (SegCodeLen - 1), 否则 LABEL_BACK_TO_REAL 处的 jmp 将引发异常. 因为目标代码已经超越了界限.

; ///////////////////////////////// End of [SECTION .c16_p] /////////////////////////////////


[SECTION .lc32] ; 32-bit 局部代码段
ALIGN 32
[BITS 32]
LABEL_LDT_SEG_CODE32:
	mov	ax, Selector_Video
	mov	gs, ax
	mov	edi, (80 * 14 + 10) * 2	; 屏幕第 14 行, 第 10 列

	mov	ax, Selector_DataL
	mov	ds, ax
	mov	esi, Offset_szLDTMessage
	
	mov	ah, 0Ch			; 0000 黑底, 1100 红字
.prints:
	mov	al, [ds:esi]
	cmp	al, 0
	jz	.prints_end
	mov	[gs:edi], ax
	inc	esi
	add	edi, 2
	jmp	.prints
.prints_end:

	; 跳入 16-bit 代码段，最终回到实模式
	jmp	Selector_Code16:0

CodeLLen	equ	$ - LABEL_LDT_SEG_CODE32

; ///////////////////////////////// End of [SECTION .lc32] /////////////////////////////////


[SECTION .ldata] ; 32-bit 局部数据段
ALIGN 32
[BITS 32]
LABEL_LDT_SEG_DATA:
szLDTMessage:		db 'LDT_Message', 0
Offset_szLDTMessage	equ szLDTMessage - LABEL_LDT_SEG_DATA

DataLLen	equ $ - LABEL_LDT_SEG_DATA

; ///////////////////////////////// End of [SECTION .ldata] /////////////////////////////////


[SECTION .cgate] ; 调用门目标代码段
ALIGN 32
[BITS 32]
LABEL_SEG_CGATE_DEST:
	mov	ax, Selector_Video
	mov	gs, ax

	; 打印栈
	mov	edi, (80 * 0 + 40) * 2	; 屏幕第 0 行, 第 40 列
	mov	ecx, 16			; 打印 16 个栈单元
	call	PrintStack_CGate

	push	ebp
	mov	ebp, esp

	; 显示字符串
	mov	edi, (80 * 1 + 2) * 2	; 屏幕第 1 行, 第 2 列
	mov	ebx, 24
	mov	ecx, 4
.getarg:
	mov	eax, [ebp+ebx]
	mov	[gs:edi], ax
	add	edi, 2
	sub	ebx, 4
	loop	.getarg

	pop	ebp

	; 跳入局部任务
	jmp	Selector_Code32L:0
	
	;retf	16

%include	"libcg.inc"		; 库函数

CGateCodeLen	equ $ - LABEL_SEG_CGATE_DEST

; ///////////////////////////////// End of [SECTION .cgate] /////////////////////////////////


[SECTION .c32_r3] ; 32-bit ring3 代码段
ALIGN 32
[BITS 32]
LABEL_SEG_CODE3:
	nop
	nop

	int	80h

	jmp	$

%include	"libr3.inc"		; 库函数

SegCode3Len	equ	$ - LABEL_SEG_CODE3

; ///////////////////////////////// End of [SECTION .c32_r3] /////////////////////////////////


