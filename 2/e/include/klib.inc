;================================================
; klib.inc 用于内核 kernel.asm
;================================================

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
