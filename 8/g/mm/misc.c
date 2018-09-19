#include "proc.h"
#include "mm.h"
#include "sysconst.h"
#include "global.h"
#include "stdio.h"
#include "stdlib.h"
#include "string.h"

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

struct page_list *find_item(uint32_t base)
{
	struct page_list *p = pf_list;
	do
	{
		if (p->BASE == base) break;
		p = p->NEXT;
	} while (p != pf_list);
	
	if (p == pf_list)
		return NULL;
	else
		return p;
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

void map_frame(struct page_list *p,
	       uint32_t page_dir_base,
	       uint32_t vm_base,
	       uint32_t nr_pages,
	       uint32_t vm_protect)
{
	int i;
	uint32_t page_tbl_base;
	uint32_t pde_idx, pte_idx;
	uint32_t pde_val, pte_val;
	uint32_t *pde, *pte;
	
	page_tbl_base = page_dir_base + PAGE_SIZE;
	pde_idx = PDE_INDEX(vm_base);
	pte_idx = PTE_INDEX(vm_base);
	pde_val = page_tbl_base + pde_idx * PAGE_SIZE;
	pte_val = p->BASE;
	pde = (uint32_t*) page_dir_base;
	pte = (uint32_t*) (page_tbl_base + pde_idx * PAGE_SIZE);

	/* 填写 PDE */
	int off = &pte[pte_idx] - (uint32_t*) ROUND_DOWN(&pte[pte_idx], PAGE_SIZE);
	int nr_pde = ((nr_pages + off) * PAGE_SIZE + PAGE_MAPPING_SIZE - 1) / PAGE_MAPPING_SIZE;
	
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
}

void unmap_frame(uint32_t page_dir_base,
	         uint32_t vm_base,
	         uint32_t vm_size)
{
	int i, j, nr_pages;
	uint32_t pde_idx, pte_idx, pm_base;
	uint32_t *pde, *pte;
	
	nr_pages = (vm_size + PAGE_SIZE - 1) / PAGE_SIZE;
	pde_idx  = PDE_INDEX(vm_base);
	pte_idx  = PTE_INDEX(vm_base);
	pde = (uint32_t*) page_dir_base;
	pte = (uint32_t*) (page_dir_base + PAGE_SIZE + pde_idx * PAGE_SIZE);
	pm_base = GET_BASE(pte[pte_idx]);
	
	/* Clear PTEs */
	for (i = 0; i < nr_pages; i++, pte_idx++)
	{
		pte[pte_idx] = 0;
	}
	
	/* Clear PDEs */
	uint32_t l = ROUND_DOWN(vm_base, PAGE_MAPPING_SIZE);
	uint32_t h = ROUND_UP(vm_base + vm_size, PAGE_MAPPING_SIZE);
	int nr_pde = (h - l) / PAGE_MAPPING_SIZE;

	for (i = 0; i < nr_pde; i++, pde_idx++)
	{
		/* Get page table's base of current PDE */
		pte = (uint32_t*) GET_BASE(pde[pde_idx]);

		/* If all PTEs in this page table have been cleared, then clear current PDE */
		for (j = 0; j < MAX_PAGE_ITEM; j++)
		{
			if (pte[j] != 0) break;
		}
		if (j == MAX_PAGE_ITEM)
		{
			pde[pde_idx] = 0;
		}
	}
	
	/* Free occupied page frames */
	struct page_list *p = find_item(pm_base);
	
	/* p 所指向的节点对应于第一个需要被 free 的页框. */
	for (i = 0; i < nr_pages; i++, p = p->NEXT)
	{
		if (p->REF == 0)
		{
			printf("\n#ERROR#{do_vm_free} cannot free page frame 0x%.8x.", p->BASE);
			return;
		}

		p->REF--;
		if (p->REF == 0)
		{
			p->TYPE	   = PAGE_FREE;
			p->PROTECT = 0;
		}
	}
}
	

