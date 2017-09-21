;================================================
; string.asm 字符串操作
;================================================

global	_strlen
global	_strcpy
global	_strcmp


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
	pop	edi
	pop	esi
	pop	ebp
	ret	
;-------------------------------------------------------------------------------


;-------------------------------------------------------------------------------
; int _strcmp(const char* s1, const char* s2, int len)
;
; @return 0 = equal, 1 = not equal
;-------------------------------------------------------------------------------
_strcmp:
	push	ebp
	mov	ebp, esp
	push	esi
	push	edi
	push	ecx
	
	mov	esi, [ebp + 8]	; s1
	mov	edi, [ebp + 12]	; s2
	mov	ecx, [ebp + 16]	; len
	cld
	repz	cmpsb		; 比较 ds:esi 与 es:edi, 相等则置 ZF = 0 并重复, 否则置 ZF = 1 并终止
	jnz	.ne		; repz 终止后若 ZF = 0, 则终止因素是 ecx = 0, 即:
				; 比较了 ecx 次后依然未发现不相等字符, 则 s1 = s2
	xor	eax, eax
	jmp	.e
.ne:
	mov	eax, 1
.e:
	pop	ecx
	pop	edi
	pop	esi
	pop	ebp
	ret
;-------------------------------------------------------------------------------


