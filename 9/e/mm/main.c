#include "proc.h"
#include "mm.h"
#include "sysconst.h"
#include "global.h"
#include "stdio.h"
#include "stdlib.h"
#include "string.h"

struct message mm_msg;
struct page_list* pf_list; /* 管理页框的双向循环链表 */
struct meminfo* mi; /* 保存内存信息 */

void TaskMM()
{
    printf("\n---------{TaskMM} Page Dir Base: 0x%.8x---------", getcr3());

    for (;;) {
        sendrecv(RECEIVE, ANY, &mm_msg);

        switch (mm_msg.value) {
        case VM_ALLOC:
            mm_msg.VM_BASE = do_vm_alloc();
            break;
        case VM_FREE:
            do_vm_free();
            break;
        case VM_ALLOC_EX:
            mm_msg.VM_BASE = do_vm_alloc();
            break;
        case BRK:
            mm_msg.RETVAL = do_brk();
            break;
        case SBRK:
            mm_msg.NEW_BRK = do_sbrk();
            break;
        default:
            printf("\nMM-{unknown message value}");
            dump_msg(&mm_msg);
        }

        sendrecv(SEND, mm_msg.source, &mm_msg);
    }
}

/**
 * NOTE:
 * 1. 分段时使用的是平坦模式, 代码段和数据段的基址为零, 因此虚拟地址 = 线性地址;
 * 2. 页目录和页表连续存放.
 */
void init_mm()
{
    int i, j;

    mi = (struct meminfo*)(MEM_INFO_VA_BASE);

    /* 所管理的物理内存是存放页表的那一块 */

    uint32_t avail_pm_base = mi->page_dir_base;
    uint32_t avail_pm_size = 0;
    uint32_t ram_size = 0;
    int avail_pm_pages;
    int reserved_pages;

    for (i = 0; i < mi->nr_pm_block; i++) {
        int n = mi->pm_block_info[i].avail_base + mi->pm_block_info[i].avail_size;
        if (n > ram_size) {
            ram_size = n;
        }

        if (mi->pm_block_info[i].avail_base == mi->page_dir_base) {
            avail_pm_size = mi->pm_block_info[i].avail_size;
            avail_pm_pages = avail_pm_size / PAGE_SIZE;
        }
    }

    /**
	 * 建立双向循环链表`pf_list', 以页为单位管理从 avail_pm_base 开始, 大小 avail_pm_size 的
	 * 物理内存. 页目录、页表及`pf_list'占据的页面被标记为 PAGE_RESERVED, 被OS保留; 其余标记
	 * 为 PAGE_FREE, 用于后续分配.
	 *
	 * 每个进程都有单独的一套页表, 将 LOADER 建立的那套页表作为 pid=0 的进程的页表 (以下简称
	 * "0号页表"), 并将其拷贝 (NR_PROCS - 1) 份, 用作其余进程的页表.
	 */

    /* 全部进程的二级页表占据的页面数 */
    reserved_pages = NR_PROCS * PAGE_TABLE_PAGES;

    struct page_list* current, *next;
    pf_list = (struct page_list*)(avail_pm_base + reserved_pages * PAGE_SIZE);
    current = pf_list;

    reserved_pages += (sizeof(struct page_list) * avail_pm_pages + PAGE_SIZE - 1) / PAGE_SIZE; /* `pf_list'占据的页面数 */

    for (i = 0; i < avail_pm_pages; i++) {
        if (i < reserved_pages) {
            current->REF = 1;
            current->TYPE = PAGE_RESERVED;
            current->PROTECT = PAGE_READ | PAGE_READWRITE;
        } else {
            current->REF = 0;
            current->TYPE = PAGE_FREE;
            current->PROTECT = 0;
        }
        current->BASE = avail_pm_base + i * PAGE_SIZE;
        next = current + 1;
        current->NEXT = next;
        next->PREV = current;
        current = next;
    }
    current = next->PREV;
    current->NEXT = pf_list;
    pf_list->PREV = current;

    /* 将 TYPE=PAGE_FREE 的页框从0号页表中解除映射, 需要的时候再建立映射 */
    uint32_t* pde = (uint32_t*)mi->page_dir_base;
    uint32_t* pte = (uint32_t*)mi->page_tbl_base;

    /* (1) 将对应的 PTE 清零 */
    current = pf_list;
    do {
        if (current->TYPE == PAGE_FREE) {
            pte[PDE_INDEX(current->BASE) * MAX_PAGE_ITEM + PTE_INDEX(current->BASE)] = 0;
        }
        current = current->NEXT;
    } while (current != pf_list);

    /* (2) 如果某一页目录项对应的页表已被全部清零, 则将该页目录项清零 */
    for (i = 0; i < mi->nr_pde; i++) {
        pte = (uint32_t*)GET_BASE(pde[i]);
        if (pte[0] == 0) { /* 如果第一个 PTE 为零, 则后面的一定也是零 */
            pde[i] = 0;
        }
    }

    /* 将0号页表拷贝给其他进程 */
    pde = (uint32_t*)(mi->page_dir_base + PAGE_TABLE_PAGES * PAGE_SIZE);
    for (i = 0; i < NR_NATIVE_PROCS + NR_TASKS - 1; i++) {
        memcpy(pde, (void*)mi->page_dir_base, PAGE_TABLE_PAGES * PAGE_SIZE);
        relocate_pde(pde); /* 重定位页目录中的页表地址值 */
        pde += PAGE_TABLE_PAGES * MAX_PAGE_ITEM;
    }

    /* 将各个进程的页目录地址记录到 PCB 中 */
    struct proc* p_proc = &FIRST_PROC;
    for (i = 0; i < NR_PROCS; i++) {
        p_proc->page_dir_base = mi->page_dir_base + i * PAGE_TABLE_PAGES * PAGE_SIZE;
        p_proc++;
    }
}
