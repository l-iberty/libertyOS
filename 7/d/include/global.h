/**
 * 全局变量 & 系统调用
 */

#ifndef GLOBAL_H
#define GLOBAL_H
#include "proc.h"

#define min(a,b) ((a>b)?b:a)
#define max(a,b) ((a>b)?a:b)

/* variables */
int		ticks;			/* get_ticks() 的返回值, 记录时钟中断的次数 */	
int		nr_current_console;	/* 当前控制台号 */
PROCESS*	p_current_proc;		/* 当前进程 */

extern u8	GDT[GDT_DESC_NUM * DESC_SIZE];	/* kernel/kernel.asm */
extern u8	IDT[IDT_DESC_NUM * DESC_SIZE];	/* kernel/kernel.asm */
extern u8	TSS[TSS_SIZE];			/* kernel/kernel.asm */

/* prototypes */
int	get_ticks();
int	sendrecv(int func_type, int pid, MESSAGE* p_msg);
void	disp_ldt(); /* 打印当前进程对应的 LDT 信息 */
u32	getpid();
u32	getppid();


#endif /* GLOBAL_H */
