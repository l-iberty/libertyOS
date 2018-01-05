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
#include "irq.h"

#define	TASK_PRIORITY		15
#define USER_PROC_PRIORITY	5

int f_reenter;

void proc_begin(); /* lib/klib.asm */


void kernel_main()
{
	print("\n------------kernel_main------------\n");
	
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
			_eflags = 0x202; /* IOPL = 0 (禁止用户进程的 I/O 操作), IF = 1, bit 2 is always 1 */
			p_proc->ticks = p_proc->priority = USER_PROC_PRIORITY;
		} else {		/* 任务 ring1 */
			_DPL = DPL_1;
			_RPL_MASK = 0xFFFD;
			_eflags = 0x1202; /* IOPL = 1, IF = 1, bit 2 is always 1 */
			p_proc->ticks = p_proc->priority = TASK_PRIORITY;
		}
		
		/* 初始化进程/任务的 LDT 描述符 */
		if (i == PID_INIT) {
			/* 0~1M */
			init_desc(&p_proc->LDT[INDEX_LDT_C * DESC_SIZE], 0, 0xFF, DA_C32 | DA_G_4K | _DPL);
			init_desc(&p_proc->LDT[INDEX_LDT_RW * DESC_SIZE], 0, 0xFF, DA_D32 | DA_G_4K | _DPL);
		}
		else {
			/* 0~4G */
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
	
	init_clock();
	init_keyboard();
	
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
		printf("\n-----Init-----");
		
		int pid = fork();
		
		if (pid != 0) {
			printf("\nparent 0x%.8x is running, child pid: 0x%.8x", getpid(), pid);
			while(1) {}
		} else {
			printf("\nchild 0x%.8x is running, parent pid: 0x%.8x", getpid(), getppid());
			while(1) {}
		}
	}
}

void TaskA()
{
	for (;;) {
		printf("\n-----TaskA-----");
		
		int fd, nr;
		char buf1[] = "qwertyiuop00000";
		char buf2[32];
		
		fd = open("/KERNEL.ASM", O_CREAT | O_RDWR);
		assert(fd != -1);
		
		nr = write(fd, buf1, strlen(buf1));
		assert(nr > 0);
		
		close(fd);
		
		fd = open("/KERNEL.ASM", O_RDWR);
		assert(fd != -1);
		
		nr = read(fd, buf2, sizeof(buf2));
		buf2[nr] = 0;
		close(fd);
		printf("\nnr read: %.4x, %s", nr, buf2);
		
		while(1){}
	}
}

void TaskB()
{
	for (;;) {}
}

void TaskC()
{
	for (;;) {}
}

