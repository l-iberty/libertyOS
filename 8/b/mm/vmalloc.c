#include "proc.h"
#include "mm.h"
#include "type.h"
#include "sysconst.h"
#include "stdio.h"
#include "stdlib.h"
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
 *	- PAGE_READWRITE
 */
void *vm_alloc(void *vm_addr, uint32_t vm_size, uint32_t vm_protect)
{
	struct message msg;

	msg.value	= VM_ALLOC;
	msg.VM_ADDR	= vm_addr;
	msg.VM_SIZE	= vm_size;
	msg.VM_PROTECT	= vm_protect;
	
	sendrecv(BOTH, PID_TASK_MM, &msg);
	
	if (msg.VM_BASE == NULL)
	{
		panic("%d->%d %.8x", msg.source, msg.dest, msg.VM_ADDR);
	}

	return msg.VM_BASE;
}

void *do_vm_alloc()
{
	int i;
	uint32_t laddr = 0; /* return value */

	int nr_pages = (mm_msg.VM_SIZE + PAGE_SIZE - 1) / PAGE_SIZE;
	uint32_t vm_addr = (uint32_t)mm_msg.VM_ADDR;
	uint32_t vm_protect = mm_msg.VM_PROTECT;
	
	struct page_list *current, *p;
	uint32_t pde_idx, pte_idx, pde_val, pte_val;
	uint32_t *pde = (uint32_t*)mi->page_dir_base;
	uint32_t *pte = (uint32_t*)mi->page_tbl_base;
	
	current = pf_list;
	p = NULL;

	do
	{
		if (vm_addr >= current->BASE &&
		    vm_addr < ((struct page_list*)current->NEXT)->BASE)
		{
			/* 检查: 从 current->BASE 开始是否有连续可用的 nr_pages 个页框可共分配? */
			for (p = current, i = 0; i < nr_pages; i++, p = p->NEXT)
			{
				if (p->TYPE != PAGE_FREE) break;
			}
			
			if (i >= nr_pages) /* current->BASE is available */
			{
				p = current;
				break;
			}
			else /* current->BASE is not available, try next */
			{
				p = NULL;
				vm_addr = ((struct page_list*)current->NEXT)->BASE;
			}
		}
		current = current->NEXT;
	} while (current != pf_list);
	
	if (p != NULL)
	{
		printf("\n{do_vm_alloc} actual base: 0x%.8x", p->BASE);
		laddr = p->BASE;
		if (p->TYPE == PAGE_FREE)
		{
			pde_idx = PDE_INDEX(p->BASE);
			pte_idx = PTE_INDEX(p->BASE);
			pde_val = mi->page_tbl_base + pde_idx * PAGE_SIZE;
			pte_val = p->BASE;

			pte += pde_idx * MAX_PAGE_ITEM;
			
			/* 填写 PDE */
			int t = &pte[pte_idx] - (uint32_t*)ROUND_DOWN(&pte[pte_idx], PAGE_SIZE);
			int nr_pde = ((nr_pages + t) * PAGE_SIZE + 
				     PAGE_MAPPING_SIZE - 1) / PAGE_MAPPING_SIZE;
			
			for (i = 0; i < nr_pde; i++, pde_idx++, pde_val += PAGE_SIZE)
			{
				if (vm_protect & PAGE_READ)
				{
					pde_val |= (PG_P | PG_RWR | PG_USU);
				}
				if (vm_protect & PAGE_READWRITE)
				{
					pde_val |= (PG_P | PG_RWW | PG_USU);
				}
				if (pde[pde_idx] == 0)
				{
					pde[pde_idx] = pde_val;
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
				if (vm_protect & PAGE_READWRITE)
				{
					pte[pte_idx] = pte_val | (PG_P | PG_RWW | PG_USU);
					p->PROTECT |= PAGE_READWRITE;
				}
				p->REF++;
				p->TYPE = PAGE_USED;
				p = p->NEXT;
			}
			reload_cr3(getcr3());
		}
	}
	
	return (void*)laddr;
}

