#include "proc.h"
#include "protect.h"
#include "stdio.h"
#include "stdlib.h"
#include "global.h"

static int msg_send(uint32_t pid_sender, uint32_t pid_receiver, struct message* p_msg);
static int msg_recv(uint32_t pid_sender, uint32_t pid_receiver, struct message* p_msg);
static void block(struct proc* p_proc);
static void unblock(struct proc* p_proc);
static int deadlock(int src, int dst);
static void reset_msg(struct message* p_msg);
static void enqueue(struct proc_queue* queue, struct proc* p_proc);
static struct proc* dequeue(struct proc_queue* queue);
static int empty(struct proc_queue* queue);


uint8_t	task_stack_init[TASK_STACK_SIZE];
uint8_t	task_stack_a[TASK_STACK_SIZE];
uint8_t	task_stack_b[TASK_STACK_SIZE];
uint8_t	task_stack_c[TASK_STACK_SIZE];
uint8_t	task_stack_tty[TASK_STACK_SIZE];
uint8_t	task_stack_hd[TASK_STACK_SIZE];
uint8_t	task_stack_fs[TASK_STACK_SIZE];
uint8_t	task_stack_mm[TASK_STACK_SIZE];
uint8_t	task_stack_exe[TASK_STACK_SIZE];

struct proc	proc_table[NR_PROCS];
struct task	task_table[NR_PROCS];
SYSCALL 	syscall_table[NR_SYSCALL];

struct task  task_table[NR_PROCS] = {{ Init, task_stack_init },
                                     { TaskA, task_stack_a },
                                     { TaskB, task_stack_b },
                                     { TaskC, task_stack_c },
                                     { TaskTTY, task_stack_tty },
                                     { TaskHD, task_stack_hd },
                                     { TaskFS, task_stack_fs },
                                     { TaskMM, task_stack_mm },
                                     { TaskEXE, task_stack_exe} };
		   	          
SYSCALL syscall_table[NR_SYSCALL] = { sys_get_ticks,
                                      sys_sendrecv,
                                      sys_getpid,
                                      sys_getppid,
                                      sys_printk,
                                      sys_sem_init,
                                      sys_sem_post,
                                      sys_sem_wait,
                                      sys_disable_paging,
                                      sys_enable_paging,
                                      sys_reload_cr3,
                                      sys_getcr3 };

/**
 * see `sys_disable_paging', `sys_enable_paging',
 * `sys_reload_cr3', `sys_getcr3' in lib/klib.asm
 */

/***************************************************************
 *			syscall routine			       *
 **************************************************************/

int sys_get_ticks()
{
	return ticks;
}

int sys_sendrecv(int func_type, int pid, struct message* p_msg)
{
	int ret = 1; /* assume failed */
	int current_pid = p_current_proc->pid;
	
	switch (func_type)
	{
		case BOTH:
		{
			ret = msg_send(current_pid, pid, p_msg);
			if (ret == 0)
			{
				ret = msg_recv(pid, current_pid, p_msg);
			}
			break;
		}
		case SEND:
		{
		    	ret = msg_send(current_pid, pid, p_msg);
		    	break;
		}
		case RECEIVE:
		{
			ret = msg_recv(pid, current_pid, p_msg);
		    	break;
		}
	}
	return ret;
}

uint32_t sys_getpid()
{
	return p_current_proc->pid;
}

uint32_t sys_getppid()
{
	return p_current_proc->pid_parent;
}

void sys_printk(const char* sz)
{
	print((char*) va2la(p_current_proc, (void*) sz));
	set_cursor_pos(MainPrintPos >> 1);
}

int sys_sem_init(struct semaphore* p_sem, int value)
{
	if (p_sem == NULL)
		return 1;
	
	p_sem->value = value;
	
	p_sem->wait_queue.count = 0;
	p_sem->wait_queue.p_head = NULL;
	p_sem->wait_queue.p_tail = NULL;
	
	return 0;
}

int sys_sem_post(struct semaphore* p_sem)
/**
 *	V(S)
 *	{
 *		S++;
 *		if (S <= 0) 唤醒信号量S的等待队列中的第一个进程
 *		else 继续运行
 *	}
 */
{
	struct proc* p_proc;
	
	if (p_sem == NULL)
		return 1;
	
	disable_int();
	
	p_sem->value++;
	
	if (p_sem->value <= 0)
	{
		p_proc = dequeue(&p_sem->wait_queue);
		p_proc->flag &= (~WAITING);
		unblock(p_proc);
	}
	
	enable_int();
	
	return 0;
}

int sys_sem_wait(struct semaphore* p_sem)
/**
 *	P(S)
 *	{
 *		S--;
 *		if (S < 0) 挂起当前进程, 将其添加到信号量S的等待队列中
 *		else 继续运行
 *	}
 */
{
	if (p_sem == NULL)
		return 1;
	
	disable_int();

	p_sem->value--;
	
	if (p_sem->value < 0)
	{
		p_current_proc->flag |= WAITING;
		enqueue(&p_sem->wait_queue, p_current_proc);
		block(p_current_proc);
	}
	
	enable_int();
	
	return 0;
}


/***************************************************************/


/**
 * 调度算法:
 * <1> 将当前进程(被中断的进程)的 ticks 减 1;
 * <2> 遍历进程表, 找到 flag = 0 且 ticks 最大的进程 p,
 *     作为即将被启动或唤醒的进程;
 * <3> 如果所有进程的 ticks 都已被减至 0, 则恢复为初值.
 *     (ticks 的初值由 priority 保存)
 *
 * 在该调度算法下, 进程的运行时间比接近 priority 之比
 */
void schedule()
{
	struct proc* p_proc;
	int max_ticks = 0;
	
	for (p_proc = &FIRST_PROC; p_proc <= &LAST_PROC; p_proc++) 
	{
		if (p_proc->flag == 0) 
		{
			if (p_proc->ticks > max_ticks) 
			{
				max_ticks = p_proc->ticks;
				p_current_proc = p_proc;
			}
		}
	}
	
	if (max_ticks == 0) 
	{
		for (p_proc = &FIRST_PROC; p_proc <= &LAST_PROC; p_proc++)
			p_proc->ticks = p_proc->priority;
	}
}


/**
 * 如果进程 pid 在等待硬件中断, inform_int() 会通知 pid: 硬件中断已到达.
 */
void inform_int(int pid)
{
	struct proc* p_proc = proc_table + pid;
	
	if ((p_proc->flag & RECEIVING) && (p_proc->pid_recvfrom == INTERRUPT)) 
	{
		p_proc->p_msg->source = INTERRUPT;
		p_proc->p_msg->dest = pid;
		p_proc->p_msg->value = HARD_INT;
		p_proc->p_msg = NULL;
		p_proc->has_int_msg = 0;
		p_proc->pid_recvfrom = NONE;
		p_proc->flag &= (~RECEIVING);
		unblock(p_proc);
	} 
	else 
	{
		p_proc->has_int_msg = 1;
	}
}

void interrupt_wait()
{
	struct message msg;
	sendrecv(RECEIVE, INTERRUPT, &msg);
}

void init_send_queue(struct proc* p_proc)
{
	p_proc->send_queue.count = 0;
	p_proc->send_queue.p_head = p_proc->send_queue.p_tail = p_proc->send_queue.procs;
}

/* 由 virtual address 求 linear address */
/* selector:offset 形式的地址称为 logical address, */
/* 其中的`offset`称为 virtual address. */
void* va2la(struct proc* proc, void* va)
{
	uint8_t* p_desc = &proc->LDT[INDEX_LDT_RW * DESC_SIZE]; /* Data segment descriptor */

	uint32_t base = get_base(p_desc);
	uint32_t la = base + (uint32_t) va; /* linear address */
	
	return (void*) la;
}

void dump_msg(struct message* p_msg)
{
	printf("\n{msg>> src=%.2x, dst=%.2x, value=%.8x}\n",
		p_msg->source, p_msg->dest, p_msg->value);
}

void failure(char* exp, char* file, char* base_file, int line)
{
	char buf[256] = { 0 };
	sprintf(buf, "{exp: %s, file: %s, base_file: %s, line: %d}",
		exp, file, base_file, line);
	/* 蓝底黄字, 1 行 0 列 */
	printmsg(buf, 0x1E, (80*1+0)*2);
	//disable_int(); // 若禁止用户进程(ring3)的IO权限，该调用会失败
	for (;;);
}

void panic(const char* fmt, ...)
{
	char buf[256] = { 0 };
	va_list arg = (va_list) ((char *) &fmt + 4);	/* 4 是参数 fmt 所占堆栈的大小 */
	
	vsprintf(buf, fmt, arg);
	/* 蓝底黄字, 0 行 0 列 */
	printmsg(buf, 0x1E, (80*0+0)*2);
	//disable_int(); // 若禁止用户进程(ring3)的IO权限，该调用会失败
	for (;;);
}


static int msg_send(uint32_t pid_sender, uint32_t pid_receiver, struct message* p_msg)
{
	struct proc* sender = proc_table + pid_sender;
	struct proc* receiver = proc_table + pid_receiver;

	while (deadlock(pid_sender, pid_receiver)) { } /* 等待死锁解除 */
	
	p_msg->source = pid_sender;
	p_msg->dest = pid_receiver;
	
	if (receiver->flag & RECEIVING) 
	{
		/* 将消息复制给 receiver 并将其取消阻塞 */
		assert(receiver->p_msg);
		memcpy(va2la(receiver, receiver->p_msg),
		       va2la(sender, p_msg),
		       sizeof(struct message));
		receiver->flag &= (~RECEIVING);
		unblock(receiver);
	} 
	else 
	{
		/* receiver 未准备接收消息, 将 sender 加入 receiver 的发送队列并阻塞之 */
		sender->flag |= SENDING;
		sender->p_msg = p_msg;
		sender->pid_sendto = pid_receiver;
		sender->pid_recvfrom = NONE;
		enqueue(&receiver->send_queue, sender);
		block(sender);
	}
	
	return 0;
}

static int msg_recv(uint32_t pid_sender, uint32_t pid_receiver, struct message* p_msg)
{
	struct proc* sender = 0;
	struct proc* receiver = proc_table + pid_receiver;
	int flag = 0;
	
	receiver->pid_sendto = NONE;
	receiver->pid_recvfrom = NONE;
	
	if (receiver->has_int_msg && (pid_sender == INTERRUPT)) 
	{
		struct message msg;
		reset_msg(&msg);
		msg.source = INTERRUPT;
		msg.dest = pid_receiver;
		msg.value = HARD_INT;
		
		memcpy(va2la(receiver, p_msg), &msg, sizeof(struct message));
		
		receiver->has_int_msg = 0;
		return 0;
	}
	
	if (pid_sender == ANY) 
	{
		if (!empty(&receiver->send_queue)) 
		{
			/* 从 receiver 的发送队列里取第一个 */
			sender = dequeue(&receiver->send_queue);
			if ((sender >= &FIRST_PROC) && (sender <= &LAST_PROC)) 
			{
				flag = 1;
			}
		}
	} 
	else if ((pid_sender >= 0) && (pid_sender < NR_PROCS)) 
	{
		sender = proc_table + pid_sender;
		/* 判断 sender 是否在向 receiver 发消息 */
		if ((sender->flag & SENDING) &&
		    (sender->pid_sendto == pid_receiver)) 
		{
		    	flag = 1;
		}
	}
	
	if (flag) 
	{
		/* 接收 sender 的消息, 并将 sender 取消阻塞 */
		assert(p_msg);
		assert(sender->p_msg);
		
		memcpy(va2la(receiver, p_msg),
		       va2la(sender, sender->p_msg),
		       sizeof(struct message));
		sender->flag &= (~SENDING);
		unblock(sender);
	}
	else
	{
		/* 没有进程向 receiver 发消息, 将 receiver 阻塞 */
		receiver->flag |= RECEIVING;
		receiver->pid_recvfrom = pid_sender;
		receiver->pid_sendto = NONE;
		receiver->p_msg = p_msg;
		block(receiver);
	}
	return 0;
}

/**
 * 当进程 p_proc 的 flag 不为 0 时, schedule() 便不再让 p_proc 获得 CPU,
 * p_proc 便被"阻塞"了. 若要阻塞 p_proc, 需确保 p_proc->flag != 0, 再
 * 调用 schedule() 调度进程.
 */
static void block(struct proc* p_proc)
{
	assert(p_proc->flag);
	schedule();
}

/**
 * unblock 不做实质性工作, p_proc 是否被阻塞取决与 p_proc->flag 是否为 0,
 * 若 p_proc->flag == 0 则 p_proc 已被取消阻塞, unblock 仅检查之.
 */
static void unblock(struct proc* p_proc)
{
	//assert(p_proc->flag == 0);
}

/**
 * src 如果发消息给 dst 会不会导致死锁?
 * @return 0:不会, 1:会
 */
static int deadlock(int src, int dst)
{
	struct proc* p_proc = proc_table + dst;
	int flag = 1;
	
	for (;;) 
	{
		if (p_proc->flag & SENDING) 
		{
			if (p_proc->pid_sendto == src) 
			{
				flag = 1;
				break;
			}
		} 
		else 
		{
			flag = 0;
			break;
		}
		
		if (p_proc->pid_sendto < 0 || p_proc->pid_sendto >= (NR_PROCS)) 
		{
			flag = 0;
			break;
		} 
		else
		{
			p_proc = proc_table + p_proc->pid_sendto;
		}
	}
	return flag;
}

static void reset_msg(struct message* p_msg)
{
	memset(p_msg, 0, sizeof(struct message));
}

static void enqueue(struct proc_queue* queue, struct proc* p_proc)
{
	if (queue->count < NR_PROCS)
	{
		*queue->p_tail++ = p_proc;
		queue->count++;
		
		assert(queue->count <= NR_PROCS);
		
		if (queue->p_tail > &queue->procs[NR_PROCS - 1])
		{
			queue->p_tail = queue->procs;
		}
	}
}

static struct proc* dequeue(struct proc_queue* queue)
{
	struct proc* p_proc = NULL;
	
	if (queue->count > 0)
	{
		p_proc = *queue->p_head++;
		queue->count--;
		
		assert(queue->count >= 0);
		
		if (queue->p_head > &queue->procs[NR_PROCS - 1])
		{
			queue->p_head = queue->procs;
		}
	}
	return p_proc;
}

static int empty(struct proc_queue* queue)
{
	return (queue->count == 0);
}
