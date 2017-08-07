;===================================================
;　kernel.asm
; 编译链接:
; nasm -f elf kernel.asm -o kernel.o
; ld -s -melf_i386 kernel.o -o kernel.bin
;===================================================

[section .text]

global	_start

_start:
	mov	ah, 0Ch
	mov	al, 'K'
	mov	[gs:(80 * 3 + 39) * 2], ax
	jmp	$
