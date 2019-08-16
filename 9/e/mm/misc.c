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
 * 遍历 pf_list, 如果分配的是可读写页框, 则:
 * - 当找到一个 PAGE_FREE 页框时, 检查从该页框开始是否
 *   有 nr_pages 个连续的 PAGE_FREE 页框可供分配, 如果
 *   没有则推进到下一个 PAGE_FREE 页框, 继续检查.
 * 如果分配的只读页框, 则:
 * - 找到第一个非 PAGE_RESERVED 页框即可.
 */
struct page_list *alloc_frame(int nr_pages, uint32_t protect)
{
	struct page_list *p = pf_list;
	struct page_list *next = NULL;

	do
	{
		if (protect & PAGE_READWRITE)
		{
			if (p->TYPE == PAGE_FREE)
			{
				if (check_free_frame(p, nr_pages, &next))
				{
					return p; /* succeeded */
				}
				else
				{
					p = next;
					continue;
				}
			}
		}
		else
		{
			if (p->TYPE != PAGE_RESERVED) { return p; }
		}
		p = p->NEXT;
	} while (p != pf_list);

	return NULL; /* failed */
}

struct page_list *find_pf_list_item(uint32_t base)
{
	struct page_list *p = pf_list;
	do
	{
		if (p->BASE == base) { return p; }
		p = p->NEXT;
	} while (p != pf_list);
	
	return NULL;
}

/* 在页表中找 nr_pages 个空闲的 PTE */
int alloc_page(int nr_pages, uint32_t *pte, uint32_t *pde_idx, uint32_t *pte_idx)
{
	int i, j;
	
	for (i = 0; i < MAX_PAGE_ITEM; i++)
	{
		for (j = 0; j < MAX_PAGE_ITEM; j++)		
		{
			if (pte[i * MAX_PAGE_ITEM + j] == 0) /* PTE为0, 可以使用 */
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
	for (int i = 0; i < n; i++)
	{
		if (pte[idx + i] != 0) { return 0; }
	}
	return 1;
}

/* 从 p->BASE 开始是否有连续 n 个空闲的页框? */
int check_free_frame(struct page_list *p, int n, struct page_list **next)
{
	int i = 0;
	
	do
	{
		if (++i == n || p->TYPE != PAGE_FREE) { break; }
		p = p->NEXT;
	} while (p != pf_list);

	if (i == n && p->TYPE == PAGE_FREE) return 1;

	*next = p;
	return 0;
}

/**
 * 对 pde 指向的页目录中的 PDE 进行重定位
 */
void relocate_pde(uint32_t *pde)
{
	uint32_t offset = (uint32_t)pde - mi->page_dir_base;
	for (int i = 0; i < MAX_PAGE_ITEM; i++)
	{
		if (pde[i] != 0) { pde[i] += offset; }
	}
}

/**
 * 从 p 指向的页框开始, 将 nr_pages 个页框映射到虚拟地址 vm_base,
 * page_dir_base 是本次操作使用的页目录物理基地址.
 */
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
		/* 只有以下两种情形需要复写 PDE */
		if ((pde[pde_idx] == 0) || /* 分配页面 */
		    (pde[pde_idx] != 0) && !(pde[pde_idx] & PG_RWW)) /* 修改访问权限为"可读写" */
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

/**
 * 解除虚页到页框的映射.
 * page_dir_base 是本次操作使用的页目录物理基地址
 */
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
	struct page_list *p = find_pf_list_item(pm_base);
	
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
	

