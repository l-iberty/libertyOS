#include "proc.h"

void print(char* sz);	/* lib/klib.asm */

extern TASK task_table[NR_TASKS];

int ticks;

void clock_handler()
{
	ticks++;
	p_current_proc++;
	if (p_current_proc >= proc_table + NR_TASKS)
	{
		p_current_proc = proc_table;
	}
}
