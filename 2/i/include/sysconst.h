/**
 * sysconst.h 系统级常量
 */

#ifndef	SYSCONST_H
#define SYSCONST_H

#define DA_IGATE		0x8E	/* 1_00_0_1110b, 详见 boot/include/pm.inc */
#define SELECTOR_FLATC		0x20	/* 与 kernel/kernel.asm 里的定义同名等值 */
#define	INT_NUM			256	/* 256 个中断 */
#define	DESC_SIZE		8	/* 8　字节描述符 */

/* 中断向量 */
#define INT_VECTOR_DE		0
#define INT_VECTOR_DB		1
#define INT_VECTOR_02H		2
#define INT_VECTOR_BP		3
#define INT_VECTOR_OF		4
#define INT_VECTOR_BR		5
#define INT_VECTOR_UD		6
#define INT_VECTOR_NM		7
#define INT_VECTOR_DF		8
#define INT_VECTOR_09H		9
#define INT_VECTOR_TS		10
#define INT_VECTOR_NP		11
#define INT_VECTOR_SS		12
#define INT_VECTOR_GP		13
#define INT_VECTOR_PF		14
#define INT_VECTOR_0EH		15
#define INT_VECTOR_MF		16
#define INT_VECTOR_AC		17
#define INT_VECTOR_MC		18
#define INT_VECTOR_XM		19
#define	INT_VECTOR_IRQ00	0x20	/* 时钟, 主 8259A, IRQ0 */
#define	INT_VECTOR_IRQ01	0x21	/* 键盘, 主 8259A, IRQ1 */
#define	INT_VECTOR_IRQ02	0x22
#define	INT_VECTOR_IRQ03	0x23
#define	INT_VECTOR_IRQ04	0x24
#define	INT_VECTOR_IRQ05	0x25
#define	INT_VECTOR_IRQ06	0x26
#define	INT_VECTOR_IRQ07	0x27
#define	INT_VECTOR_IRQ08	0x28
#define	INT_VECTOR_IRQ09	0x29
#define	INT_VECTOR_IRQ10	0x2A
#define	INT_VECTOR_IRQ11	0x2B
#define	INT_VECTOR_IRQ12	0x2C
#define	INT_VECTOR_IRQ13	0x2D
#define	INT_VECTOR_IRQ14	0x2E
#define	INT_VECTOR_IRQ15	0x2F

#endif /* SYSCONST_H */
