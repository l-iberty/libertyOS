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
	uint32_t new_brk = (uint32_t) mm_msg.BRK_ADDR;
	struct proc *pcaller = proc_table + mm_msg.source;
	
	if (new_brk == pcaller->brk) { return 0; /* do nothing */ }
	
	/* seting args for `do_vm_alloc()' and `do_vm_free()'. */
	mm_msg.PD_BASE = pcaller->page_dir_base;
	mm_msg.PT_BASE = mm_msg.PD_BASE + PAGE_SIZE;
	
	if (new_brk > pcaller->brk) /* allocate memory */
	{
		new_brk = ROUND_UP(new_brk, PAGE_SIZE);
		if (new_brk > pcaller->start_brk + PROC_DATA_SEG_MAXSIZE)
		{
			printf("\n#ERROR#{do_brk} addr exceeds process's maximum data size.");
			return -1;
		}
		
		/* seting args for `do_vm_alloc()'. */
		mm_msg.VM_SIZE = new_brk - pcaller->brk;
		mm_msg.VM_ADDR = (void*) pcaller->brk;
		mm_msg.VM_PROTECT = PAGE_READWRITE;
		
		//panic("%.8x, %.8x, %.8x, %.8x, %.8x", mm_msg.VM_SIZE, mm_msg.VM_ADDR, mm_msg.VM_PROTECT, mm_msg.PD_BASE, mm_msg.PT_BASE);
		void *addr = do_vm_alloc();
		if (addr != (void*) pcaller->brk)
		{
			printf("\n#ERROR#{do_brk} no enough memory.");
			return -1;
		}
	}
	else /* free memory */
	{
		new_brk = ROUND_DOWN(new_brk, PAGE_SIZE);
		if (new_brk < pcaller->start_brk)
		{
			printf("\n#ERROR#{do_brk} invalid addr.");
			return -1;
		}
		
		/* seting args for `do_vm_free()'. */
		mm_msg.VM_SIZE = pcaller->brk - new_brk;
		mm_msg.VM_ADDR = (void*) new_brk;
				
		do_vm_free();
	}
	
	pcaller->brk = new_brk;
	return 0;
}

void *do_sbrk()
{
	uint32_t old_brk, new_brk;
	int brk_inc = mm_msg.BRK_INC;
	struct proc *pcaller = proc_table + mm_msg.source;
	
	old_brk = pcaller->brk;
	new_brk = old_brk + ROUND_UP(brk_inc, PAGE_SIZE);
	
	if (brk_inc == 0)
	{
		/* return current program break. */
		return old_brk;
	}
	
	if (brk_inc < 0)
	{
		printf("\n#ERROR#{do_sbrk} invalid increment.");
		return (void*) -1;
	}
	
	if (new_brk - old_brk > PROC_DATA_SEG_MAXSIZE)
	{
		printf("\n#ERROR#{do_sbrk} increment exceeds process's maximum data size.");
		return (void*) -1;
	}
	
	/* seting args for `do_vm_alloc()'. */
	mm_msg.VM_SIZE = new_brk - old_brk;
	mm_msg.VM_ADDR = (void*) old_brk;
	mm_msg.VM_PROTECT = PAGE_READWRITE;
	mm_msg.PD_BASE = pcaller->page_dir_base;
	mm_msg.PT_BASE = mm_msg.PD_BASE + PAGE_SIZE;
	
	void *addr = do_vm_alloc();
	if (addr != (void*) old_brk)
	{
		printf("\n#ERROR#{do_sbrk} no enough memory.");
		return (void*) -1;
	}
	
	pcaller->brk = new_brk;
	return (void*) old_brk;
}

