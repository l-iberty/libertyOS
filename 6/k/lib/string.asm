;================================================
; string.asm 字符串操作
;================================================

global	_strlen
global	_strcpy


[SECTION .text]

;-------------------------------------------------------------------------------
; size_t _strlen(char* s)
;-------------------------------------------------------------------------------
_strlen:
	push	ebp
	mov	ebp, esp
	push	esi

	mov	esi, [ebp + 8]
	xor	eax, eax
.1:	
	cmp	byte [esi], 0
	je	.end
	inc	eax
	inc	esi
	jmp	.1
.end:
	pop	esi
	pop	ebp
	ret
;-------------------------------------------------------------------------------


;-------------------------------------------------------------------------------
; void _strcpy(char* dst, const char* src)
;-------------------------------------------------------------------------------
_strcpy:
	push	ebp
	mov	ebp, esp
	push	esi
	push	edi
	push	es
	
	mov	ax, ds
	mov	es, ax
	mov	edi, [ebp + 8]	; char* dst
	mov	esi, [ebp + 12]	; char* src
	cld
.1:
	cmp	byte [esi], 0
	je	.end
	movsb			; es:edi <- ds:esi
	jmp	.1
.end:
	movsb			; '\0'
	pop	es
	pop	edi
	pop	esi
	pop	ebp
	ret	
;-------------------------------------------------------------------------------

