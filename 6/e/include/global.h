/**
 * 全局变量 & 系统调用
 */

#ifndef GLOBAL_H
#define GLOBAL_H
#include "proc.h"

/* variables */
int	ticks;			/* get_ticks() 的返回值, 记录时钟中断的次数 */	
int	nr_current_console;	/* 当前控制台号 */

/* prototypes */
int	get_ticks();
int	sendrecv(int func_type, int pid, MESSAGE* p_msg);


#endif /* GLOBAL_H */
