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
#define N			10

int re_enter;

struct semaphore sem_mutex;
int array[N];

void proc_begin(); /* lib/klib.asm */


void kernel_main()
{
	print("\n------------kernel_main------------\n");
	
	re_enter = 0;
	
	ticks = 0;
	
	struct proc* p_proc = proc_table;
	struct task* p_task = task_table;
	
	uint8_t _DPL;	/* 描述符特权级 */
	uint16_t _RPL_MASK;	/* 请求特权级掩码 */
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
			init_desc(&p_proc->LDT[INDEX_LDT_C * DESC_SIZE], 0, 0xFF, DA_C32 | DA_G_4K | _DPL);
			init_desc(&p_proc->LDT[INDEX_LDT_RW * DESC_SIZE], 0, 0xFF, DA_D32 | DA_G_4K | _DPL);
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
		
		p_proc->pid = i;
		p_proc->pid_parent = NONE;
		p_proc->flag = 0;
		p_proc->p_msg = NULL;
		p_proc->pid_sendto = NONE;
		p_proc->pid_recvfrom = NONE;
		p_proc->has_int_msg = 0;
		
		init_send_queue(p_proc);
		
		memset(p_proc->filp, 0, sizeof(struct file_desc*) * NR_FILES);
	}
	
	init_clock();
	init_keyboard();
	
	p_current_proc = proc_table;
	proc_begin();
	
	while(1) {}
}

void sleep(int ms)
{
	int t = get_ticks();
	
	while (((get_ticks() - t) * 1000 / CLK_FREQ) < ms) {}
}


void Init()
{
	int pid;
	
	sleep(5000);

	pid = fork();
	
	if (pid != 0) 
	{
		printf("\n{Init} parent %d running, child pid: %d", getpid(), pid);
		
		pid = fork();
		if (pid != 0)
		{
			printf("\n{Init} parent %d running, child pid: %d", getpid(), pid);
		}
		else
		{
			printf("\n{Init} child %d running, parent pid: %d", getpid(), getppid());
		}
	}
	else
	{
		printf("\n{Init} child %d running, parent pid: %d", getpid(), getppid());
	}

	for (;;) 
	{
	
	}
}

void TaskA()
{
	int i;
	
	printf("\n{Task-A}");
	
	sem_init(&sem_mutex, 1);

	sem_wait(&sem_mutex);
	printf("\nTask-A sem_wait succeeds\n");
	for (i = 0; i < N; i++)
	{
		array[i] = i + 1;
		sleep(1000);
	}
	sem_post(&sem_mutex);
	
	for (;;)
	{

	}
}

void TaskB()
{
	int i;
	
	printf("\n{Task-B}");
	
	sem_wait(&sem_mutex);
	printf("\n\nTask-B sem_wait succeeds\n");
	for (i = 0; i < N; i++)
	{
		printf("%d ", array[i]);
	}
	sem_post(&sem_mutex);
	printf("\n");

	for (;;)
	{
		
	}
}

void TaskC()
{
	printf("\n{Task-C}");
	
	void *la;
	
	/* 11M ~ 13M */
	la = vm_alloc((void*)0xb00080, 0x200000, PAGE_READWRITE);
	printf("\n(1) vmalloc: %.8x",(uint32_t)la);
	assert(la);
	memset(la, 0xcd, 0x200000);

	/* 8M ~ 12M */
	la = vm_alloc((void*)0x800600, 0x400000, PAGE_READWRITE);
	printf("\n(2) vmalloc: %.8x",(uint32_t)la);
	assert(la);
	memset(la, 0xcc, 0x400000);
	
	/* 12M ~ 12M + 7K */
	la = vm_alloc((void*)0x1c00000, 7 * 1024, PAGE_READWRITE);
	printf("\n(3) vmalloc: %.8x", (uint32_t)la);
	assert(la);
	memset(la, 0xdd, 4096 * 2);

	/* 9M ~ 12M */
	la = vm_alloc((void*)0x900000, 0x300000, PAGE_READWRITE);
	printf("\n(4) vmalloc: %.8x", (uint32_t)la);
	assert(la);
	memset(la, 0xdd, 0x300000);

	int fd;
	char s[32];
	fd = open("/test1", O_CREAT | O_RDWR);
	write(fd, "testfile", 9);
	close(fd);
	
	fd = open("/test2", O_CREAT | O_RDWR);
	write(fd, "hello, world", 13);
	close(fd);

	fd = open("/test1", O_RDWR);
	read(fd, s, sizeof(s));
	close(fd);
	printf("\ntest1: %s",s);

	fd = open("/test2", O_RDWR);
	read(fd, s, sizeof(s));
	close(fd);
	printf("\ntest2: %s",s);

	for (;;)
	{

	}
}

