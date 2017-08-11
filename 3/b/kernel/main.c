#include "type.h"
#include "proc.h"
#include "sysconst.h"
#include "protect.h"

PROCESS		proc_table[NR_TASKS];

PROCESS*	p_current_proc;

void proc_begin();
void println(char *sz);
void print(char *sz);

extern u8	GDT[GDT_DESC_NUM * DESC_SIZE];	/* kernel/kernel.asm */

void kernel_main()
{
	PROCESS* p_proc = proc_table;
	
	/* 初始化 GDT 中 TaskA 的 LDT 描述符 */
	init_desc(&GDT[INDEX_LDT_DESC_A * DESC_SIZE], (u32) p_proc->LDT,
		sizeof(p_proc->LDT) - 1, DA_LDT);
	
	/* 与 TaskA 的描述符对应的 GDT 选择子 */
	p_proc->ldt_selector = SELECTOR_LDT_A;
	
	/* 初始化 TaskA 的 LDT 描述符 */
	init_desc(&p_proc->LDT[0 * DESC_SIZE], 0, 0xFFFFF, DA_C32 | DA_G_4K | DPL_3);
	init_desc(&p_proc->LDT[1 * DESC_SIZE], 0, 0xFFFFF, DA_D32 | DA_G_4K | DPL_3);
	
	/* 仅初始化必要的寄存器 */
	p_proc->regs.gs = SELECTOR_VIDEO;
	p_proc->regs.fs = SELECTOR_LDT_FLATRW;
	p_proc->regs.es = SELECTOR_LDT_FLATRW;
	p_proc->regs.ds = SELECTOR_LDT_FLATRW;
	p_proc->regs.eip = (u32) TaskA;
	p_proc->regs.cs = SELECTOR_LDT_FLATC;
	p_proc->regs.eflags = 0x3202; /* IOPL = 3, IF = 1, bit 2 is always 1 */
	p_proc->regs.esp = (u32) task_stack + TASK_STACK_SIZE;
	p_proc->regs.ss = SELECTOR_LDT_FLATRW;
	
	p_current_proc = p_proc;
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
		print("A.");
		delay(10);
	}
}
