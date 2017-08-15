;================================================
; klib.asm 库函数
;================================================


global	memcpy
global	println
global	print
global	printmsg
global	printchar
global	itoa
global	in_byte
global	out_byte

[SECTION .data]
PrintPos	dd	(80 * 6 + 0) * 2	; println 显示字符串的地址 (初始值: 6 行 0 列)


[SECTION .text]

;-------------------------------------------------------------------------------
; memcpy(es:pDst, ds:pSrc, len)
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
.1:
	mov	al, [ds:esi]
	mov	[es:edi], al
	inc	esi
	inc	edi
	loop	.1

	pop	ecx
	pop	esi
	pop	edi
	pop	ebp
	ret
;-------------------------------------------------------------------------------


[SECTION .text]
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
	mov	edi, dword [PrintPos]
	mov	ah, 07h				; 0000 黑底, 0111 灰字
.printc:
	mov	al, [esi]
	cmp	al, 0
	jz	.end	
	cmp	al, 0Ah				; 是 '\n' 则回车换行
	jnz	.1
	add	edi, 160
	jmp	.2
.1:
	mov	[gs:edi], ax
	add	edi, 2
.2:
	inc	esi
	jmp	.printc
.end:
	add	dword [PrintPos], 160	; 下次打印时另起一行
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
	push	ecx
	
	xor	ecx, ecx			; ecx 统计字符串长度
	mov	esi, [ebp + 8]			; sz　字符串地址
	mov	edi, dword [PrintPos]
	mov	ah, 07h				; 0000 黑底, 0111 灰字
.printc:
	mov	al, [esi]
	cmp	al, 0
	jz	.end	
	cmp	al, 0Ah				; 是 '\n' 则回车换行
	jnz	.1
	add	edi, 160
	jmp	.2
.1:
	mov	[gs:edi], ax
	add	edi, 2
.2:
	inc	esi
	inc	ecx
	jmp	.printc
.end:
	add	ecx, ecx			; ecx = ecx * 2
	add	dword [PrintPos], ecx		; 不换行, 仅后移一个位置
	
	pop	ecx
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
	add	edi, 160
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
; void printchar(char ch)
;-------------------------------------------------------------------------------
printchar:
	push	ebp
	mov	ebp, esp
	push	edi
	
	mov	edi, dword [PrintPos]
	mov	ah, 07h				; 0000 黑底, 0111 灰字
	mov	al, [ebp + 8]
	mov	[gs:edi], ax
	add	dword [PrintPos], 2

	pop	edi
	pop	ebp
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
