;-------------------------------------------------------------------------------
; DispAL_Ring3: 显示 AL 中的数字 (十六进制)
; 显示位置: gs:edi 指向的显存
; 调用结束后, gs:edi 指向下一个显示位置
;-------------------------------------------------------------------------------
DispAL_Ring3:
	push	ebx
	push	ecx

	mov	bl, al
	shr	al, 4
	mov	ecx, 2
.loop_ring3:
	and	al, 0Fh
	cmp	al, 0Ah
	jb	.1_ring3
	sub	al, 0Ah
	add	al, 'A'
	jmp	.2_ring3
.1_ring3:
	add	al, '0'
.2_ring3:
	mov	ah, 0Fh			; 0000: 黑底    1111: 白字
	mov	[gs:edi], ax
	add	edi, 2

	mov	al, bl
	loop	.loop_ring3

	add	edi, 2
	pop	ecx
	pop	ebx
	ret
;-------------------------------------------------------------------------------


;-------------------------------------------------------------------------------
; Disp_DWORD_Ring3: 在 gs:edi 指向的显存显示一个 dword (十六进制)
;　调用者将 dword 存放在 eax
;-------------------------------------------------------------------------------
Disp_DWORD_Ring3:
	push	ebx
	push	ecx
	push	edx
	push	edi

	mov	edx, eax	; 保存 eax
	mov	cl, 24		; 移位位数
	mov	ebx, 4		; 循环次数

.disp_al_ring3:
	mov	eax, edx
	shr	eax, cl
	call	DispAL_Ring3
	sub	cl, 8
	sub	edi, 2		; 显示位置回退一个字符
	dec	ebx
	cmp	ebx, 0
	jg	.disp_al_ring3

	mov	al, 'h'
	mov	[gs:edi], ax

	pop	edi
	pop	edx
	pop	ecx
	pop	ebx
	ret
;-------------------------------------------------------------------------------


;-------------------------------------------------------------------------------
; PrintStack_Ring3: 打印栈
; 打印起始位置: gs:edi 指向的显存
; 打印单元数: ecx (每个单元为一个 dword)
;-------------------------------------------------------------------------------
PrintStack_Ring3:
	push	ebp
	mov	ebp, esp
.print_stack_ring3:
	mov	eax, [ss:ebp]
	call	Disp_DWORD_Ring3
	add	edi, 160		; 换行
	add	ebp, 4
	cmp	ebp, ButtomOfStack3
	je	.stack3end
	loop	.print_stack_ring3
.stack3end:
	pop	ebp
	ret
;-------------------------------------------------------------------------------
