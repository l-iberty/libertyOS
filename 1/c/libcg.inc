;-------------------------------------------------------------------------------
; DispAL_CGate: 显示 AL 中的数字 (十六进制)
; 显示位置: gs:edi 指向的显存
; 调用结束后, gs:edi 指向下一个显示位置
;-------------------------------------------------------------------------------
DispAL_CGate:
	push	ebx
	push	ecx

	mov	bl, al
	shr	al, 4
	mov	ecx, 2
.loop_cgate:
	and	al, 0Fh
	cmp	al, 0Ah
	jb	.1_cgate
	sub	al, 0Ah
	add	al, 'A'
	jmp	.2_cgate
.1_cgate:
	add	al, '0'
.2_cgate:
	mov	ah, 0Fh			; 0000: 黑底    1111: 白字
	mov	[gs:edi], ax
	add	edi, 2

	mov	al, bl
	loop	.loop_cgate

	add	edi, 2
	pop	ecx
	pop	ebx
	ret
;-------------------------------------------------------------------------------


;-------------------------------------------------------------------------------
; Disp_DWORD_CGate: 在 gs:edi 指向的显存显示一个 dword (十六进制)
;　调用者将 dword 存放在 eax
;-------------------------------------------------------------------------------
Disp_DWORD_CGate:
	push	ebx
	push	ecx
	push	edx
	push	edi

	mov	edx, eax	; 保存 eax
	mov	cl, 24		; 移位位数
	mov	ebx, 4		; 循环次数

.disp_al_cgate:
	mov	eax, edx
	shr	eax, cl
	call	DispAL_CGate
	sub	cl, 8
	sub	edi, 2		; 显示位置回退一个字符
	dec	ebx
	cmp	ebx, 0
	jg	.disp_al_cgate

	pop	edi
	pop	edx
	pop	ecx
	pop	ebx
	ret
;-------------------------------------------------------------------------------


;-------------------------------------------------------------------------------
; PrintStack_CGate: 打印栈
; 打印起始位置: gs:edi 指向的显存
; 打印单元数: ecx (每个单元为一个 dword)
;-------------------------------------------------------------------------------
PrintStack_CGate:
	mov	ebx, esp
.print_stack_cgate:
	mov	eax, [ss:ebx]
	call	Disp_DWORD_CGate
	add	edi, 160		; 换行
	add	ebx, 4
	loop	.print_stack_cgate

	ret
;-------------------------------------------------------------------------------
