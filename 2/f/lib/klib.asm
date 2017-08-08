;================================================
; klib.asm 内核库函数
;================================================


global	memcpy
global	println


[SECTION .data]
PrintPos	dd	(80 * 6 + 0) * 2	; println 显示字符串的地址
StrColor	db	07h			; println 显示字符串的颜色 (0000 黑底, 0111 灰字)


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
; println(ds:psz)
;-------------------------------------------------------------------------------
println:
	push	ebp
	mov	ebp, esp

	push	edi
	push	esi
		
	mov	esi, [ebp + 8]			; psz　字符串地址
	mov	edi, dword [PrintPos]
	mov	ah, byte [StrColor]
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
