#include "type.h"
#include "proc.h"
#include "sysconst.h"
#include "protect.h"
#include "keyboard.h"
#include "tty.h"

#define	MAX_PRIORITY 0xFFFFFFFF

int f_reenter;
int is_current_proc_done;

void proc_begin();	/* lib/klib.asm */
void println(char *sz);	/* lib/klib.asm */
void print(char *sz);	/* lib/klib.asm */
void _memcpy(void* pDst, void* pSrc, int len);
void _memset(void* pDst, u8 value, int len);
void itoa(char* str, int v, int len, u8 flag);	/* lib/klib.asm */
u32 _strlen(char* s);				/* lib/string.asm */
void _strcpy(char* dst, const char* src);	/* lib/string.asm */
void _printf(const char* fmt, ...);
void init_msg_queue(MESSAGE_QUEUE* p_msg_queue);
int isEmpty_msg_queue(PROCESS* p_proc);

int get_ticks(); /* system call */
int sendrecv(int func_type, int pid, MESSAGE* p_msg); /* system call */

extern u8	GDT[GDT_DESC_NUM * DESC_SIZE];	/* kernel/kernel.asm */

extern int ticks;

TASK task_table[NR_PROCS + NR_TASKS] = {{ TaskA, task_stack_a },
					{ TaskB, task_stack_b },
					{ TaskC, task_stack_c },
					{ Task_hd, task_stack_hd },
					{ Task_fs, task_stack_fs },
					{ Task_tty, task_stack_tty }};
		   	          
SYSCALL syscall_table[NR_SYSCALL] = { sys_get_ticks,
				      sys_sendrecv };

void kernel_main()
{
	println("------------kernel_main------------");
	
	f_reenter = 0;
	is_current_proc_done = 0;
	
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
		
		/* PID */
		p_proc->pid = i;
		
		/* msg_queue */
		init_msg_queue(&p_proc->msg_queue);
	}
	
	proc_table[0].ticks = proc_table[0].priority = 1;
	proc_table[1].ticks = proc_table[1].priority = 1;
	proc_table[2].ticks = proc_table[2].priority = 1;
	proc_table[3].ticks = proc_table[3].priority = 1;
	proc_table[4].ticks = proc_table[4].priority = 1;
	proc_table[5].ticks = proc_table[5].priority = 1;
	
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
	is_current_proc_done = 0;

	int var;
	_printf("virtual addr of `var` is 0x%.8x\nlinear addr of `var` is 0x%.8x",
		&var, va2la(p_current_proc, &var));

	//is_current_proc_done = 1;	// 通知时钟中断例程:
	while(1) {}			// 进程结束, 可以切换
	
	_printf("-----TaskA-----");
	
	MESSAGE msg;
	
	/* <1> To: TaskC */
	msg.value = 0xAABB;
	if (!sendrecv(SEND, 2, &msg))
	{
		_printf("Sending message: src_pid = %.4x, dst_pid = %.4x, value = %.8x",
			msg.src_pid, msg.dst_pid, msg.value);
	}
	else
	{
		_printf("error: sendrecv [pid: %.4x]", p_current_proc->pid);
	}
	
	reset_msg(&msg);
	
	/* <2> To: TaskC */
	msg.value = 0xCCDD;
	if (!sendrecv(SEND, 2, &msg))
	{
		_printf("Sending message: src_pid = %.4x, dst_pid = %.4x, value = %.8x",
			msg.src_pid, msg.dst_pid, msg.value);
	}
	else
	{
		_printf("error: sendrecv [pid: %.4x]", p_current_proc->pid);
	}
	
	is_current_proc_done = 1;
	while(1) {}
}

void TaskB()
{
	//is_current_proc_done = 0;
	is_current_proc_done = 1;	// 通知时钟中断例程:
	while(1) {}			// 进程结束, 可以切换
	
	_printf("-----TaskB-----");
	
	MESSAGE msg;
	
	/* <1> To: TaskC */
	msg.value = 0x12345678;
	if (!sendrecv(SEND, 2, &msg))
	{
		_printf("Sending message: src_pid = %.4x, dst_pid = %.4x, value = %.8x",
			msg.src_pid, msg.dst_pid, msg.value);
	}
	else
	{
		_printf("error: sendrecv [pid: %.4x]", p_current_proc->pid);
	}
	
	reset_msg(&msg);

	/* <2> To: TaskC */
	msg.value = 0xFFEECCDD;
	if (!sendrecv(SEND, 2, &msg))
	{
		_printf("Sending message: src_pid = %.4x, dst_pid = %.4x, value = %.8x",
			msg.src_pid, msg.dst_pid, msg.value);
	}
	else
	{
		_printf("error: sendrecv [pid: %.4x]", p_current_proc->pid);
	}
	
	is_current_proc_done = 1;
	while(1) {}
}

void TaskC()
{
	//is_current_proc_done = 0;
	is_current_proc_done = 1;	// 通知时钟中断例程:
	while(1) {}			// 进程结束, 可以切换
	
	_printf("-----TaskC-----");
	
	MESSAGE msg;
	
	while (1)
	{
		if (!isEmpty_msg_queue(p_current_proc))
		{
			if (!sendrecv(RECEIVE, 1, &msg))
			{
				_printf("Reveiving message: src_pid = %.4x, dst_pid = %.4x, value = %.8x",
					msg.src_pid, msg.dst_pid, msg.value);
			}
			else
			{
				_printf("error: sendrecv [pid: %.4x]", p_current_proc->pid);
			}
		}
		else
		{
			_printf("[pid: %.4x] No messages to process!", p_current_proc->pid);
			break;
		}
	}
	
	is_current_proc_done = 1;
	while(1) {}
}

int sys_get_ticks()
{
	print("^");
	return ticks;
}

int sys_sendrecv(int func_type, int pid, MESSAGE* p_msg)
{
	PROCESS* p_proc = proc_table + pid;
	int ret = 1; /* assume failed */
	
	if (func_type == SEND)
	{
		/* todo SEND */
		ret = msg_send(p_proc, p_msg);
		
	}
	else if (func_type == RECEIVE)
	{
		/* todo RECEIVE */
		ret = msg_recv(p_msg);
	}
	
	return ret;
}

/* 由 virtual address 求 linear address */
/* selector:offset 形式的地址称为 logical address, */
/* 其中的`offset`称为 virtual address. */
u32 va2la(PROCESS* proc, void* va)
{
	u8* p_desc = &proc->LDT[1 * DESC_SIZE]; /* Data segment descriptor */
	u32 base_low = (u32) (*(u16*) (p_desc + 2));
	u32 base_mid = (u32) *(p_desc + 4);
	u32 base_high = (u32) *(p_desc + 7);
	
	u32 base = (base_high << 24) | (base_mid << 16) | (base_low);
	u32 la = base + (u32) va; /* linear address */
	
	return la;
}

/**
 * @param p_proc 接收方
 * @param p_msg  发送给接受方的消息
 *
 * @return Zero if successful
 */
int msg_send(PROCESS* p_proc, MESSAGE* p_msg)
{
	int ret = 1; /* assume failed */
	
	p_msg->src_pid = p_current_proc->pid;	/* 发送方是当前进程(调用者进程) */
	p_msg->dst_pid = p_proc->pid;		/* 接收方是 p_proc 指向的进程 */
	
	if (p_proc->msg_queue.count < MAX_MSGSIZE)
	{
		_memcpy(p_proc->msg_queue.p_tail++, p_msg, sizeof(MESSAGE));
		p_proc->msg_queue.count++;
		
		if (p_proc->msg_queue.p_tail >= p_proc->msg_queue.msg_buf + MAX_MSGSIZE)
		{
			p_proc->msg_queue.p_tail = p_proc->msg_queue.msg_buf;
		}
		
		ret = 0;
	}
	
	return ret;
}

/**
 * @param p_msg  接收来自发送方的消息
 *
 * @return Zero if successful
 */
int msg_recv(MESSAGE* p_msg)
{
	MESSAGE_QUEUE* 	p_src_msg_queue = &p_current_proc->msg_queue;
	
	int ret = 1; /* assume failed */
	
	if (p_src_msg_queue->count > 0)
	{
		_memcpy(p_msg, p_src_msg_queue->p_head++, sizeof(MESSAGE));
		p_src_msg_queue->count--;
		
		if (p_src_msg_queue->p_head >= p_src_msg_queue->msg_buf + MAX_MSGSIZE)
		{
			p_src_msg_queue->p_head = p_src_msg_queue->msg_buf;
		}
		
		ret = 0;
	}
	
	return ret;
}

void reset_msg(MESSAGE* p_msg)
{
	_memset(p_msg, 0, sizeof(MESSAGE));
}

void init_msg_queue(MESSAGE_QUEUE* p_msg_queue)
{
	p_msg_queue->count = 0;
	p_msg_queue->p_head = p_msg_queue->msg_buf;
	p_msg_queue->p_tail = p_msg_queue->msg_buf;
}

int isEmpty_msg_queue(PROCESS* p_proc)
{
	return (p_proc->msg_queue.count == 0);
}

