#include "proc.h"
#include "stdio.h"
#include "global.h"


/**
 * 调度算法:
 * <1> 将当前进程(被中断的进程)的 ticks 减 1;
 * <2> 遍历进程表, 找到 flag = 0 且 ticks 最大的进程 p,
 *     作为即将被启动或唤醒的进程;
 * <3> 如果所有进程的 ticks 都已被减至 0, 则恢复为初值.
 *     (ticks 的初值由 priority 保存)
 *
 * 在该调度算法下, 进程的运行时间比接近 priority 之比
 */
void schedule()
{
	PROCESS* p_proc;
	int max_ticks = 0;
	
	for (p_proc = &FIRST_PROC; p_proc <= &LAST_PROC; p_proc++)
	{
		if (p_proc->flag == 0)
		{
			if (p_proc->ticks > max_ticks)
			{
				max_ticks = p_proc->ticks;
				p_current_proc = p_proc;
			}
			if (max_ticks == 0)
			{
				for (p_proc = &FIRST_PROC; p_proc <= &LAST_PROC; p_proc++)
					p_proc->ticks = p_proc->priority;
			}
		}
	}
}

void clock_handler()
{
	ticks++;
	p_current_proc->ticks--;
	
	schedule();
}

