#include "proc.h"
#include "mm.h"
#include "type.h"
#include "sysconst.h"
#include "stdio.h"
#include "stdlib.h"
#include "global.h"

/**
 * `brk()' and `sbrk()' change the location of the "program break",
 * which defines the end of the process's data segment.
 */

/**
 * @brief
 * Set the end of the data segment to the value specified by `addr',
 * when that value is resonable, the system has enough memory, and
 * the process does not exceed its maximum data size.
 * @return
 * On success, return 0. On error, (-1).
 */
int brk(void *addr)
{
	struct message msg;
	
	msg.value =	BRK;
	msg.BRK_ADDR = 	addr;
	
	sendrecv(BOTH, PID_TASK_MM, &msg);
	
	return msg.RETVAL;
}

/**
 * @brief
 * Increment the program's data space by `increment' bytes. Calling
 * `sbrk()' with an increment of 0 can be used to find the current
 * location of the program break.
 * @return
 * On success, return the previous program break. On error, (void*)(-1).
 */
void *sbrk(int increment)
{
	struct message msg;
	
	msg.value =	SBRK;	
	msg.BRK_INC =	increment;
	
	sendrecv(BOTH, PID_TASK_MM, &msg);
	
	return msg.NEW_BRK;
}

int do_brk()
{
	struct proc *pcaller = proc_table + mm_msg.source;
	uint32_t new_brk = (uint32_t) mm_msg.BRK_ADDR;
	
	if (new_brk == pcaller->mm.brk) { return 0; /* do nothing */ }
	
	if ((int)(new_brk - pcaller->mm.start_brk) > PROC_DATA_SEG_MAXSIZE)
	{
		printf("\n#ERROR#{do_brk} addr exceeds process's maximum data size.");
		return -1;
	}
	
	if ((int)new_brk < (int)pcaller->mm.start_brk)
	{
		printf("\n#ERROR#{do_brk} invalid addr.");
		return -1;
	}

	/* setting args for `do_vm_alloc()' and `do_vm_free()'. */
	mm_msg.PD_BASE = pcaller->page_dir_base;
	mm_msg.PT_BASE = mm_msg.PD_BASE + PAGE_SIZE;
	
	do
	{
		if (new_brk > pcaller->mm.brk) /* allocate memory */
		{
			//printf("\n$alloc");
			/* 从 start_brk 到 mlimit 之间的页面是已经映射好的, 如果 new_brk 在此范围内则无需映射新的页面 */
			if (new_brk <= pcaller->mm.mlimit) { /*printf("\n$no need to alloc");*/ break; }
		
			/* setting args for `do_vm_alloc()'. */
			mm_msg.VM_SIZE = new_brk - pcaller->mm.mlimit;
			mm_msg.VM_ADDR = (void*) pcaller->mm.mlimit;
			mm_msg.VM_PROTECT = PAGE_READWRITE;
		
			void *addr = do_vm_alloc();
			if (addr != (void*) pcaller->mm.mlimit)
			{
				printf("\n#ERROR#{do_brk} no enough memory.");
				return -1;
			}
			/* update caller's mlimit. */
			pcaller->mm.mlimit = ROUND_UP(new_brk, PAGE_SIZE);
			//printf("\n$mlimit = 0x%.8x", pcaller->mm.mlimit);
		}
		else /* free memory */
		{
			//printf("\n$free");
			uint32_t vm_size = ROUND_DOWN((pcaller->mm.mlimit - new_brk), PAGE_SIZE);
			if (vm_size == 0) { /*printf("\n$no need to free");*/ break; } /* no virtual pages needed to free. */
			
			/* setting args for `do_vm_free()'. */
			mm_msg.VM_SIZE = vm_size;
			mm_msg.VM_ADDR = (void*) ROUND_UP(new_brk, PAGE_SIZE);

			do_vm_free();
			/* update caller's mlimit. */
			pcaller->mm.mlimit = ROUND_UP(new_brk, PAGE_SIZE);
			//printf("\n$mlimit = 0x%.8x", pcaller->mm.mlimit);
		}
	} while (0);
	
	pcaller->mm.brk = new_brk;
	return 0;
}

void *do_sbrk()
{
	struct proc *pcaller = proc_table + mm_msg.source;
	mm_msg.BRK_ADDR = (void*) pcaller->mm.brk + mm_msg.BRK_INC; /* setting arg for `do_brk()'. */
	
	if (do_brk() == -1) return (void*) -1;
	
	return (void*) pcaller->mm.brk;
}

