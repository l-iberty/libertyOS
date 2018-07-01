/**
 * sysconst.h 系统级常量
 */

#ifndef	SYSCONST_H
#define SYSCONST_H

/* boot/include/pm.inc */
#define DA_C32		0x4098
#define DA_D32		0x4092
#define DA_S32_L	0x4096
#define DA_S32_H	0x4092
#define DA_C16		0x98
#define DA_LDT		0x4082
#define DA_CGATE	0x8C
#define DA_IGATE	0x8E
#define DA_TSS		0x89
#define DA_G_4K		0x8000
#define DPL_0		0
#define DPL_1		32
#define DPL_2		64
#define DPL_3		96
#define SA_RPL_0	0
#define SA_RPL_1	1
#define SA_RPL_2	2
#define SA_RPL_3	3
#define SA_TI_GDT	0
#define SA_TI_LDT	4

#define SELECTOR_VIDEO		8				/* \		*/
#define SELECTOR_FLATC		32				/* |		*/
#define SELECTOR_FLATRW		40				/* |		*/
#define	SELECTOR_TSS		48				/* | 选择子	*/
#define SELECTOR_LDT_FIRST	56				/* |		*/
#define SELECTOR_LDT_FLATC	(0 | SA_RPL_3 | SA_TI_LDT)	/* |		*/
#define SELECTOR_LDT_FLATRW	(8 | SA_RPL_3 | SA_TI_LDT)	/* /		*/

#define GDT_DESC_NUM		256	/* GDT 描述符个数 */
#define	IDT_DESC_NUM		256	/* IDT 描述符个数, 256 个中断 */
#define LDT_DESC_NUM		2	/* LDT 描述符个数 */
#define INDEX_LDT_C		0
#define INDEX_LDT_RW		1
#define	DESC_SIZE		8	/* 8　字节描述符 */
#define TSS_SIZE		104
#define INDEX_TSS_DESC		6	/* GDT 中索引为 6 的描述符是 TSS 描述符 */
#define INDEX_LDT_DESC_FIRST	7	/* GDT 中索引为 7 的描述符是第一个进程 TaskA 的 LDT 描述符 */
#define	NR_PROCS		32	/* 进程数 */
#define NR_NATIVE_PROCS		4	/* 系统初启动时的进程数 */
#define NR_TASKS		4	/* 任务数 */
#define NR_SYSCALL		8	/* 系统调用数 */
#define NR_IRQ			16	/* 主从8259A芯片引脚数 */

#define SEND			1			/* \			*/
#define RECEIVE			2			/* | sendrecv 功能号	*/
#define BOTH			(SEND | RECEIVE)	/* /			*/

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

#define INT_VECTOR_SYSCALL	0x80	/* system call */

/* CMOS ARM */
#define CMOS_ADDR_REG		0x70
#define CMOS_DATA_REG		0x71
#define NR_SECOND		0
#define NR_MINUTE		2
#define NR_HOUR			4
#define NR_DAY                  7
#define NR_MONTH                8
#define NR_YEAR                 9

/* VGA Registers */
#define CRTC_ADDR_REG		0x3D4
#define CRTC_DATA_REG		0x3D5

/* CRT Controller Data Registers */
#define START_ADDR_H		0xC
#define START_ADDR_L		0xD
#define CURSOR_LOCATION_H	0xE
#define CURSOR_LOCATION_L	0xF

#define V_MEM_SIZE		0x8000	/* 显存总大小 = 32K: B8000h ~ BFFFFh */
#define V_MEM_BASE		0xB8000
#define SCREEN_WIDTH		80
#define SCREEN_HIGH		25
#define SCREEN_SIZE		(SCREEN_HIGH * SCREEN_WIDTH)
#define V_MEM_SCREEN		(SCREEN_SIZE * 2) /* 一个屏幕映射到显存空间的大小 = 80*25*2 = 4000 字节 */

#define NR_CONSOLES		4	/* 控制台个数 (也是终端个数) */
#define NR_FILES		32	/* 根目录最大文件数 = SECTOR_SIZE / sizeof(DIR_ENTRY) */

/* I/O ports for 8253/8254 PIT (Programmable Interval Timer) */
#define TIMER_0			0x40
#define TIMER_1			0x41
#define TIMER_2			0x42
#define TIMER_MODE_CTL		0x43

/* 8253/8254 PIT mode */
#define RATE_GENERATOR		0x34 /* 00-11-010-0 */
				     /* Counter0 - Low-Byte then High-Byte - rate generator - binary */
#define TIMER_IN_FREQ		1193180L /* input frequency (Hz) for timer in　PC */
#define CLK_FREQ		100	/* clock frequency (Hz) */

#endif /* SYSCONST_H */
