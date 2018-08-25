#include "proc.h"
#include "mm.h"
#include "type.h"
#include "sysconst.h"
#include "stdio.h"
#include "stdlib.h"
#include "global.h"

int alloc_page(int nr_pages, uint32_t *pte, uint32_t *pde_idx, uint32_t *pte_idx);
struct page_list *alloc_frame(int nr_pages);
int check_free_page(uint32_t *pte, uint32_t idx, int n);
int check_free_frame(struct page_list *p, int n);


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
	msg.PD_BASE	= p_current_proc->page_dir_base;
	msg.PT_BASE	= msg.PD_BASE + PAGE_SIZE; /* 页表紧随页目录 */
	
	sendrecv(BOTH, PID_TASK_MM, &msg);
	
	return msg.VM_BASE;
}

void *do_vm_alloc()
{
	int i;
	uint32_t laddr = 0; /* return value */

	int	 nr_pages	= (mm_msg.VM_SIZE + PAGE_SIZE - 1) / PAGE_SIZE;
	uint32_t vm_addr	= (uint32_t)mm_msg.VM_ADDR;
	uint32_t vm_protect	= mm_msg.VM_PROTECT;
	uint32_t page_dir_base	= mm_msg.PD_BASE;
	uint32_t page_tbl_base	= mm_msg.PT_BASE;
	
	uint32_t pde_idx, pte_idx, pde_val, pte_val;
	
	uint32_t *pde = (uint32_t*) page_dir_base;
	uint32_t *pte = (uint32_t*) page_tbl_base;
	
	struct page_list *p;
	
	p = alloc_frame(nr_pages);
	if (p == NULL)
	{
		printf("\n#ERROR#{do_vm_alloc} physical memory not enough.");
		return NULL;
	}
	
	if (vm_addr)
	{
		laddr = ROUND_DOWN(vm_addr, PAGE_SIZE);
		pde_idx = PDE_INDEX(laddr);
		pte_idx = PTE_INDEX(laddr);
		
		/* 映射这些页框所需的 PTE 是否全部空闲? */
		if (!check_free_page(pte, pde_idx * MAX_PAGE_ITEM + pte_idx, nr_pages))
		{
			printf("\n#ERROR#{do_vm_alloc} virual memory not enough.");
			return NULL;
		}
	}
	else
	{
		if (!alloc_page(nr_pages, pte, &pde_idx, &pte_idx))
		{
			printf("\n#ERROR#{do_vm_alloc} virual memory not enough.");
			return NULL;
		}
		laddr = MAKE_LINEAR_ADDR(pde_idx, pte_idx, 0);
	}
	
	
	/* 将 p->BASE 开始的 nr_pages 个页框映射到线性地址 laddr */
	pde_val = page_tbl_base + pde_idx * PAGE_SIZE;
	pte_val = p->BASE;
	pte += pde_idx * MAX_PAGE_ITEM; /* pte 指向 pde[pde_idx] 对应的页表 */
	
	/* 填写 PDE */
	int t = &pte[pte_idx] - (uint32_t*)ROUND_DOWN(&pte[pte_idx], PAGE_SIZE);
	int nr_pde = ((nr_pages + t) * PAGE_SIZE + PAGE_MAPPING_SIZE - 1) / PAGE_MAPPING_SIZE;
	
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
	
	/* 填写 PTE; 更新链表`pf_list'里的信息 */
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

	return (void*)laddr;
}


/* 在页表中找 nr_pages 个空闲的 PTE */
int alloc_page(int nr_pages, uint32_t *pte, uint32_t *pde_idx, uint32_t *pte_idx)
{
	int i, j;
	
	for (i = 0; i < MAX_PAGE_ITEM; i++)
	{
		for (j = 0; j < MAX_PAGE_ITEM; j++)		
		{
			if (pte[i * MAX_PAGE_ITEM + j] == 0) /* PTE 为0, 可以使用 */
			{
				if (check_free_page(pte, i * MAX_PAGE_ITEM + j, nr_pages))
				{
					*pde_idx = i;
					*pte_idx = j;
					return 1; /* succeeded */
				}
			}
		}
	}
	return 0; /* failed */
}

/**
 * 通过 pf_list 找到 nr_pages 个空闲的页框
 *
 * 页框分配算法:
 * 遍历 pf_list, 当找到一个 PAGE_FREE 页框时, 检查从该页框开始是否
 * 有 nr_pages 个连续的 PAGE_FREE 页框可供分配, 如果没有则推进到下
 * 一个 PAGE_FREE 页框, 继续检查.
 */
struct page_list *alloc_frame(int nr_pages)
{
	struct page_list *p = pf_list;

	do
	{
		if (p->TYPE == PAGE_FREE)
		{
			if (check_free_frame(p, nr_pages))
			{
				return p; /* succeeded */
			}
		}
		p = p->NEXT;
	} while (p != pf_list);

	return NULL; /* failed */
}

/* 从 pte[idx] 开始是否有连续 n 个空闲的 PTE ? */
int check_free_page(uint32_t *pte, uint32_t idx, int n)
{
	int i;
	
	for (i = 0; i < n; i++)
	{
		if (pte[idx + i] != 0)
		{
			return 0;
		}
	}
	return 1;
}

/* 从 p->BASE 开始是否有连续 n 个空闲的页框? */
int check_free_frame(struct page_list *p, int n)
{
	int i = 0;

	do
	{
		if (++i == n || p->TYPE != PAGE_FREE) break;
		p = p->NEXT;
	} while (p != pf_list);

	return (i == n && p->TYPE == PAGE_FREE);
}




