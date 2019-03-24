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

#define	TASK_PRIORITY		15
#define USER_PROC_PRIORITY	5

int re_enter;

void proc_begin();
void init_clock();


void kernel_main()
{
	print("\n------------kernel_main------------\n");
	
	re_enter = 0;
	
	ticks = 0;
	
	struct proc* p_proc = proc_table;
	struct task* p_task = task_table;
	
	uint8_t _DPL; /* 描述符特权级 */
	uint16_t _RPL_MASK; /* 请求特权级掩码 */
	uint32_t _eflags;
	
	/* initialize proc_table */
	
	init_prot();
	
	for (int i = 0; i < NR_PROCS; i++, p_proc++, p_task++)
	{
		if (i >= NR_NATIVE_PROCS + NR_TASKS)
		{
			p_proc->flag = FREE_SLOT;
			continue;
		}
		if (i < NR_NATIVE_PROCS)	/* 用户进程 ring3 */
		{
			_DPL = DPL_3;
			_RPL_MASK = 0xFFFF;
			_eflags = 0x202; /* IOPL = 0 (禁止用户进程的 I/O 操作), IF = 1, bit 2 is always 1 */
			p_proc->ticks = p_proc->priority = USER_PROC_PRIORITY;
		}
		else	/* 任务 ring1 */
		{
			_DPL = DPL_1;
			_RPL_MASK = 0xFFFD;
			_eflags = 0x1202; /* IOPL = 1, IF = 1, bit 2 is always 1 */
			p_proc->ticks = p_proc->priority = TASK_PRIORITY;
		}
		
		/* 初始化进程/任务的 LDT 描述符 */
		if (i == PID_INIT)
		{
			/* 0~1M */
			init_desc(&p_proc->LDT[INDEX_LDT_C * DESC_SIZE],  0, PROC_IMAGE_SIZE - 1, DA_C32 | _DPL);
			init_desc(&p_proc->LDT[INDEX_LDT_RW * DESC_SIZE], 0, PROC_IMAGE_SIZE - 1, DA_D32 | _DPL);
		}
		else
		{
			/* 0~4G */
			init_desc(&p_proc->LDT[INDEX_LDT_C * DESC_SIZE], 0, 0xFFFFF, DA_C32 | DA_G_4K | _DPL);
			init_desc(&p_proc->LDT[INDEX_LDT_RW * DESC_SIZE], 0, 0xFFFFF, DA_D32 | DA_G_4K | _DPL);
		}
		
		/* 仅初始化必要的寄存器 */
		p_proc->regs.gs = SELECTOR_VIDEO;
		p_proc->regs.fs = SELECTOR_LDT_FLATRW & _RPL_MASK;
		p_proc->regs.es = SELECTOR_LDT_FLATRW & _RPL_MASK;
		p_proc->regs.ds = SELECTOR_LDT_FLATRW & _RPL_MASK;
		p_proc->regs.eip = (uint32_t) p_task->task_entry;
		p_proc->regs.cs = SELECTOR_LDT_FLATC & _RPL_MASK;
		p_proc->regs.eflags = _eflags;
		p_proc->regs.esp = (uint32_t) p_task->task_stack + TASK_STACK_SIZE;
		p_proc->regs.ss = SELECTOR_LDT_FLATRW & _RPL_MASK;
		
		p_proc->pid =		i;
		p_proc->pid_parent =	NONE;
		p_proc->flag =		READY;
		p_proc->p_msg =		NULL;
		p_proc->pid_sendto =	NONE;
		p_proc->pid_recvfrom =	NONE;
		p_proc->has_int_msg =	0;
		p_proc->interval_ms =	0;
		p_proc->start_brk = 	PROC_START_BRK;
		p_proc->brk = 		PROC_START_BRK;
		
		init_send_queue(p_proc);
		
		memset(p_proc->filp, 0, sizeof(struct file_desc*) * NR_FILES);
	}
	
	init_mm();
	init_clock();
	init_keyboard();
	
	p_current_proc = proc_table;
	proc_begin();
	
	while(1) {}
}



