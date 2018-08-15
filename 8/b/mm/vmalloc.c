#include "proc.h"
#include "mm.h"
#include "type.h"
#include "sysconst.h"
#include "stdio.h"
#include "global.h"


/**
 * @param vm_addr
 *	The starting address of the region to allocate. If the memory is free, the specified
 *	address is rounded down to the nearest multiple of the allocation granularity. If the
 *	memory is already reserved, or this parameter is NULL, the system determines where to
 *	allocate the region.
 * @param vm_size
 *	The size of the region, in bytes. This value is rounded up to the nearest multiple
 *	of the allocation granularity.
 * @param vm_protect
 *	The type of memory allocation. This parameter must contain one of the following values:
 *	- PAGE_READ
 *	- PAGE_WRITE
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
	int i;
	u32 laddr = 0; /* return value */
	
	int nr_pages = (mm_msg.VM_SIZE + PAGE_SIZE - 1) / PAGE_SIZE;
	u32 vm_addr = (u32)mm_msg.VM_ADDR;
	u32 vm_protect = mm_msg.VM_PROTECT;
	
	PAGE_FRAME *current, *p;
	u32 pde_idx, pte_idx, pde_val, pte_val;
	u32 *pde = (u32*)mi->page_dir_base;
	u32 *pte = (u32*)mi->page_tbl_base;
	
	current = pf_list;
	p = NULL;

	while (current->NEXT != pf_list)
	{
		if (vm_addr >= current->BASE &&
		    vm_addr < ((PAGE_FRAME*)current->NEXT)->BASE)
		{
			p = current;
			break;
		}
		current = current->NEXT;
	}
	if (p != NULL)
	{
		printf("\n{do_vm_alloc} actual base: 0x%.8x, type: ", p->BASE);
		laddr = p->BASE;
		if (p->TYPE == PAGE_FREE)
		{
			pde_idx = PDE_INDEX(p->BASE);
			pte_idx = PTE_INDEX(p->BASE);
			pde_val = mi->page_tbl_base + pde_idx * PAGE_SIZE;
			pte_val = p->BASE;

			pte += pde_idx * MAX_PAGE_ITEM;
			
			/* 填写 PDE */
			int t = &pte[pte_idx] - (u32*)ROUND_DOWN(&pte[pte_idx], PAGE_SIZE);
			int nr_pde = ((nr_pages + t) * PAGE_SIZE + 
				     PAGE_MAPPING_SIZE - 1) / PAGE_MAPPING_SIZE;
			
			for (i = 0; i < nr_pde; i++, pde_idx++, pde_val += PAGE_SIZE)
			{
				if (vm_protect & PAGE_READ)
				{
					pde[pde_idx] = pde_val | (PG_P | PG_RWR | PG_USU);
				}
				if (vm_protect & PAGE_WRITE)
				{
					pde[pde_idx] = pde_val | (PG_P | PG_RWW | PG_USU);
				}
			}
			
			/* 填写 PTE; 更新双向循环链表里的信息 */
			for (i = 0; i < nr_pages; i++, pte_idx++, pte_val += PAGE_SIZE)
			{
				if (vm_protect & PAGE_READ)
				{
					pte[pte_idx] = pte_val | (PG_P | PG_RWR | PG_USU);
					p->PROTECT |= PAGE_READ;
				}
				if (vm_protect & PAGE_WRITE)
				{
					pte[pte_idx] = pte_val | (PG_P | PG_RWW | PG_USU);
					p->PROTECT |= PAGE_WRITE;
				}
				p->REF++;
				p->TYPE = PAGE_USED;
				p = p->NEXT;
			}
			reload_cr3(getcr3());
		}
		else if (p->TYPE == PAGE_RESERVED)
		{
			printf("RESERVED ");
		}
		else if (p->TYPE == PAGE_USED)
		{
			printf("USED ");
		}
	}

	return (void*)laddr;
}

