;===================================================
;　loader.asm
; 编译方法: nasm loader.asm -o loader.bin
;===================================================

	org	0ae00h
	jmp	LABEL_START

; 常量------------------------------------------------------
ButtomOfStack	equ	0ae00h		; 栈底地址
PageDirBase	equ	100000h		; 页目录起始物理地址 1M
PageTblBase	equ	101000h		; 页表起始物理地址 1M + 4K
%include	"load.inc"
%include	"pm.inc"
;-----------------------------------------------------------

; FAT12 磁盘头部信息-----------------------------------------
%include	"fat12hdr.inc"
;-----------------------------------------------------------


; GDT
; [SECTION .gdt] 不应使用 SECTION 定义
;					Base		Limit		Attr
LABEL_GDT:		Descriptor	0,		0,		0		; 空描述符, 处理器的要求
LABEL_DESC_VIDEO:	Descriptor	0B8000h,	0FFFFh,		DA_D32+DPL_3	; 32-bit 位视频段描述符
LABEL_DESC_PAGE_DIR:	Descriptor	PageDirBase,	4095,		DA_D32		; 页目录描述符
LABEL_DESC_PAGE_TBL:	Descriptor	PageTblBase,	1023,		DA_D32|DA_G_4K	; 页表描述符 (4K 粒度)
LABEL_DESC_FLAT_C:	Descriptor	0,		0FFFFFh,	DA_C32|DA_G_4K	; 0 ~ 4G 代码段
LABEL_DESC_FLAT_RW:	Descriptor	0,		0FFFFFh,	DA_D32|DA_G_4K	; 0 ~ 4G 数据段
; 通过上面两个 Flat 段选择子进行寻址时, 偏移地址就是物理地址或线性地址

GdtLen	equ	$ - LABEL_GDT
GdtPtr:	dw	GdtLen - 1			; GDT　界限
	dd	BaseOfLoaderPAddr + LABEL_GDT	; GDT线性基地址

; GDT　选择子, 16 bits
Selector_Video		equ	LABEL_DESC_VIDEO	- LABEL_GDT
Selector_PageDir	equ	LABEL_DESC_PAGE_DIR	- LABEL_GDT
Selector_PageTbl	equ	LABEL_DESC_PAGE_TBL	- LABEL_GDT
Selector_FlatC		equ	LABEL_DESC_FLAT_C	- LABEL_GDT
Selector_FlatRW		equ	LABEL_DESC_FLAT_RW	- LABEL_GDT

; ///////////////////////////////// End of [SECTION .gdt] /////////////////////////////////


LABEL_START:
	mov	ax, cs
	mov	ds, ax
	mov	es, ax
	mov	ss, ax
	mov	sp, ButtomOfStack

	; 得到内存信息
	mov	ebx, 0
	mov	di, _MemChkBuf
.loop:
	mov	eax, 0E820h	; 功能号
	mov	ecx, 20
	mov	edx, 0534D4150h	; 'SMAP'
	int	15h
	jc	.mem_chk_fail	; jump if CF = 1
	add	di, 20
	inc	dword [_dwNumOfARDS]
	cmp	ebx, 0
	jne	.loop
	jmp	.mem_chk_ok
.mem_chk_fail:
	mov	dword [_dwNumOfARDS], 0
.mem_chk_ok:

	; 0 号软驱(A盘)复位
	xor	ah, ah
	xor	dl, dl
	int	13h

	; 读 A 盘的根目录区(共14个扇区)至缓冲区
	mov	ax, BaseOfKernel
	mov	es, ax
	
	push	BaseOfRootBuf
	push	RootDirSecCnt
	push	RootDirSecNo
	call	ReadSector
	add	sp, 6

	; 遍历根目录区的条目, 匹配文件名
	; 外层循环遍历根目录区的条目
	; 内层循环遍历字符串 DIR_Name

	cld
	mov	si, BaseOfRootBuf
.strcmp:
	mov	di, _LoaderFileName
	mov	cx, DirNameLen		; 内层循环次数
.continue:
	lodsb				; AL <- (DS:SI), 并令 SI++
	cmp	al, byte [di]
	jnz	.next	
	inc	di
	loop	.continue
.found:
	sub	si, DirNameLen		; 使 SI 指向当前条目开头
	add	si, OffsetFstClus	; 使 SI 指向 DIR_FstClus
	mov	ax, [si]
	mov	[_LoaderClusNo], ax	; 读取 DIR_FstClus 并保存

	call	LoadKernel		; 加载 KERNEL.BIN
	mov	al, 1			; 'Kernel loaded'
	mov	dh, 2			; 行号 2
	call	DispMsg

	jmp	LABEL_READY_FOR_PM	; 准备进入保护模式
.next:
	dec	byte [_LoopCnt]
	cmp	byte [_LoopCnt], 0
	jz	.notfound
	; 令 SI 指向下一个条目开头
	and	si, 0FFF0h		; 将 SI 低 4 位清零, 使其指向当前条目开头
	add	si, 20h
	jmp	.strcmp
.notfound:
	mov	al, 0			; 'No kernel!'
	mov	dh, 2			; 行号 2
	call	DispMsg
	jmp	$

;-----------------------------------------------------------
; ReadSector(SecNo, SecCnt, pBuf)
; 功能: 读磁盘扇区至缓冲区
; 参数:
; 起始扇区号:	SecNo, 16 bits
; 要读的扇区数:	SecCnt, 16 bits (lower 8 bits avaliable)
; 缓冲区地址:	pBuf, 16 bits (段地址在 es 中)
;-----------------------------------------------------------
ReadSector:
	push	bp
	mov	bp, sp

	mov	ax, [bp+4]		; SecNo
	mov	bl, [BPB_SecPerTrk]	; 每磁道扇区数
	div	bl			; 16 bits 被除数 AX, 8 bits 除数 BL
					; AH = 余数, AL = 商
	mov	ch, al
	shr	ch, 1			; CH = 柱面(磁道)号 = (AL >> 1)
	mov	dh, al
	and	dh, 1			; DH = 磁头号 = (AL & 1)
	mov	cl, ah
	inc	cl			; CL = 起始扇区号 = (AH + 1)
	mov	dl, 0			; DL = 驱动器号 (0号软驱, A盘)
	mov	al, byte [bp+6]		; SecCnt
	mov	bx, [bp+8]		; pBuf
	mov	ah, 02h			; AH = 功能号 (02h, 读磁盘扇区)
	int	13h

	pop	bp
	ret
;-----------------------------------------------------------


;-----------------------------------------------------------
; LoadKernel 加载 KERNEL.BIN
;-----------------------------------------------------------
LoadKernel:
	mov	word [_ppDst], OffsetOfKernel

	; 加载 KERNEL.BIN 的第 1 个簇
	push	word [_ppDst]		; pDst
	push	word [_LoaderClusNo]	; ClusNo
	call	CopyCode
	add	sp, 4
	add	word [_ppDst], 200h	; BPB_BytsPerSec * BPB_SecPerClus = 200h

	; 如果 KERNEL.BIN 跨越多个簇, 需要通过 FAT 表查找之

	; 读取 FAT1 到 BaseOfFatBuffer
	push	BaseOfFatBuf
	push	word [BPB_FATSz16]
	push	Fat1SecNo
	call	ReadSector
	add	sp, 6

.begin:
	mov	ah, 0Eh			;　每读一个簇(这里, 1 个簇包含 1 个扇区)就打印一个 '.',
	mov	al, '.'			; 形成如下效果:
	mov	bx, 0007h		; 	Ready    .....
	int	10h			; 'Ready    ' 由 MBR 成功加载 Loader 后显示

	; 判断簇号的奇偶性
	mov	ax, [_LoaderClusNo]
	push	ax			; save ax
	mov	bl, 2
	div	bl			; AH = 余数, AL = 商
	mov	[_Reminder], ah
	pop	ax			; resume ax
	mov	bl, 2
	div	bl
	mov	bl, 3
	mul	bl			; AX = AL * BL
	add	ax, BaseOfFatBuf
	push	ax			; save ax
	cmp	byte [_Reminder], 0
	jnz	.odd
.even: ; 簇号为偶数
	inc	ax
	mov	si, ax
	mov	al, byte [si]
	and	ax, 0Fh
	shl	ax, 8
	pop	bx			; resume ax in bx
	mov	bl, byte [bx]
	and	bx, 0FFh
	or	ax, bx
	cmp	ax, 0FFFh		; 结束标志 0FFFh
	jz	.end
	mov	[_LoaderClusNo], ax	; 保存下一个簇号

	; 加载 KERNEL.BIN 的下一个簇
	push	word [_ppDst]		; pDst
	push	ax			; ClusNo
	call	CopyCode
	add	sp, 4
	add	word [_ppDst], 200h	; BPB_BytsPerSec * BPB_SecPerClus = 200h

	jmp	.begin

.odd: ; 簇号为奇数
	add	ax, 2
	mov	si, ax
	mov	al, byte [si]
	and	ax, 00FFh
	shl	ax, 4
	pop	bx
	inc	bx
	mov	bl, byte [bx]
	and	bx, 00FFh
	shr	bx, 4
	or	ax, bx
	cmp	ax, 0FFFh
	jz	.end
	mov	[_LoaderClusNo], ax	; 保存下一个簇号

	push	word [_ppDst]		; pDst
	push	ax			; ClusNo
	call	CopyCode
	add	sp, 4
	add	word [_ppDst], 200h	; BPB_BytsPerSec * BPB_SecPerClus = 200h

	jmp	.begin

.end:
	ret

;-----------------------------------------------------------


;-----------------------------------------------------------
; CopyCode (ClusNo, es:pDst)
; ClusNo	簇号
; es:pDst	目标地址
;-----------------------------------------------------------
CopyCode:
	push	bp
	mov	bp, sp

	; 将 ClusNo 对应扇区的数据读到缓冲区
	; SecNo = (ClusNo - 2) + DataSecNo = ClusNo + 31

	mov	ax, [bp+4]		; ClusNo
	add	ax, 31			; SecNo
	xor	cx, cx
	mov	cl, [BPB_SecPerClus]

	push	word [bp+6]		; pBuf
	push	cx			; SecCnt
	push	ax			; SecNo
	call	ReadSector
	add	sp, 6

	pop	bp
	ret	
;-----------------------------------------------------------


;-----------------------------------------------------------
; DispMsg 打印字符串
; al = 字符串在 _MsgBase 里的索引
; dh = row
;-----------------------------------------------------------
DispMsg:
	push	es
	push	ax
	
	mov	ax, ds
	mov	es, ax

	pop	ax
	mov	bl, _MsgLen
	mul	bl			; AX = AL * BL
	add	ax, _MsgBase
	mov	bp, ax			; ES:BP = 串地址
	mov	cx, _MsgLen		; CX = 串长度
	mov	ax, 01301h		; AH = 13h  AL = 01h
	mov	bx, 0007h		; 页号为0(BH = 0) 黑底灰字(BL = 07h,高亮)
	mov	dl, 0			; DL = column
	int	10h
	
	pop	es
	ret
;-----------------------------------------------------------


LABEL_READY_FOR_PM: ; 准备进入保护模式
	; 加载 GDTR
	lgdt	[GdtPtr]

	; 打开　A20 地址线
	in	al, 92h
	or	al, 0000_0010b
	out	92h, al

	; 关中断
	cli

	;  CR0 的 PE 位置1
	mov	eax, cr0
	or	eax, 1
	mov	cr0, eax

	; 进入保护模式
	jmp	dword Selector_FlatC:(BaseOfLoaderPAddr + LABEL_SEG_CODE32)

; /////////////////////////////////////////////////////////////////////////////////////////

[SECTION .c32]
ALIGN 32
[BITS 32]
LABEL_SEG_CODE32:
	mov	ax, Selector_FlatRW
	mov	ds, ax
	mov	es, ax
	mov	ss, ax
	mov	esp, ButtomOfStack

	mov	ax, Selector_Video
	mov	gs, ax

	call	DispMemInfo

	call	SetupPaging

	call	InitKernel
	jmp	Selector_FlatC:KernelEntryPointPAddr

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
; DispMemInfo	打印内存信息
;--------------------------------------------------------------------------
DispMemInfo:
	mov	esi, szMemChkTitle
	mov	edi, [dwMemDispPos]
	call	DispStr
	
	call	DispMemInfo_NewLine
	
	mov	esi, MemChkBuf
	mov	edx, esi
	mov	ecx, [dwNumOfARDS]
.loop:
	mov	ebx, 5			; ARDS 有 5 个成员
.1:
	mov	eax, [esi]
	call	Disp_DWORD
	add	edi, 22			; 移动打印位置
	add	esi, 4			; esi 指向下一个 ARDS 成员
	dec	ebx
	jnz	.1
	cmp	dword [edx + OffType], 1	;if (Type == 1)
	jne	.2				;{
	mov	eax, [edx + OffBaseAddrLow]	;	if (BaseAddrLow + LengthLow > MemSize)
	add	eax, [edx + OffLengthLow]	;	{
	cmp	eax, dword [dwMemSize]		;		MemSize = BaseAddrLow + LengthLow
	jb	.2				;	}
	mov	[dwMemSize], eax		;}
.2:
	add	edx, 20				; `. esi 指向下一个 ARDS 结构
	mov	esi, edx			;  /
	call	DispMemInfo_NewLine		;printf("\n")
	loop	.loop
	
	mov	esi, szMemSize			;
	call	DispStr				;printf("RAM Size: ")
	mov	eax, [dwMemSize]		;
	call	Disp_DWORD			;Disp_DWORD(dwMemSize)
	
	ret
;--------------------------------------------------------------------------


;--------------------------------------------------------------------------
; DispMemInfo_NewLine
;--------------------------------------------------------------------------
DispMemInfo_NewLine:
	add	dword [dwMemDispPos], 160
	mov	edi, [dwMemDispPos]
	ret
;--------------------------------------------------------------------------

;--------------------------------------------------------------------------
; InitKernel
;--------------------------------------------------------------------------
InitKernel:
	mov	ax, Selector_FlatRW
	mov	es, ax

	mov	cx, [BaseOfKernelPAddr + 2Ch]	; cx <- ELF32_Ehdr.e_phnum
	movzx	ecx, cx
	mov	esi, [BaseOfKernelPAddr + 1Ch]	; esi <- ELF32_Ehdr.e_phoff
	add	esi, BaseOfKernelPAddr		; esi 指向第一个 Program Header
.copy_seg:
	cmp	dword [esi], 1
	jnz	.end
	push	dword [esi + 10h]		; len (ELF_Phdr.p_filesz)
	mov	eax, [esi + 4]			; eax <- ELF_Phdr.p_offset
	add	eax, BaseOfKernelPAddr
	push	eax				; pSrc
	push	dword [esi + 8]			; pDst (ELF_Phdr.p_vaddr)
	call	CopyMemory
	add	esp, 12

	add	esi, 20h			; esi 指向 next Program Header (每个 Program Header 大小为 20h 字节)
	loop	.copy_seg
.end:
	ret
;--------------------------------------------------------------------------

%include	"lib32.inc"

; ///////////////////////////////// End of [SECTION .C32] /////////////////////////////////


; /////////////////////////////////////////////////////////////////////////////////////////

[SECTION .data]
ALIGN 32
LABEL_SEG_DATA:
; 实模式下使用这些符号
_szPMMessage:		db	'In Protect Mode', 0
_LoaderFileName:	db	'KERNEL  BIN'
_LoaderClusNo:		dw	0
_Reminder:		db	0
_ppDst:			dw	0
_LoopCnt:		db	RootDirItemCnt

_MemChkBuf: 	times 256 db 0
_dwNumOfARDS:		dd	0 ; 地址范围描述符结构(ARDS)个数
_szMemChkTitle:		db	'BaseAddrL  BaseAddrH  LengthLow  LengthHigh  Type', 0
_szMemSize:		db	'RAM Size: '
_dwMemSize:		dd	0
_dwMemDispPos:		dd	(80 * 4 + 0) * 2 ; DispMemInfo 打印内存信息起始位置

_MsgBase: ; 以下字符串固定长度为 13
_MsgLen			equ	13
_Msg0:			db	'No kernel    '
_Msg1:			db	'Kernel loaded'


; 保护模式下使用这些符号
; ds 指向的 GDT 描述符对应的内存区段的段基址为 0, "_szPMMessage" 等符号的数值是其
; 相对 LOADER 加载基地址的偏移.
szPMMessage		equ	BaseOfLoaderPAddr + _szPMMessage
MemChkBuf		equ	BaseOfLoaderPAddr + _MemChkBuf
dwNumOfARDS		equ	BaseOfLoaderPAddr + _dwNumOfARDS
szMemChkTitle		equ	BaseOfLoaderPAddr + _szMemChkTitle
szMemSize		equ	BaseOfLoaderPAddr + _szMemSize
dwMemSize		equ	BaseOfLoaderPAddr + _dwMemSize
dwMemDispPos		equ	BaseOfLoaderPAddr + _dwMemDispPos


; ARDS　各成员相对结构体开头的偏移
OffBaseAddrLow		equ	0
OffBaseAddrHigh		equ	4
OffLengthLow		equ	8
OffLengthHigh		equ	12
OffType			equ	16


; ///////////////////////////////// End of [SECTION .data] /////////////////////////////////

