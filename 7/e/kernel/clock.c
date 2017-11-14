#include "proc.h"
#include "stdio.h"
#include "stdlib.h"
#include "global.h"
#include "protect.h"
#include "irq.h"


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
	
	for (p_proc = &FIRST_PROC; p_proc <= &LAST_PROC; p_proc++) {
		if (p_proc->flag == 0) {
			if (p_proc->ticks > max_ticks) {
				max_ticks = p_proc->ticks;
				p_current_proc = p_proc;
			}
		}
	}
	
	if (max_ticks == 0) {
		for (p_proc = &FIRST_PROC; p_proc <= &LAST_PROC; p_proc++)
			p_proc->ticks = p_proc->priority;
	}
}

void disp_time()
{
	u8 second, minute, hour, day, month, year;
	
	out_byte(CMOS_ADDR_REG, NR_SECOND);
	second = in_byte(CMOS_DATA_REG);
	
	out_byte(CMOS_ADDR_REG, NR_MINUTE);
	minute = in_byte(CMOS_DATA_REG);
	
	out_byte(CMOS_ADDR_REG, NR_HOUR);
	hour = in_byte(CMOS_DATA_REG);
	
	out_byte(CMOS_ADDR_REG, NR_DAY);
	day = in_byte(CMOS_DATA_REG);
	
	out_byte(CMOS_ADDR_REG, NR_MONTH);
	month = in_byte(CMOS_DATA_REG);
	
	out_byte(CMOS_ADDR_REG, NR_YEAR);
	year = in_byte(CMOS_DATA_REG);
	
	char buf[18];
	sprintf(buf, "%.2x/%.2x/%.2x %.2x:%.2x:%.2x",
			year, month, day, hour, minute, second);
	printmsg(buf, 0x0C, (80*0+62)*2); /* 0000 黑底, 1100 亮红 */
}

/* Ring0 */
void clock_handler(int irq)
{
	ticks++;
	p_current_proc->ticks--;
	
	disp_time();
	
	schedule();
}

void init_clock()
{
	put_irq_handler(IRQ_CLOCK, clock_handler);
	enable_irq(IRQ_CLOCK);
}
