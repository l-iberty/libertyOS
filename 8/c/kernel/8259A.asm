;================================================
; 8259A.asm 初始化 8259A 芯片
;================================================

%include "const.inc"

global	init_8259A

[SECTION .text]

;--------------------------------------------------------------------------
; init_8259A
;--------------------------------------------------------------------------
init_8259A:
	mov	al, 00010001b
	out	INT_M_CTL, al		; 主8259A, ICW1.
	call	io_delay

	out	INT_S_CTL, al		; 从8259A, ICW1.
	call	io_delay

	mov	al, 020h		; IRQ0 对应中断向量 0x20
	out	INT_M_CTLMASK, al	; 主8259A, ICW2.
	call	io_delay

	mov	al, 028h		; IRQ8 对应中断向量 0x28
	out	INT_S_CTLMASK, al	; 从8259A, ICW2.
	call	io_delay

	mov	al, 00000100b		; 主片的 2 号引脚级联到从片
	out	INT_M_CTLMASK, al	; 主8259A, ICW3.
	call	io_delay

	mov	al, 00000100b		; 从片级联到主片的 2 号引脚
	out	INT_S_CTLMASK, al	; 从8259A, ICW3.
	call	io_delay

	mov	al, 00000001b		; AEOI=0 非自动结束方式
	out	INT_M_CTLMASK, al	; 主8259A, ICW4.
	call	io_delay

	out	INT_S_CTLMASK, al	; 从8259, ICW4.
	call	io_delay

	mov	al, 0FFh		; 屏蔽主8259A所有中断
	out	INT_M_CTLMASK, al	; 主8259A, OCW1.
	call	io_delay

	mov	al, 0FFh		; 屏蔽从8259A所有中断
	out	INT_S_CTLMASK, al	; 从8259A, OCW1.
	call	io_delay

	ret
;--------------------------------------------------------------------------


;--------------------------------------------------------------------------
; io_delay
;--------------------------------------------------------------------------
io_delay:
	nop
	nop
	nop
	nop
	ret
;--------------------------------------------------------------------------
