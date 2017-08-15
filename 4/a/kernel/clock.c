#include "proc.h"

void print(char* sz);	/* lib/klib.asm */

extern TASK task_table[NR_TASKS];

int ticks;

void schedule()
{
	if (--p_current_proc->ticks == 0)
	{
		p_current_proc->ticks = p_current_proc->priority;
		if (++p_current_proc >= proc_table + NR_TASKS)
		{
			p_current_proc = proc_table;
		}
	}
}

void clock_handler()
{
	ticks++;
	schedule();
}

