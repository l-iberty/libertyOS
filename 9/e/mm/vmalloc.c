#include "proc.h"
#include "mm.h"
#include "type.h"
#include "sysconst.h"
#include "stdio.h"
#include "stdlib.h"
#include "global.h"

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
void* vm_alloc(void* vm_addr, uint32_t vm_size, uint32_t vm_protect)
{
    struct message msg;

    msg.value = VM_ALLOC;
    msg.VM_ADDR = vm_addr;
    msg.VM_SIZE = vm_size;
    msg.VM_PROTECT = vm_protect;
    msg.PD_BASE = p_current_proc->page_dir_base;
    msg.PT_BASE = msg.PD_BASE + PAGE_SIZE; /* 页表紧随页目录 */

    sendrecv(BOTH, PID_TASK_MM, &msg);

    return msg.VM_BASE;
}

void* vm_alloc_ex(uint32_t pid, void* vm_addr, uint32_t vm_size, uint32_t vm_protect)
{
    struct message msg;

    msg.value = VM_ALLOC;
    msg.VM_ADDR = vm_addr;
    msg.VM_SIZE = vm_size;
    msg.VM_PROTECT = vm_protect;
    msg.PD_BASE = proc_table[pid].page_dir_base;
    msg.PT_BASE = msg.PD_BASE + PAGE_SIZE;

    sendrecv(BOTH, PID_TASK_MM, &msg);

    return msg.VM_BASE;
}

void* do_vm_alloc()
{
    int i;
    uint32_t vm_base = 0; /* return value */

    int nr_pages = (mm_msg.VM_SIZE + PAGE_SIZE - 1) / PAGE_SIZE;
    uint32_t vm_addr = (uint32_t)mm_msg.VM_ADDR;
    uint32_t vm_protect = mm_msg.VM_PROTECT;
    uint32_t page_dir_base = mm_msg.PD_BASE;
    uint32_t page_tbl_base = mm_msg.PT_BASE;

    uint32_t pde_idx, pte_idx, pde_val, pte_val;

    uint32_t* pde = (uint32_t*)page_dir_base;
    uint32_t* pte = (uint32_t*)page_tbl_base;

    struct page_list* p;

    p = alloc_frame(nr_pages, vm_protect);
    if (p == NULL) {
        printf("\n#ERROR#{do_vm_alloc} physical memory not enough.");
        return NULL;
    }

    if (vm_protect & PAGE_READWRITE) {
        if (vm_addr) {
            vm_base = ROUND_DOWN(vm_addr, PAGE_SIZE);
            pde_idx = PDE_INDEX(vm_base);
            pte_idx = PTE_INDEX(vm_base);

            /* 映射这些页框所需的 PTE 是否全部空闲? */
            if (!check_free_page(pte, pde_idx * MAX_PAGE_ITEM + pte_idx, nr_pages)) {
                printf("\n#ERROR#{do_vm_alloc} virtual memory not enough.#1");
                return NULL;
            }
        } else {
            if (!alloc_page(nr_pages, pte, &pde_idx, &pte_idx)) {
                printf("\n#ERROR#{do_vm_alloc} virtual memory not enough.#2");
                return NULL;
            }
            vm_base = MAKE_LINEAR_ADDR(pde_idx, pte_idx, 0);
        }
    } else { /* 如果分配的 PAGE_READ 页面, 直接映射即可 */
        vm_base = ROUND_DOWN(vm_addr, PAGE_SIZE);
    }

    /* 将 p->BASE 开始的 nr_pages 个页框映射到线性地址 vm_base */
    map_frame(p, page_dir_base, vm_base, nr_pages, vm_protect);

    return (void*)vm_base;
}
