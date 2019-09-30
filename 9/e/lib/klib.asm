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
global	disable_paging
global	enable_paging
global	getcr2
global	load_cr3
global	sys_getcr3

global	print_pos
global	main_print_pos

[SECTION .data]
print_pos	dd	0			; 1~* 号控制台显示字符的地址
main_print_pos	dd	(80 * 12 + 0) * 2	; 专用于 printf 在 0 号控制台的输出

[SECTION .text]

;-------------------------------------------------------------------------------
; void println(const char* sz)
; 字符串 sz 的段选择器默认使用 ds
;-------------------------------------------------------------------------------
println:
	push	ebp
	mov	ebp, esp

	push	edi
	push	esi

	mov	esi, [ebp + 8]			; sz　字符串地址
	mov	edi, dword [main_print_pos]
	mov	ah, 07h				; 0000 黑底, 0111 灰字
.printc:
	mov	al, [esi]
	cmp	al, 0
	jz	.end	
	cmp	al, 0Ah				; 是 '\n' 吗?
	jnz	.1
	call	newline
	mov	dword [main_print_pos], edi
	jmp	.2
.1:
	mov	[gs:edi], ax
	add	edi, 2
.2:
	inc	esi
	jmp	.printc
.end:
	add	dword [main_print_pos], 160	; 下次打印时另起一行
	pop	esi
	pop	edi
	pop	ebp
	ret
;-------------------------------------------------------------------------------


;-------------------------------------------------------------------------------
; void print(const char* sz)
; 字符串 sz 的段选择器默认使用 ds
;-------------------------------------------------------------------------------
print:
	push	ebp
	mov	ebp, esp

	push	edi
	push	esi
	
	mov	esi, [ebp + 8]			; sz　字符串地址
	mov	edi, dword [main_print_pos]
	mov	ah, 07h				; 0000 黑底, 0111 灰字
.printc:
	mov	al, [esi]
	cmp	al, 0
	jz	.end	
	cmp	al, 0Ah				; 是 '\n' 吗?
	jnz	.1
	call	newline
	mov	dword [main_print_pos], edi
	jmp	.2
.1:
	mov	[gs:edi], ax
	add	edi, 2
.2:
	inc	esi
	jmp	.printc
.end:
	mov	dword [main_print_pos], edi

	pop	esi
	pop	edi
	pop	ebp
	ret
;-------------------------------------------------------------------------------


;-------------------------------------------------------------------------------
; void printmsg(const char* sz, uint8_t color, uint32_t pos)
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
	
	mov	edi, dword [print_pos]
	mov	ah, 07h			; 0000 黑底, 0111 灰字
	mov	al, [ebp + 8]
	cmp	al, 0Ah			; 是 '\n' 吗?
	jnz	.1
	call	newline
	mov	dword [print_pos], edi
	jmp	.2
.1:
	mov	[gs:edi], ax
	add	dword [print_pos], 2
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
	
	sub	dword [print_pos], 2
	mov	edi, dword [print_pos]
	mov	ah, 07h				; 0000 黑底, 0111 灰字
	mov	al, ' '
	mov	[gs:edi], ax
	
	pop	edi
	ret
;-------------------------------------------------------------------------------


;-------------------------------------------------------------------------------
; void itoa(char* str, int v, int len, uint8_t flag)
; flag: 是否在 str 末尾添加 '\0'
; 例:
; char str[4 + 1];
; itoa(str, 0x1234, 4, 1); // 便可从 str 取得字符串 "1234"
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
; uint8_t in_byte(uint16_t port)
;-------------------------------------------------------------------------------
in_byte:
	mov	edx, [esp + 4]	; port
	xor	eax, eax
	in	al, dx
	nop
	nop
	ret
;-------------------------------------------------------------------------------


;-------------------------------------------------------------------------------
; void out_byte(uint16_t port, uint8_t byte)
;-------------------------------------------------------------------------------
out_byte:
	mov	edx, [esp + 4]	; port
	mov	al, [esp + 8]	; byte
	out	dx, al
	nop
	nop
	ret
;-------------------------------------------------------------------------------


;-------------------------------------------------------------------------------
; void port_read(uint16_t port, void* buf, int len)
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
; void port_write(uint16_t port, void* buf, int len)
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

;-------------------------------------------------------------------------------
; void disable_paging()
;-------------------------------------------------------------------------------
disable_paging:
	mov	eax, cr0
	and	eax, 07FFFFFFFh
	mov	cr0, eax
	ret
;-------------------------------------------------------------------------------

;-------------------------------------------------------------------------------
; void enable_paging()
;-------------------------------------------------------------------------------
enable_paging:
	mov	eax, cr0
	or	eax, 80000000h
	mov	cr0, eax
	ret
;-------------------------------------------------------------------------------

;-------------------------------------------------------------------------------
; uint32_t getcr2()
;-------------------------------------------------------------------------------
getcr2:
	mov	eax, cr2
	ret
;-------------------------------------------------------------------------------

;-------------------------------------------------------------------------------
; uint32_t load_cr3(uint32_t cr3)
;-------------------------------------------------------------------------------
load_cr3:
	mov	eax, [esp + 4]
	mov	cr3, eax
	ret
;-------------------------------------------------------------------------------

;-------------------------------------------------------------------------------
; uint32_t sys_getcr3()
;-------------------------------------------------------------------------------
sys_getcr3:
	mov	eax, cr3
	ret
;-------------------------------------------------------------------------------

