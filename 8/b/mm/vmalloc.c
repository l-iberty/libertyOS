#include "proc.h"
#include "mm.h"
#include "type.h"
#include "sysconst.h"
#include "stdio.h"
#include "global.h"


/**
 * @param vm_addr
 *	The starting address of the region to allocate. If the memory is free, the specified
 *	address is rounded up to the next page boundary. If the memory is already reserved,
 *	the address is rounded down to the next page boundary. If this parameter is NULL,
 *	the system determines where to allocate the region.
 * @param vm_size
 *	The size of the region, in bytes. This value is rounded up to the nearest multiple
 *	of the allocation granularity.
 * @param vm_protect
 *	The type of memory allocation. This parameter must contain one of the following values:
 *	- PAGE_READ
 *	- PAGE_WRITE
 *	- PAGE_EXECUTE
 */
void *vm_alloc(void *vm_addr, u32 vm_size, u32 vm_protect)
{
	MESSAGE msg;
	
	msg.value	= VM_ALLOC;
	msg.VM_ADDR	= vm_addr;
	msg.VM_SIZE	= vm_size;
	msg.VM_PROTECT	= vm_protect;
	
	sendrecv(BOTH, PID_TASK_MM, &msg);
	
	return msg.VM_BASE;
}

void *do_vm_alloc()
{
	u32 vm_addr = (u32)mm_msg.VM_ADDR;
	u32 vm_size = mm_msg.VM_SIZE;
	u32 vm_protect = mm_msg.VM_PROTECT;
	printf("%.8x %.8x %.8x",vm_addr,vm_size,vm_protect);
	
	return NULL;
}

