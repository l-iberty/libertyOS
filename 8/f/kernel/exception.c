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
	uint32_t pg_attr; /* PDE & PTE 的属性 */
	
	struct page_list *p;
	
	if (PG_ERR_CODE_P(err_code) == 0)
	{
		if (pte[pte_idx] == 0) /* CASE-1: PTE全为0, 目标页帧不存在, 给未被映射的地址映射上物理页 */
		{
			/* Search for a free page frame */
			p = pf_list;
			do
			{
				if (p->TYPE == PAGE_FREE) break;
				p = p->NEXT;
			} while (p != pf_list);
			
			if (p == pf_list) /* 没有空闲页框可供分配, 需要执行页面置换 */
			{
				
			}
			else
			{
				p->TYPE = PAGE_MAPPED;
#if 0
/* 设置PDE和PTE属性位时, 根据访问情况确定 */

				if (PG_ERR_CODE_WR(err_code) == 1)
				{
					/* The access causing the fault was a write. */
					pg_attr = (PG_P | PG_RWW | PG_USU);
					p->PROTECT = PAGE_READWRITE;
				}
				else
				{
					/* The access causing the fault was a read. */
					pg_attr = (PG_P | PG_RWR | PG_USU);
					p->PROTECT = PAGE_READ;
				}
				
				pte[pte_idx] = p->BASE | pg_attr;
				if (pde[pde_idx] == 0)
				{
					pde[pde_idx] = (page_tbl_base + pde_idx * PAGE_SIZE) | pg_attr;
				}
#else
/* 设置页面访问权限为"可读写" */
				p->PROTECT = PAGE_READWRITE;
				pg_attr = (PG_P | PG_RWW | PG_USU);
				
				pte[pte_idx] = p->BASE | pg_attr;
				if (pde[pde_idx] == 0)
				{
					pde[pde_idx] = (page_tbl_base + pde_idx * PAGE_SIZE) | pg_attr;
				}
#endif				
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


