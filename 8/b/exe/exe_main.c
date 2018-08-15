#include "proc.h"
#include "mm.h"
#include "exe.h"
#include "sysconst.h"
#include "global.h"
#include "stdio.h"
#include "stdlib.h"
#include "string.h"


void TaskEXE()
{
	printf("\n-----TaskEXE-----");
	
	for (;;) 
	{
		sendrecv(RECEIVE, ANY, &exe_msg);
		
		switch (exe_msg.value)
		{
			case FORK:
			{
				exe_msg.RETVAL = do_fork();
				break;
			}
			default:
			{
				printf("\nEXE-{unknown message value}");
				dump_msg(&exe_msg);
			}
		}

		sendrecv(SEND, exe_msg.source, &exe_msg);
	}
}


u32 alloc_mem(u32 pid, size_t size)
{
	assert(pid >= (NR_NATIVE_PROCS + NR_TASKS));

	u32 base = PROCS_BASE +
		   (pid - (NR_NATIVE_PROCS + NR_TASKS)) * PROC_IMAGE_SIZE;
	
	return (u32)vm_alloc((void*)base, size, PAGE_READ | PAGE_WRITE);
}
