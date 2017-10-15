#include "proc.h"
#include "fs.h"
#include "mm.h"
#include "sysconst.h"
#include "global.h"
#include "stdio.h"
#include "stdlib.h"
#include "string.h"

void Task_mm()
{
	printf("\n-----Task_mm-----");
	
	for (;;) {
		sendrecv(RECEIVE, ANY, &mm_msg);
		
		switch (mm_msg.value)
		{
			case FORK:
			{
				mm_msg.RETVAL = do_fork();
				break;
			}
			default:
			{
				printf("\nMM-{unknown message value}");
				dump_msg(&hd_msg);
			}
		}
		
		sendrecv(SEND, mm_msg.source, &mm_msg);
	}
}

u32 alloc_mem(u32 pid, size_t size)
{
	assert(pid >= (NR_NATIVE_PROCS + NR_TASKS));

	int chunk = (size + PROC_IMAGE_SIZE - 1) / PROC_IMAGE_SIZE;
	u32 base = PROCS_BASE +
			(pid - (NR_NATIVE_PROCS + NR_TASKS)) * PROC_IMAGE_SIZE;
	
	return base;
}

