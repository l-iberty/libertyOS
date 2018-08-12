#include "proc.h"
#include "fs.h"
#include "mm.h"
#include "sysconst.h"
#include "global.h"
#include "stdio.h"
#include "stdlib.h"
#include "string.h"

void TaskMM()
{
	printf("\n-----TaskMM-----");
	
	init_mm();
	
	for (;;) 
	{
		sendrecv(RECEIVE, ANY, &mm_msg);
		
		switch (mm_msg.value)
		{
			case FORK:
			{
				mm_msg.RETVAL = do_fork();
				break;
			}
			case VM_ALLOC:
			{
				mm_msg.VM_BASE = do_vm_alloc();
				break;
			}
			default:
			{
				printf("\nMM-{unknown message value}");
				dump_msg(&hd_msg);
			}
		}
		
		sendrecv(SEND, mm_msg.source, &mm_msg);
	}
}


/**
 * 分段时使用的是平坦模式, 代码段和数据段的基址为零, 因此虚拟地址 = 线性地址;
 * loader 建立的页表又使得 线性地址 = 物理地址.
 * 从而, 虚拟地址 = 线性地址 = 物理地址
 */
void init_mm()
{
	MEMINFO *p_mi = (MEMINFO*)(MEM_INFO_VA_BASE);
	
	printf("\n{MM}    Available Physical Memory Base: 0x%.8x", p_mi->avail_pm_base);
	printf("\n{MM}    Available Physical Memory Size: 0x%.8x", p_mi->avail_pm_size);
	printf("\n{MM}    Page Directory Base: 0x%.8x", p_mi->page_dir_base);
	printf("\n{MM}    Page Table Base: 0x%.8x", p_mi->page_tbl_base);
	printf("\n{MM}    Number of PDE: 0x%.8x", p_mi->nr_pde);
	
	int reserved_pages = p_mi->nr_pde + 1; /* 二级页表所占虚页(物理页)数目. 包括 1 个页目录和 nr_pde 个页表 */
	int avail_pages = p_mi->avail_pm_size / PAGE_SIZE;
	
	assert(p_mi->page_dir_base == p_mi->avail_pm_base);
	
	/**
	 * 建立双向循环链表, 以页为单位管理从 avail_pm_base 开始, 大小 avail_pm_size 的物理内存.
	 * 由于页目录和页表从 avail_pm_base 开始存放, 因此将其占据的页框 type 设为 PAGE_RESERVED,
	 * 被OS保留; 其余设为 PAGE_FREE, 以便后续分配.
	 *
	 * 链表紧随页表存放, 所占内存大小 sizeof(PAGE_FRAME) * avail_pages, 这部分内存也要保留.
	 */
	
	PAGE_FRAME *head, *current, *next;
	head = (PAGE_FRAME*)(p_mi->avail_pm_base + reserved_pages * PAGE_SIZE);
	current = head;
	reserved_pages += ROUND_UP((sizeof(PAGE_FRAME) * avail_pages), PAGE_SIZE) / PAGE_SIZE;
	
	int i;
	for (i = 0; i < avail_pages; i++, current = next)
	{
		if (i < reserved_pages)
		{
			current->REF = 1;
			current->TYPE = PAGE_RESERVED;
			current->PROTECT = PAGE_READ | PAGE_WRITE;
		}
		else
		{
			current->REF = 0;
			current->TYPE = PAGE_FREE;
			current->PROTECT = 0;
		}
		current->BASE = p_mi->avail_pm_base + i * PAGE_SIZE;
		next = current + 1;
		current->NEXT = next;
		next->PREV = current;
	}
	current = current->PREV;
	current->NEXT = head;
	head->PREV = current;
}

u32 alloc_mem(u32 pid, size_t size)
{
	assert(pid >= (NR_NATIVE_PROCS + NR_TASKS));

	int chunk = (size + PROC_IMAGE_SIZE - 1) / PROC_IMAGE_SIZE;
	u32 base = PROCS_BASE +
			(pid - (NR_NATIVE_PROCS + NR_TASKS)) * PROC_IMAGE_SIZE;
	
	return base;
}

