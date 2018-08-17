#include "proc.h"
#include "mm.h"
#include "exe.h"
#include "sysconst.h"
#include "global.h"
#include "stdio.h"
#include "stdlib.h"
#include "string.h"

struct message exe_msg;

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


uint32_t alloc_mem(uint32_t pid, size_t size)
{
	assert(pid >= (NR_NATIVE_PROCS + NR_TASKS));

	uint32_t base = PROCS_BASE +
		   (pid - (NR_NATIVE_PROCS + NR_TASKS)) * PROC_IMAGE_SIZE;
	
	return (uint32_t)vm_alloc((void*)base, size, PAGE_READWRITE);
}
