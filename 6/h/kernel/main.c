#include "type.h"
#include "proc.h"
#include "fs.h"
#include "sysconst.h"
#include "protect.h"
#include "keyboard.h"
#include "tty.h"
#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "global.h"

#define	MAX_PRIORITY 0x7FFFFFFF
#define MIN_PRIORITY 0x2

extern u8 GDT[GDT_DESC_NUM * DESC_SIZE]; /* kernel/kernel.asm */

int f_reenter;

void proc_begin(); /* lib/klib.asm */

TASK task_table[NR_PROCS + NR_TASKS] = {{ TaskA, task_stack_a },
										{ TaskB, task_stack_b },
										{ TaskC, task_stack_c },
										{ Task_tty, task_stack_tty },
										{ Task_hd, task_stack_hd },
										{ Task_fs, task_stack_fs } };
		   	          
SYSCALL syscall_table[NR_SYSCALL] = { sys_get_ticks,
				      				  sys_sendrecv };

void kernel_main()
{
	_printf("------------kernel_main------------\n");
	
	f_reenter = 0;
	
	ticks = 0;
	
	PROCESS* p_proc = proc_table;
	TASK* p_task = task_table;
	
	u8 _DPL;	/* 描述符特权级 */
	u16 _RPL_MASK;	/* 请求特权级掩码 */
	u32 _eflags;
	
	/* initialize proc_table */
	for (int i = 0; i < NR_PROCS + NR_TASKS; i++, p_proc++, p_task++)
	{
		if (i < NR_PROCS) {	/* 用户进程 ring3 */
			_DPL = DPL_3;
			_RPL_MASK = 0xFFFF;
			_eflags = 0x3202; /* IOPL = 3 (允许用户进程的 I/O 操作), IF = 1, bit 2 is always 1 */
		} else {		/* 任务 ring1 */
			_DPL = DPL_1;
			_RPL_MASK = 0xFFFD;
			_eflags = 0x1202; /* IOPL = 1, IF = 1, bit 2 is always 1 */
		}
		
		/* 初始化 GDT 中进程/任务的 LDT 描述符 */
		init_desc(&GDT[(INDEX_LDT_DESC_FIRST + i) * DESC_SIZE], (u32) p_proc->LDT,
			sizeof(p_proc->LDT) - 1, DA_LDT);
			
		/* 与进程/任务的描述符对应的 GDT 选择子 */
		p_proc->ldt_selector = SELECTOR_LDT_FIRST + (i << 3);
		
		/* 初始化进程/任务的 LDT 描述符 */
		init_desc(&p_proc->LDT[0 * DESC_SIZE], 0, 0xFFFFF, DA_C32 | DA_G_4K | _DPL);
		init_desc(&p_proc->LDT[1 * DESC_SIZE], 0, 0xFFFFF, DA_D32 | DA_G_4K | _DPL);
		
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
		p_proc->flag = 0;
		p_proc->p_msg = NULL;
		p_proc->pid_sendto = NONE;
		p_proc->pid_recvfrom = NONE;
		p_proc->has_int_msg = 0;
		
		init_send_queue(p_proc);
		
		_memset(p_proc->filp, 0, sizeof(FILE_DESC*) * NR_FILES);
	}
	
	/* initialize f_desc_table & inode_table */
	_memset(f_desc_table, 0, sizeof(FILE_DESC) * NR_FILES);
	_memset(inode_table, 0, sizeof(I_NODE) * NR_FILES);
	
	proc_table[0].ticks = proc_table[0].priority = 105;
	proc_table[1].ticks = proc_table[1].priority = MIN_PRIORITY;
	proc_table[2].ticks = proc_table[2].priority = 100;
	proc_table[3].ticks = proc_table[3].priority = MIN_PRIORITY;
	proc_table[4].ticks = proc_table[4].priority = 100;
	proc_table[5].ticks = proc_table[5].priority = 100;
	
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


void TaskA()
{
	for (;;)
	{
		delay(5);
		
		int fd;
		
		init_video();
		_printf("\n-----TaskA-----");
		
		fd = open("/taska-1", O_CREAT);
		_printf("\nfd: 0x%.8x;\nfile_desc:\nfd_mode: 0x%.8x, fd_pos: 0x%.8x",
			fd, p_current_proc->filp[fd]->fd_mode, p_current_proc->filp[fd]->fd_pos);
		_printf("\ninode:\ni_mode: 0x%.8x, i_size: 0x%.8x,\
				\ni_start_sector: 0x%.8x, i_nr_sectors: 0x%.8x\n",
			p_current_proc->filp[fd]->fd_inode->i_mode,
			p_current_proc->filp[fd]->fd_inode->i_size,
			p_current_proc->filp[fd]->fd_inode->i_start_sector,
			p_current_proc->filp[fd]->fd_inode->i_nr_sectors);
		
		_printf("\nclose(%.1x) return: 0x%.8x\n", fd, close(fd));
		_printf("\nclose(%.1x) return: 0x%.8x\n", fd, close(fd)); /* fail */
		
		halt("TaskA");
	}
}

void TaskB()
{
	for (;;)
	{
		delay(5);
	}
}

void TaskC()
{
	for (;;)
	{
		delay(5);
	}
}

