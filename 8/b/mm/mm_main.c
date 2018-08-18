#include "proc.h"
#include "mm.h"
#include "sysconst.h"
#include "global.h"
#include "stdio.h"
#include "stdlib.h"
#include "string.h"

struct message   mm_msg;
struct page_list *pf_list; /* 管理页框的双向循环链表 */
struct meminfo   *mi;      /* 保存内存信息 */

void TaskMM()
{
	printf("\n-----TaskMM-----");
	
	init_mm();
	
	for (;;) 
	{
		sendrecv(RECEIVE, ANY, &mm_msg);
		
		switch (mm_msg.value)
		{
			case VM_ALLOC:
			{
				mm_msg.VM_BASE = do_vm_alloc();
				break;
			}
			default:
			{
				printf("\nMM-{unknown message value}");
				dump_msg(&mm_msg);
			}
		}

		sendrecv(SEND, mm_msg.source, &mm_msg);
	}
}


/**
 * NOTE:
 * 1. 分段时使用的是平坦模式, 代码段和数据段的基址为零, 因此虚拟地址 = 线性地址;
 *    loader 建立的页表又使得 线性地址 = 物理地址.
 *    从而, 虚拟地址 = 线性地址 = 物理地址
 * 2. 页目录和页表连续存放.
 */
void init_mm()
{
	int i;
	
	mi = (struct meminfo*)(MEM_INFO_VA_BASE);
	
	printf("\n{MM}    Page Directory Base: 0x%.8x", mi->page_dir_base);
	printf("\n{MM}    Page Table Base: 0x%.8x", mi->page_tbl_base);
	printf("\n{MM}    Number of PDE: %d", mi->nr_pde);
	
	/* 所管理的物理内存是存放页表的那一块 */
	
	uint32_t avail_pm_base = mi->page_dir_base;
	uint32_t avail_pm_size = 0;
	uint32_t ram_size = 0;
	int avail_pm_pages;
	
	for (i = 0; i < mi->nr_pm_block; i++)
	{
		printf("\n{MM}    Avaliable PM Base: 0x%.8x, Size: 0x%.8x",
			mi->pm_block_info[i].avail_base,
			mi->pm_block_info[i].avail_size);
		
		int n = mi->pm_block_info[i].avail_base + mi->pm_block_info[i].avail_size;
		if (n > ram_size) { ram_size = n; }
		
		if (mi->pm_block_info[i].avail_base == mi->page_dir_base)
		{
			avail_pm_size = mi->pm_block_info[i].avail_size;
			avail_pm_pages = avail_pm_size / PAGE_SIZE;
		}
	}
	printf("\n{MM}    RAM Size: %dM", ram_size >> 20);
	int reserved_pages = 1 + 1024; /* 二级页表最大占据的页面数: 1 个页目录, 1024 个页表 */
	
	/**
	 * 建立双向循环链表, 以页为单位管理从 avail_pm_base 开始, 大小 avail_pm_size 的物理内存.
	 * 由于页目录和页表从 avail_pm_base 开始存放, 因此将其占据的页框 type 设为 PAGE_RESERVED,
	 * 被OS保留; 其余设为 PAGE_FREE, 以便后续分配.
	 *
	 * 链表紧随页表存放, 所占内存大小 sizeof(struct page_list) * avail_pages, 这部分内存也要保留.
	 */

	struct page_list *current, *next;
	pf_list = (struct page_list*)(avail_pm_base + reserved_pages * PAGE_SIZE);
	current = pf_list;
	reserved_pages += (sizeof(struct page_list) * avail_pm_pages + PAGE_SIZE - 1) / PAGE_SIZE;
	
	printf("\n{MM}    Reserved pages: %d", reserved_pages);
	
	for (i = 0; i < avail_pm_pages; i++, current = next)
	{
		if (i < reserved_pages)
		{
			current->REF = 1;
			current->TYPE = PAGE_RESERVED;
			current->PROTECT = PAGE_READ | PAGE_READWRITE;
		}
		else
		{
			current->REF = 0;
			current->TYPE = PAGE_FREE;
			current->PROTECT = 0;
		}
		current->BASE = avail_pm_base + i * PAGE_SIZE;
		next = current + 1;
		current->NEXT = next;
		next->PREV = current;
	}
	current = current->PREV;
	current->NEXT = pf_list;
	pf_list->PREV = current;
	
	/* 将 TYPE=PAGE_FREE 的页框从页表中解除映射, 需要的时候再建立映射 */
	uint32_t *pde = (uint32_t*) mi->page_dir_base;
	uint32_t *pte = (uint32_t*) mi->page_tbl_base;
	
	/* (1) 将对应的 PTE 清零 */
	current = pf_list;
	do
	{
		if (current->TYPE == PAGE_FREE)
		{
			pte[PDE_INDEX(current->BASE) * MAX_PAGE_ITEM +
			    PTE_INDEX(current->BASE)] = 0;
		}
		current = current->NEXT;
	} while (current != pf_list);
	
	/* (2) 如果某一页目录项对应的页表已被全部清零, 则将该页目录项清零 */
	for (i = 0; i < mi->nr_pde; i++)
	{
		pte = (uint32_t*) GET_BASE(pde[i]);
		if (pte[0] == 0) /* 如果第一个 PTE 为零, 则后面的一定也是零 */
		{
			pde[i] = 0;
		}
	}
}


