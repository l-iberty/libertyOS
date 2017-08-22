#include "proc.h"

void init_video();	/* lib/io.c */
void _printf(const char* fmt, ...);

int ticks;

void schedule()
{
	if (--p_current_proc->ticks == 0)
	{
		p_current_proc->ticks = p_current_proc->priority;
		if (++p_current_proc >= proc_table + NR_PROCS + NR_TASKS)
		{
			p_current_proc = proc_table;
			init_video();
		}
	}
}

void clock_handler()
{
	ticks++;
	schedule();
}

