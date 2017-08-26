;-------------------------------------------------------------------------------
; DispAL: 显示 AL 中的数字 (十六进制)
; 显示位置: gs:edi 指向的显存
; 调用结束后, gs:edi 指向下一个显示位置
;-------------------------------------------------------------------------------
DispAL:
	push	ebx
	push	ecx

	mov	bl, al
	shr	al, 4
	mov	ecx, 2
.loop:
	and	al, 0Fh
	cmp	al, 0Ah
	jb	.1
	sub	al, 0Ah
	add	al, 'A'
	jmp	.2
.1:
	add	al, '0'
.2:
	mov	ah, 0Fh			; 0000: 黑底    1111: 白字
	mov	[gs:edi], ax
	add	edi, 2

	mov	al, bl
	loop	.loop

	add	edi, 2
	pop	ecx
	pop	ebx
	ret
;-------------------------------------------------------------------------------


;-------------------------------------------------------------------------------
; DispAlChar: 显示 AL 中的 ACSII 字符
; 显示位置: gs:edi 指向的显存
; 调用结束后, gs:edi 指向下一个显示位置
;-------------------------------------------------------------------------------
DispAlChar:
	mov	ah, 0Fh			; 0000: 黑底    1111: 白字
	mov	[gs:edi], ax
	add	edi, 2
	ret
;-------------------------------------------------------------------------------


;-------------------------------------------------------------------------------
; DispStr: 显示以 0 结尾字符串
; 显示位置: gs:edi 指向的显存
; 字符串地址: ds:esi
;-------------------------------------------------------------------------------
DispStr:
.continue:
	mov	al, [ds:esi]
	cmp	al, 0
	jz	.end
	call	DispAlChar
	inc	esi
	jmp	.continue
.end:
	ret
;-------------------------------------------------------------------------------


;-------------------------------------------------------------------------------
; Disp_DWORD: 在 gs:edi 指向的显存显示一个 dword (十六进制)
;　调用者将 dword 存放在 eax
;-------------------------------------------------------------------------------
Disp_DWORD:
	push	ebx
	push	ecx
	push	edx
	push	edi

	mov	edx, eax	; 保存 eax
	mov	cl, 24		; 移位位数
	mov	ebx, 4		; 循环次数
	
.disp_al:
	mov	eax, edx
	shr	eax, cl
	call	DispAL
	sub	cl, 8
	sub	edi, 2		; 显示位置回退一个字符
	dec	ebx
	cmp	ebx, 0
	jg	.disp_al

	mov	al, 'h'
	mov	[gs:edi], ax

	pop	edi
	pop	edx
	pop	ecx
	pop	ebx
	ret
;-------------------------------------------------------------------------------


;-------------------------------------------------------------------------------
; CopyMemory(es:pDst, ds:pSrc, len)
;-------------------------------------------------------------------------------
CopyMemory:
	push	ebp
	mov	ebp, esp

	push	edi
	push	esi
	push	ecx

	mov	edi, [ebp+8]	; pDst
	mov	esi, [ebp+12]	; pSrc
	mov	ecx, [ebp+16]	; len
.memcpy:
	mov	al, [ds:esi]
	mov	[es:edi], al
	inc	esi
	inc	edi
	loop	.memcpy

	pop	ecx
	pop	esi
	pop	edi
	pop	ebp
	ret
;-------------------------------------------------------------------------------


;-------------------------------------------------------------------------------
; PrintStack: 打印栈
; 打印起始位置: gs:edi 指向的显存
; 打印单元数: ecx (每个单元为一个 dword)
;-------------------------------------------------------------------------------
PrintStack:
	push	ebp
	mov	ebp, esp
.print_stack:
	mov	eax, [ss:ebp]
	call	Disp_DWORD
	add	edi, 160		; 换行
	add	ebp, 4
	cmp	ebp, ButtomOfStack
	je	.stackend
	loop	.print_stack
.stackend:
	pop	ebp
	ret
;-------------------------------------------------------------------------------
