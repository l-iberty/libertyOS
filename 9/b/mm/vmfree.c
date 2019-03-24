#include "proc.h"
#include "mm.h"
#include "type.h"
#include "sysconst.h"
#include "stdio.h"
#include "stdlib.h"
#include "global.h"


void vm_free(void *vm_addr, uint32_t vm_size)
{
	struct message msg;

	msg.value   = VM_FREE;
	msg.VM_ADDR = vm_addr;
	msg.VM_SIZE = vm_size;
	msg.PD_BASE = p_current_proc->page_dir_base;
	msg.PT_BASE = msg.PD_BASE + PAGE_SIZE; /* 页表紧随页目录 */

	sendrecv(BOTH, PID_TASK_MM, &msg);
}

void do_vm_free()
{
	int i, j;
	uint32_t pm_addr;

	int      nr_pages	= (mm_msg.VM_SIZE + PAGE_SIZE - 1) / PAGE_SIZE;
	uint32_t vm_size	= mm_msg.VM_SIZE;
	uint32_t vm_addr	= (uint32_t) mm_msg.VM_ADDR;
	uint32_t page_dir_base	= mm_msg.PD_BASE;
	uint32_t page_tbl_base	= mm_msg.PT_BASE;
	
	assert(vm_size > 0);
	assert(vm_addr % PAGE_SIZE == 0);
	
	unmap_frame(page_dir_base, vm_addr, vm_size);
}


