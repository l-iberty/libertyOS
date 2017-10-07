;================================================
; klib.asm 库函数
;================================================

global	memcpy
global	memset
global	memcmp
global	println
global	print
global	printmsg
global	printchar
global	backspace
global	itoa
global	in_byte
global	out_byte
global	port_read
global	port_write
global	disable_int
global	enable_int
global	PrintPos
global	MainPrintPos

[SECTION .data]
PrintPos	dd	0			; 1~* 号控制台显示字符的地址
MainPrintPos	dd	(80 * 12 + 0) * 2	; 专用于 printf 在 0 号控制台的输出

[SECTION .text]

;-------------------------------------------------------------------------------
; void memcpy(es:pDst, ds:pSrc, len)
;-------------------------------------------------------------------------------
memcpy:
	push	ebp
	mov	ebp, esp

	push	edi
	push	esi
	push	ecx

	mov	edi, [ebp+8]	; pDst
	mov	esi, [ebp+12]	; pSrc
	mov	ecx, [ebp+16]	; len
	cmp	ecx, 0
	jbe	.end
.1:
	mov	al, [ds:esi]
	mov	[es:edi], al
	inc	esi
	inc	edi
	loop	.1
.end:
	pop	ecx
	pop	esi
	pop	edi
	pop	ebp
	ret
;-------------------------------------------------------------------------------


;-------------------------------------------------------------------------------
; void memset(void* pDst, u8 value, int len)
;-------------------------------------------------------------------------------
memset:
	push	ebp
	mov	ebp, esp

	push	es	
	push	edi
	push	ecx

	mov	ax, ds
	mov	es, ax
	
	mov	edi, [ebp + 8]		; pDst
	mov	al, byte [ebp + 12]	; value
	mov	ecx, [ebp + 16]		; len
	
	rep	stosb	; (ES:EDI) <- AL
	
	pop	ecx
	pop	edi
	pop	es
	pop	ebp
	ret
;-------------------------------------------------------------------------------


;-------------------------------------------------------------------------------
; int memcmp(void* p1, void* p2, int len)
; # Notes:
; #	a.逐字节比较 p1 和 p2 指向的内存, 段地址默认 ds
; #	b.比较长度(字节数) = len
; #	c.返回值: 0 = equal, 1 = not equal
;-------------------------------------------------------------------------------
memcmp:
	push	ebp
	mov	ebp, esp
	
	push	ecx
	push	edi
	push	esi
	
	mov	edi, [ebp + 8]	; p1
	mov	esi, [ebp + 12]	; p2
	mov	ecx, [ebp + 16]	; len
	
.lcmp:	mov	bl, byte [edi]
	mov	bh, byte [esi]
	cmp	bh, bl
	jne	.notequal
	inc	edi
	inc	esi
	loop	.lcmp
.equal:
	xor	eax, eax
	jmp	.end
.notequal:
	mov	eax, 1
.end:
	pop	esi
	pop	edi
	pop	ecx
	pop	ebp
	ret
;-------------------------------------------------------------------------------


;-------------------------------------------------------------------------------
; void println(char* sz)
; 字符串 sz 的段选择器默认使用 ds
;-------------------------------------------------------------------------------
println:
	push	ebp
	mov	ebp, esp

	push	edi
	push	esi

	mov	esi, [ebp + 8]			; sz　字符串地址
	mov	edi, dword [MainPrintPos]
	mov	ah, 07h				; 0000 黑底, 0111 灰字
.printc:
	mov	al, [esi]
	cmp	al, 0
	jz	.end	
	cmp	al, 0Ah				; 是 '\n' 吗?
	jnz	.1
	call	newline
	mov	dword [MainPrintPos], edi
	jmp	.2
.1:
	mov	[gs:edi], ax
	add	edi, 2
.2:
	inc	esi
	jmp	.printc
.end:
	add	dword [MainPrintPos], 160	; 下次打印时另起一行
	pop	esi
	pop	edi
	pop	ebp
	ret
;-------------------------------------------------------------------------------


;-------------------------------------------------------------------------------
; void print(char* sz)
; 字符串 sz 的段选择器默认使用 ds
;-------------------------------------------------------------------------------
print:
	push	ebp
	mov	ebp, esp

	push	edi
	push	esi
	
	mov	esi, [ebp + 8]			; sz　字符串地址
	mov	edi, dword [MainPrintPos]
	mov	ah, 07h				; 0000 黑底, 0111 灰字
.printc:
	mov	al, [esi]
	cmp	al, 0
	jz	.end	
	cmp	al, 0Ah				; 是 '\n' 吗?
	jnz	.1
	call	newline
	mov	dword [MainPrintPos], edi
	jmp	.2
.1:
	mov	[gs:edi], ax
	add	edi, 2
.2:
	inc	esi
	jmp	.printc
.end:
	mov	dword [MainPrintPos], edi

	pop	esi
	pop	edi
	pop	ebp
	ret
;-------------------------------------------------------------------------------


;-------------------------------------------------------------------------------
; void printmsg(char* sz, u8 color, u32 pos)
; 字符串 sz 的段选择器默认使用 ds
;-------------------------------------------------------------------------------
printmsg:
	push	ebp
	mov	ebp, esp

	push	edi
	push	esi

	mov	esi, [ebp + 8]			; sz　字符串地址
	mov	ah, byte [ebp + 12]		; color
	mov	edi, [ebp + 16]			; pos
.printc:
	mov	al, [esi]
	cmp	al, 0
	jz	.end	
	cmp	al, 0Ah				; 是 '\n' 则回车换行
	jnz	.1
	call	newline
	jmp	.2
.1:
	mov	[gs:edi], ax
	add	edi, 2
.2:
	inc	esi
	jmp	.printc
.end:
	pop	esi
	pop	edi
	pop	ebp
	ret
;-------------------------------------------------------------------------------


;-------------------------------------------------------------------------------
; newline 将显示位置移动到下一行行首.
; 默认 edi 存放着显示位置, 该函数将改变 edi 的内容:
; edi = (edi / 160) * 160 + 160
;-------------------------------------------------------------------------------
newline:
	push	eax
	push	ebx
	
	mov	eax, edi
	mov	ebx, 160
	div	bl ; 16 bits 被除数 AX, 8 bits 除数 BL
		   ; AH = 余数, AL = 商
	and	eax, 0FFh ; 取 AL
	mul	ebx	  ; (EDX,EAX)<-(EAX)*(SRC)
	mov	edi, eax  ; 当前行首
	add	edi, 160  ; 下一行行首
	
	pop	ebx
	pop	eax
	ret
;-------------------------------------------------------------------------------


;-------------------------------------------------------------------------------
; void printchar(char ch)
;-------------------------------------------------------------------------------
printchar:
	push	ebp
	mov	ebp, esp
	push	edi
	
	mov	edi, dword [PrintPos]
	mov	ah, 07h			; 0000 黑底, 0111 灰字
	mov	al, [ebp + 8]
	cmp	al, 0Ah			; 是 '\n' 吗?
	jnz	.1
	call	newline
	mov	dword [PrintPos], edi
	jmp	.2
.1:
	mov	[gs:edi], ax
	add	dword [PrintPos], 2
.2:
	pop	edi
	pop	ebp
	ret
;-------------------------------------------------------------------------------


;-------------------------------------------------------------------------------
; void backspace()
;-------------------------------------------------------------------------------
backspace:
	push	edi
	
	sub	dword [PrintPos], 2
	mov	edi, dword [PrintPos]
	mov	ah, 07h				; 0000 黑底, 0111 灰字
	mov	al, ' '
	mov	[gs:edi], ax
	
	pop	edi
	ret
;-------------------------------------------------------------------------------


;-------------------------------------------------------------------------------
; void itoa(char* str, int v, int len, u8 flag)
; flag: 是否在 str 末尾添加 '\0'
; 例:
; char str[4 + 1];
; itoa(str, 0x1234, 4); // 便可从 str 取得字符串 "1234"
;-------------------------------------------------------------------------------
itoa:
	push	ebp
	mov	ebp, esp

	push	ecx
	push	esi

	mov	edx, [ebp + 16]		; len
	mov	eax, [ebp + 12]		; v
	mov	ebx, eax
	mov	esi, [ebp + 8]		; str
	add	esi, edx		; esi <- &str[len]
	mov	cl, [ebp + 20]		; flag
	cmp	cl, 0
	jz	.false
	mov	byte [esi], 0		; '\0'
.false:
	dec	esi
	mov	cl, 0
.loop:
	shr	eax, cl
	and	al, 0Fh
	cmp	al, 9
	jg	.1
	add	al, '0'
	jmp	.2
.1:
	sub	al, 0Ah
	add	al, 'A'
.2:
	mov	[esi], al
	dec	esi
	add	cl, 4
	mov	eax, ebx
	dec	edx
	jnz	.loop

	pop	esi
	pop	ecx
	pop	ebp
	ret
;-------------------------------------------------------------------------------


;-------------------------------------------------------------------------------
; u8 in_byte(u16 port)
;-------------------------------------------------------------------------------
in_byte:
	xor	eax, eax
	mov	edx, [esp + 4]	; port
	in	al, dx
	nop
	nop
	ret
;-------------------------------------------------------------------------------


;-------------------------------------------------------------------------------
; void out_byte(u16 port, u8 byte)
;-------------------------------------------------------------------------------
out_byte:
	push	ebp
	mov	ebp, esp
	
	mov	edx, [ebp + 8]		; port
	mov	al, byte [ebp + 12]	; byte
	out	dx, al
	
	pop	ebp
	ret
;-------------------------------------------------------------------------------


;-------------------------------------------------------------------------------
; void port_read(u16 port, void* buf, int len)
;-------------------------------------------------------------------------------
port_read:
	push	ebp
	mov	ebp, esp
	push	edx
	push	edi
	push	ecx
	
	mov	edx, [ebp + 8]		; port
	mov	edi, [ebp + 12]		; buf
	mov	ecx, [ebp + 16]		; len
	cld
	rep	insw ; 从 EDX 指定的 I/O 端口将字读入 ES:EDI 指定的内存
	
	pop	ecx
	pop	edi
	pop	edx
	pop	ebp
	ret
;-------------------------------------------------------------------------------


;-------------------------------------------------------------------------------
; void port_write(u16 port, void* buf, int len)
;-------------------------------------------------------------------------------
port_write:
	push	ebp
	mov	ebp, esp
	push	edx
	push	esi
	push	ecx
	
	mov	edx, [ebp + 8]		; port
	mov	esi, [ebp + 12]		; buf
	mov	ecx, [ebp + 16]		; len
	cld
	rep	outsw ; output word from memory location specified in DS:ESI to I/O port specified in EDX
	
	pop	ecx
	pop	esi
	pop	edx
	pop	ebp
	ret
;-------------------------------------------------------------------------------


;-------------------------------------------------------------------------------
; void disable_int()
;-------------------------------------------------------------------------------
disable_int:
	cli
	ret
;-------------------------------------------------------------------------------


;-------------------------------------------------------------------------------
; void enable_int()
;-------------------------------------------------------------------------------
enable_int:
	sti
	ret
;-------------------------------------------------------------------------------

