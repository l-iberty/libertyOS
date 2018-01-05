# 对中断服务程序的改进
之前的设计中，重入的中断无法执行其具体的服务例程，使得系统经常因键盘操作得不到相应而卡机.

## 8259A-master
```
; master -------------------------------------------------------------------
%macro hwint_master 1
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
	
	inc	dword [f_reenter]
	cmp	dword [f_reenter], 0	; f_reenter = 0 则没有发生中断重入
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
```

## 8259A-slave
```
; slave -------------------------------------------------------------------
%macro hwint_slave 1
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
	
	inc	dword [f_reenter]
	cmp	dword [f_reenter], 0	; f_reenter = 0 则没有发生中断重入
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
```

## system call
```
sys_call:
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
	inc	dword [f_reenter]
	cmp	dword [f_reenter], 0	; f_reenter = 0 则没有发生中断重入
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
```

## 测试结果正确
![](screenshot/f.png)

而且键盘响应比以前流畅多了.