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

#endif /* SYSCONST_H */
