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
	
	/* 是否有空闲页框可供分配? */
	/**
	 * 页框分配算法:
	 * 遍历 pf_list, 当找到一个 PAGE_FREE 页框时, 检查从该页框开始是否
	 * 有 nr_pages 个连续的 PAGE_FREE 页框可供分配, 如果没有则推进到下
	 * 一个 PAGE_FREE 页框, 继续检查.
	 */
	int nr_free_frame = 0;
	current = pf_list;
	p = NULL;
	do
	{
		if (current->TYPE == PAGE_FREE)
		{
			/* 从 current->BASE 开始是否有连续可用的 nr_pages 个页框可供分配? */
			nr_free_frame = 0;
			p = current;
			do
			{
				if (p->TYPE == PAGE_FREE)
				{
					if (++nr_free_frame == nr_pages) break;
				}
				else
				{
					break;
				}
				p = p->NEXT;
			} while (p != pf_list);

			if (nr_free_frame == nr_pages)
			{
				p = current;
				break;
			}
		}
		current = current->NEXT;
	} while (current != pf_list);
	
	if (nr_free_frame < nr_pages)
	{
		printf("\n#ERROR#{do_vm_alloc} physical memory not enough.");
		return NULL;
	}
	
	/* Get linear address */
	laddr = ROUND_DOWN(vm_addr, PAGE_SIZE);
	
	/* 映射这些页框所需的 PTE 是否全部空闲? */
	int flag = 0;
	pde_idx = PDE_INDEX(laddr);
	pte_idx = PTE_INDEX(laddr);
	pte += pde_idx * MAX_PAGE_ITEM; /* 令 pte 指向 pde_idx 对应的页表 */
	for (i = 0; i < nr_pages; i++, pte_idx++)
	{
		if (pte[pte_idx] != 0)
		{
			flag = 1;
			break;
		}
	}
	if (flag == 1)
	{
		/* 所需 PTE 被占用, 无法分配虚拟内存 */
		printf("\n#ERROR#{do_vm_alloc} virual memory not enough.");
		return NULL;
	}

	/* 将 p->BASE 开始的 nr_pages 个页框映射到线性地址 laddr */
	pde_idx = PDE_INDEX(laddr);
	pte_idx = PTE_INDEX(laddr);
	pde_val = mi->page_tbl_base + pde_idx * PAGE_SIZE;
	pte_val = p->BASE; /* physical address */
	
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
		p->TYPE = PAGE_MAPPED;
		p = p->NEXT;
	}
	reload_cr3(getcr3());

	return (void*)laddr;
}

