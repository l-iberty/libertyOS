#include "type.h"
#include "proc.h"
#include "fs.h"
#include "mm.h"
#include "sysconst.h"
#include "protect.h"
#include "keyboard.h"
#include "tty.h"
#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "global.h"

#define	TASK_PRIORITY		15
#define USER_PROC_PRIORITY	5

int f_reenter;

void proc_begin(); /* lib/klib.asm */

TASK task_table[NR_PROCS] = {{ Init, task_stack_init },
                             { TaskA, task_stack_a },
                             { TaskB, task_stack_b },
                             { TaskC, task_stack_c },
                             { Task_tty, task_stack_tty },
                             { Task_hd, task_stack_hd },
                             { Task_fs, task_stack_fs },
                             { Task_mm, task_stack_mm } };
		   	          
SYSCALL syscall_table[NR_SYSCALL] = { sys_get_ticks,
                                      sys_sendrecv };

void kernel_main()
{
	printf("------------kernel_main------------\n");
	
	f_reenter = 0;
	
	ticks = 0;
	
	PROCESS* p_proc = proc_table;
	TASK* p_task = task_table;
	
	u8 _DPL;	/* 描述符特权级 */
	u16 _RPL_MASK;	/* 请求特权级掩码 */
	u32 _eflags;
	
	/* initialize proc_table */
	
	init_prot();
	
	for (int i = 0; i < NR_PROCS; i++, p_proc++, p_task++)
	{
		if (i >= NR_NATIVE_PROCS + NR_TASKS) {
			p_proc->flag = FREE_SLOT;
			continue;
		}
		if (i < NR_NATIVE_PROCS) {	/* 用户进程 ring3 */
			_DPL = DPL_3;
			_RPL_MASK = 0xFFFF;
			_eflags = 0x3202; /* IOPL = 3 (允许用户进程的 I/O 操作), IF = 1, bit 2 is always 1 */
			p_proc->ticks = p_proc->priority = USER_PROC_PRIORITY;
		} else {		/* 任务 ring1 */
			_DPL = DPL_1;
			_RPL_MASK = 0xFFFD;
			_eflags = 0x1202; /* IOPL = 1, IF = 1, bit 2 is always 1 */
			p_proc->ticks = p_proc->priority = TASK_PRIORITY;
		}
		
		/* 初始化进程/任务的 LDT 描述符 */
		if (i == PID_INIT) {
			init_desc(&p_proc->LDT[INDEX_LDT_C * DESC_SIZE], 0, 0xFF, DA_C32 | DA_G_4K | _DPL);
			init_desc(&p_proc->LDT[INDEX_LDT_RW * DESC_SIZE], 0, 0xFF, DA_D32 | DA_G_4K | _DPL);
		}
		else {
			init_desc(&p_proc->LDT[INDEX_LDT_C * DESC_SIZE], 0, 0xFFFFF, DA_C32 | DA_G_4K | _DPL);
			init_desc(&p_proc->LDT[INDEX_LDT_RW * DESC_SIZE], 0, 0xFFFFF, DA_D32 | DA_G_4K | _DPL);
		}
		
		/* 仅初始化必要的寄存器 */
		p_proc->regs.gs = SELECTOR_VIDEO;
		p_proc->regs.fs = SELECTOR_LDT_FLATRW & _RPL_MASK;
		p_proc->regs.es = SELECTOR_LDT_FLATRW & _RPL_MASK;
		p_proc->regs.ds = SELECTOR_LDT_FLATRW & _RPL_MASK;
		p_proc->regs.eip = (u32) p_task->task_entry;
		p_proc->regs.cs = SELECTOR_LDT_FLATC & _RPL_MASK;
		p_proc->regs.eflags = _eflags;
		p_proc->regs.esp = (u32) p_task->task_stack + TASK_STACK_SIZE;
		p_proc->regs.ss = SELECTOR_LDT_FLATRW & _RPL_MASK;
		
		p_proc->pid = i;
		p_proc->pid_parent = NONE;
		p_proc->flag = 0;
		p_proc->p_msg = NULL;
		p_proc->pid_sendto = NONE;
		p_proc->pid_recvfrom = NONE;
		p_proc->has_int_msg = 0;
		
		init_send_queue(p_proc);
		
		memset(p_proc->filp, 0, sizeof(FILE_DESC*) * NR_FILES);
	}
	
	p_current_proc = proc_table;
	proc_begin();
	
	while(1) {}
}

void delay(int x)
{
	for (int i = 0; i < x * 100; i++)
	{
		for (int j = 0; j < 1000; j++);
	}
}


void Init()
{
	for (;;) {
		delay(5);
		int pid = fork();
		
		if (pid != 0) {
			printf("\nparent 0x%.8x is running, child pid: 0x%.8x", getpid(), pid);
			while(1) {}
		} else {
			__asm__("ud2");
			printf("\n\nchild is running, base: 0x%.8x, pid: 0x%.8x; parent: 0x%.8x\nldt_sel: %.4x",
				get_base(&p_current_proc->LDT[INDEX_LDT_C * DESC_SIZE]), p_current_proc->pid, getppid(),
				p_current_proc->ldt_selector);
			
			while(1) {}
		}
	}
}

void TaskA()
{
	for (;;) {
	
	}
}

void TaskB()
{
	for (;;) {
	
	}
}

void TaskC()
{
	for (;;) {
	
	}
}

