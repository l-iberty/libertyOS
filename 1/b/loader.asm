;===================================================
;　loader.asm
; 编译方法: nasm loader.asm -o loader.bin
;===================================================
	org	0ae00h			; 最好与 boot.asm 中的 BaseOfLoader 相等
	mov	ax, 0B800h
	mov	gs, ax
	mov	ah, 0Ch			; 0000 黑底, 1100 红字
	mov	al, 'L'
	mov	di, (80 * 0 + 39) * 2	; 屏幕第 0 行, 第 39 列
	mov	[gs:di], ax
	jmp	$
