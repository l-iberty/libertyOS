#include "type.h"
#include "proc.h"
#include "tty.h"
#include "fs.h"
#include "mm.h"
#include "exe.h"
#include "sysconst.h"
#include "protect.h"
#include "keyboard.h"
#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "global.h"
#include "irq.h"

#define TASK_PRIORITY 15
#define USER_PROC_PRIORITY 5

void proc_begin();
void init_clock();

void kernel_main()
{
    print("\n------------kernel_main------------\n");

    re_enter = 0;
    ticks = 0;

    struct proc* p_proc = proc_table;
    struct task* p_task = task_table;

    uint8_t dpl; /* 描述符特权级 */
    uint16_t rpl_mask; /* 请求特权级掩码 */
    uint32_t eflags;

    /* initialize proc_table */

    init_prot();

    for (int i = 0; i < NR_PROCS; i++, p_proc++, p_task++) {
        if (i >= NR_NATIVE_PROCS + NR_TASKS) {
            p_proc->flag = FREE_SLOT;
            continue;
        }
        if (i < NR_NATIVE_PROCS) { /* 用户进程 ring3 */
            dpl = DPL_3;
            rpl_mask = 0xFFFF;
            eflags = 0x202; /* IOPL = 0 (禁止用户进程的 I/O 操作), IF = 1, bit 2 is always 1 */
            p_proc->ticks = p_proc->priority = USER_PROC_PRIORITY;
        } else { /* 任务 ring1 */
            dpl = DPL_1;
            rpl_mask = 0xFFFD;
            eflags = 0x1202; /* IOPL = 1, IF = 1, bit 2 is always 1 */
            p_proc->ticks = p_proc->priority = TASK_PRIORITY;
        }

        /* 初始化进程/任务的 LDT 描述符 */
        if (i == PID_INIT) {
            /* 0~1M */
            init_desc(&p_proc->LDT[INDEX_LDT_C * DESC_SIZE], 0, PROC_IMAGE_SIZE - 1, DA_C32 | dpl);
            init_desc(&p_proc->LDT[INDEX_LDT_RW * DESC_SIZE], 0, PROC_IMAGE_SIZE - 1, DA_D32 | dpl);
        } else {
            /* 0~4G */
            init_desc(&p_proc->LDT[INDEX_LDT_C * DESC_SIZE], 0, 0xFFFFF, DA_C32 | DA_G_4K | dpl);
            init_desc(&p_proc->LDT[INDEX_LDT_RW * DESC_SIZE], 0, 0xFFFFF, DA_D32 | DA_G_4K | dpl);
        }

        /* 仅初始化必要的寄存器 */
        p_proc->regs.gs = SELECTOR_VIDEO;
        p_proc->regs.fs = SELECTOR_LDT_FLATRW & rpl_mask;
        p_proc->regs.es = SELECTOR_LDT_FLATRW & rpl_mask;
        p_proc->regs.ds = SELECTOR_LDT_FLATRW & rpl_mask;
        p_proc->regs.eip = (uint32_t)p_task->task_entry;
        p_proc->regs.cs = SELECTOR_LDT_FLATC & rpl_mask;
        p_proc->regs.eflags = eflags;
        p_proc->regs.esp = (uint32_t)p_task->task_stack + TASK_STACK_SIZE;
        p_proc->regs.ss = SELECTOR_LDT_FLATRW & rpl_mask;

        p_proc->pid = i;
        p_proc->pid_parent = INVALID_PID;
        p_proc->flag = READY;
        p_proc->p_msg = NULL;
        p_proc->pid_sendto = INVALID_PID;
        p_proc->pid_recvfrom = INVALID_PID;
        p_proc->has_int_msg = 0;
        p_proc->interval_ms = 0;
        p_proc->mm.start_brk = PROC_START_BRK;
        p_proc->mm.brk = PROC_START_BRK;
        p_proc->mm.mlimit = PROC_START_BRK;
        p_proc->mm.first_chunk = NULL;

        init_send_queue(p_proc);

        memset(p_proc->filp, 0, sizeof(struct file_desc*) * NR_FILES);
    }

    init_mm();
    init_clock();
    init_keyboard();

    p_current_proc = proc_table;
    proc_begin();

    while (1) {
    }
}
