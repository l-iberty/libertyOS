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

	sendrecv(BOTH, PID_TASK_MM, &msg);
}

void do_vm_free()
{
	int i, j;
	uint32_t pm_addr;

	uint32_t vm_size = mm_msg.VM_SIZE;
	uint32_t vm_addr = (uint32_t)mm_msg.VM_ADDR;
	int nr_pages = (vm_size + PAGE_SIZE - 1) / PAGE_SIZE;

	assert(vm_size > 0);
	assert(vm_addr % PAGE_SIZE == 0);
	
	uint32_t pde_idx = PDE_INDEX(vm_addr);
	uint32_t pte_idx = PTE_INDEX(vm_addr);
	uint32_t *pde = (uint32_t*)mi->page_dir_base;
	uint32_t *pte = (uint32_t*)mi->page_tbl_base + pde_idx * MAX_PAGE_ITEM;

	/* Get base of the first page frame */
	pm_addr = GET_BASE(pte[pte_idx]);

	/* Clear PTEs */
	for (i = 0; i < nr_pages; i++, pte_idx++)
	{
		pte[pte_idx] = 0;
	}

	/* Clear PDEs */
	uint32_t l = ROUND_DOWN(vm_addr, PAGE_MAPPING_SIZE);
	uint32_t h = ROUND_UP(vm_addr + vm_size, PAGE_MAPPING_SIZE);
	int nr_pde = (h - l) / PAGE_MAPPING_SIZE;
	int flag;

	for (i = 0; i < nr_pde; i++, pde_idx++)
	{
		/* Get page table's base of current PDE */
		pte = (uint32_t*)GET_BASE(pde[pde_idx]);

		/* If all PTEs in this page table have been cleared, then clear current PDE */
		for (flag = 0, j = 0; j < MAX_PAGE_ITEM; j++)
		{
			if (pte[j] != 0)
			{
				flag = 1;
				break;
			}
		}
		if (flag == 0)
		{
			pde[pde_idx] = 0;
		}
	}

	/* Mark occupied page frames as `PAGE_FREE' */
	struct page_list *current, *p;
	current = pf_list;
	p = NULL;
	do
	{
		if (current->BASE == pm_addr)
		{
			p = current;
			break;
		}
		current = current->NEXT;
	} while (current != pf_list);
	assert(p != NULL);

	for (i = 0; i < nr_pages; i++, p = p->NEXT)
	{
		p->REF--;
		if (p->REF == 0)
		{
			p->TYPE = PAGE_FREE;
			p->PROTECT = 0;
		}
	}
}


