
#include "type.h"
#include "proc.h"
#include "sysconst.h"
#include "protect.h"
#include "keyboard.h"

#define	MAX_PRIORITY 0xFFFFFFFF

int f_reenter;

void proc_begin();	/* lib/klib.asm */
void println(char *sz);	/* lib/klib.asm */
void print(char *sz);	/* lib/klib.asm */
void itoa(char* str, int v, int len, u8 flag);	/* lib/klib.asm */
u32 _strlen(char* s);				/* lib/string.asm */
void _strcpy(char* dst, const char* src);	/* lib/string.asm */
void _printf(const char* fmt, ...);

int get_ticks(); /* system call */

extern u8	GDT[GDT_DESC_NUM * DESC_SIZE];	/* kernel/kernel.asm */

extern int ticks;

TASK task_table[NR_PROCS + NR_TASKS] = {{ TaskA, task_stack_a },
		   	                { TaskB, task_stack_b },
		   	                { TaskC, task_stack_c },
		   	                { Task_tty, task_stack_tty }};
		   	          
SYSCALL syscall_table[NR_SYSCALL] = { sys_get_ticks };

void kernel_main()
{
	println("------------kernel_main------------");
	
	init_keyboard();
	
	f_reenter = 0;
	
	ticks = 0;
	
	PROCESS* p_proc = proc_table;
	TASK* p_task = task_table;
	
	u8 _DPL;	/* 描述符特权级 */
	u16 _RPL_MASK;	/* 请求特权级掩码 */
	u32 _eflags;
	
	for (int i = 0; i < NR_PROCS + NR_TASKS; i++, p_proc++, p_task++)
	{
		if (i < NR_PROCS) {	/* 用户进程 ring3 */
			_DPL = DPL_3;
			_RPL_MASK = 0xFFFF;
			_eflags = 0x202; /* IOPL = 0 (禁止用户进程的 I/O 操作), IF = 1, bit 2 is always 1 */
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
	}
	
	proc_table[0].ticks = proc_table[0].priority = 1;
	proc_table[1].ticks = proc_table[1].priority = 1;
	proc_table[2].ticks = proc_table[2].priority = 1;
	proc_table[3].ticks = proc_table[3].priority = MAX_PRIORITY;
	
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
	while (1)
	{
/*		char szTicks[5];*/
/*		itoa(szTicks, get_ticks(), 4, 1);*/
/*		print(szTicks);*/
/*		print("A.");*/
/*		delay(50);*/

		println("\n----------TaskA----------\n");
		
		_printf("This is a string: %s", "linux");
		println("Hello,\nWorld\n!");		
		println("Using printf:");
		_printf("%.2x, %.4x, %.8x, file: %s",
			12, 0x1234, 0x7FF1BCD, __FILE__);
		
		println("\n----------TaskA----------");
			
		__asm__("ud2");
	}
}

void TaskB()
{
	while (1)
	{
/*		print("B.");*/
/*		delay(50);*/
	}
}

void TaskC()
{
	while (1)
	{
/*		print("C.");*/
/*		delay(50);*/
	}
}

int sys_get_ticks()
{
	print("^");
	return ticks;
}





























