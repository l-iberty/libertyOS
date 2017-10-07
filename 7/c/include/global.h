/**
 * 全局变量 & 系统调用
 */

#ifndef GLOBAL_H
#define GLOBAL_H
#include "proc.h"

#define min(a,b) ((a>b)?b:a)
#define max(a,b) ((a>b)?a:b)

/* variables */
int	ticks;			/* get_ticks() 的返回值, 记录时钟中断的次数 */	
int	nr_current_console;	/* 当前控制台号 */

extern u8	GDT[GDT_DESC_NUM * DESC_SIZE];	/* kernel/kernel.asm */
extern u8	IDT[IDT_DESC_NUM * DESC_SIZE];	/* kernel/kernel.asm */
extern u8	TSS[TSS_SIZE];			/* kernel/kernel.asm */

/* prototypes */
int	get_ticks();
int	sendrecv(int func_type, int pid, MESSAGE* p_msg);


#endif /* GLOBAL_H */
