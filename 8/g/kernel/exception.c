#include "exception.h"
#include "global.h"
#include "proc.h"
#include "stdio.h"
#include "stdlib.h"
#include "mm.h"

EXCEP_HANDLER exception_table[NR_EXCEPTION];

void put_excep_handler(int vecno, EXCEP_HANDLER handler)
{
	exception_table[vecno] = handler;
}

/* #UD */
void do_invalid_opcode(int vecno, uint32_t err_code, uint32_t eip, uint16_t cs, uint32_t eflags)
{
	panic("-------#UD-------\nPID: %d\nERR_CODE: 0x%.8x\nCS: 0x%.4x EIP: 0x%.8x EFLAGS: 0x%.8x",
	      p_current_proc->pid, err_code, cs, eip, eflags);

	disable_int();
	while (1) {}
}


/* #GP */
void do_general_protection(int vecno, uint32_t err_code, uint32_t eip, uint16_t cs, uint32_t eflags)
{
	panic("-------#GP-------\nPID: %d\nERR_CODE: 0x%.8x\nCS: 0x%.4x EIP: 0x%.8x EFLAGS: 0x%.8x",
	      p_current_proc->pid, err_code, cs, eip, eflags);
	
	disable_int();
	while (1) {}
}

/* #PF */
void do_page_fault(int vecno, uint32_t err_code, uint32_t eip, uint16_t cs, uint32_t eflags)
{
	uint32_t laddr = getcr2();
	uint32_t page_dir_base = p_current_proc->page_dir_base;
	uint32_t page_tbl_base = page_dir_base + PAGE_SIZE; /* 页表紧随页目录 */
	
	uint32_t *pde = (uint32_t*) page_dir_base;
	uint32_t *pte = (uint32_t*) page_tbl_base;
	uint32_t pde_idx = PDE_INDEX(laddr);
	uint32_t pte_idx = pde_idx * MAX_PAGE_ITEM + PTE_INDEX(laddr);
	
	struct page_list *p;
	
	if (PG_ERR_CODE_P(err_code) == 0)
	{
		if (pte[pte_idx] == 0) /* CASE-1: PTE全为0, 目标页帧不存在, 给未被映射的地址映射上物理页 */
		{
			/* Search for a free page frame */
			p = alloc_frame(1);
			if (p == NULL)
			{
				/* 没有空闲页框可供分配, 需要执行页面置换 */
			}
			else
			{
				/* 不论是读操作还是写操作引发异常, 都映射一个可读写的页框 */
				map_frame(p, page_dir_base, GET_BASE(laddr), 1, PAGE_READWRITE);
			}
		}
		else /* CASE-2: PTE非空, 相应的物理页帧不在内存中, 需要执行页面置换 */
		{
		
		}
	}
	else /* CASE-3: 访问权限非法（只考虑读写权限） */
	{
		if (PG_ERR_CODE_WR(err_code) == 1)
		{
			panic("LA = 0x%.8x-The access causing the fault was a write.", getcr2());
		}
		else
		{
			panic("LA = 0x%.8x-The access causing the fault was a read.", getcr2());
		}
		disable_int();
		while (1) {}
	}
}


