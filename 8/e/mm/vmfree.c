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
	uint32_t vm_addr	= (uint32_t)mm_msg.VM_ADDR;
	uint32_t page_dir_base	= mm_msg.PD_BASE;
	uint32_t page_tbl_base	= mm_msg.PT_BASE;
	
	assert(vm_size > 0);
	assert(vm_addr % PAGE_SIZE == 0);
	
	uint32_t pde_idx = PDE_INDEX(vm_addr);
	uint32_t pte_idx = PTE_INDEX(vm_addr);
	uint32_t *pde = (uint32_t*)page_dir_base;
	uint32_t *pte = (uint32_t*)page_tbl_base + pde_idx * MAX_PAGE_ITEM;
	
	pm_addr = GET_BASE(pte[pte_idx]); // addr of the first allocated page frame

	/* Clear PTEs */
	for (i = 0; i < nr_pages; i++, pte_idx++)
	{
		pte[pte_idx] = 0;
	}

	/* Clear PDEs */
	uint32_t l = ROUND_DOWN(vm_addr, PAGE_MAPPING_SIZE);
	uint32_t h = ROUND_UP(vm_addr + vm_size, PAGE_MAPPING_SIZE);
	int nr_pde = (h - l) / PAGE_MAPPING_SIZE;

	for (i = 0; i < nr_pde; i++, pde_idx++)
	{
		/* Get page table's base of current PDE */
		pte = (uint32_t*)GET_BASE(pde[pde_idx]);

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
	
	/* `p' 所指向的节点对应于第一个需要被 free 的页框. */

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


