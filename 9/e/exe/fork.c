#include "proc.h"
#include "exe.h"
#include "mm.h"
#include "sysconst.h"
#include "global.h"
#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "protect.h"

uint32_t fork()
{
    struct message msg;
    msg.value = FORK;

    sendrecv(BOTH, PID_TASK_EXE, &msg);
    assert(msg.RETVAL == 0);

    return msg.FORK_PID;
}

uint32_t do_fork()
{
    /* find a free slot in proc_table[] */
    uint32_t pid = exe_msg.source;
    uint32_t child_pid = INVALID_PID;
    struct proc* p_child_proc;

    for (p_child_proc = &FIRST_PROC; p_child_proc <= &LAST_PROC; p_child_proc++) {
        if (p_child_proc->flag == FREE_SLOT) {
            child_pid = p_child_proc - &FIRST_PROC;
            break;
        }
    }
    if (child_pid == INVALID_PID) {
        printf("\n#ERROR#-do_fork: no free slot in proc_table[]");
        return -1;
    }

    /* copy PCB */
    uint16_t child_ldt_sel = p_child_proc->ldt_selector;
    uint32_t child_page_dir_base = p_child_proc->page_dir_base;

    memcpy(p_child_proc, &proc_table[pid], sizeof(struct proc));
    p_child_proc->pid = child_pid;
    p_child_proc->pid_parent = pid;
    p_child_proc->ldt_selector = child_ldt_sel;
    p_child_proc->page_dir_base = child_page_dir_base;

    uint8_t* p_desc;

    /* text segment */
    p_desc = &proc_table[pid].LDT[INDEX_LDT_C * DESC_SIZE];
    uint32_t caller_T_base = get_base(p_desc);
    uint32_t caller_T_limit = get_limit(p_desc);
    uint32_t caller_T_size = caller_T_limit + 1;

    /* data segment */
    p_desc = &proc_table[pid].LDT[INDEX_LDT_RW * DESC_SIZE];
    uint32_t caller_D_base = get_base(p_desc);
    uint32_t caller_D_limit = get_limit(p_desc);
    uint32_t caller_D_size = caller_D_limit + 1;

    assert((caller_T_base == caller_D_base) && (caller_T_limit == caller_D_limit) && (caller_T_size == caller_D_size));

    /* copy page table */
    uint32_t* pde = (uint32_t*)(FIRST_PROC.page_dir_base + child_pid * PAGE_TABLE_PAGES * PAGE_SIZE);
    phymemcpy(pde, (void*)proc_table[pid].page_dir_base, PAGE_TABLE_PAGES * PAGE_SIZE);
    relocate_pde(pde);

    void* child_base = vm_alloc_ex(child_pid, NULL, caller_D_size, PAGE_READWRITE);
    assert(child_base);

    /* child's LDT */
    init_desc(&p_child_proc->LDT[INDEX_LDT_RW * DESC_SIZE],
              (uint32_t)child_base,
              caller_D_limit,
              DA_D32 | DPL_3);

    /* copy data segment */
    phymemcpy(la2pa(p_child_proc, child_base), (void*)caller_D_base, caller_D_size);

    /* return child's PID to parent */
    exe_msg.FORK_PID = child_pid;

    /* wake up child */
    struct message msg2child;
    msg2child.value = FORK;
    msg2child.RETVAL = 0;
    msg2child.FORK_PID = 0;
    sendrecv(SEND, child_pid, &msg2child);

    return 0;
}
