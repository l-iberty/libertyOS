#include "proc.h"
#include "stdio.h"
#include "stdlib.h"
#include "global.h"


/***************************************************************
 *			syscall routine			       *
 **************************************************************/

int sys_get_ticks()
{
	_printf("^");
	return ticks;
}

int sys_sendrecv(int func_type, int pid, MESSAGE* p_msg)
{
	int ret = 1; /* assume failed */
	
	switch (func_type)
	{
		case BOTH:
		{
			ret = msg_send(p_current_proc->pid, pid, p_msg);
			if (ret == 0)
				ret = msg_recv(pid, p_current_proc->pid, p_msg);
			break;
		}
		case SEND:
		{
		    ret = msg_send(p_current_proc->pid, pid, p_msg);
		    break;
		}
        case RECEIVE:
        {
		    ret = msg_recv(pid, p_current_proc->pid, p_msg);
            break;
        }
	}
	
	return ret;
}

/***************************************************************/

/* 由 virtual address 求 linear address */
/* selector:offset 形式的地址称为 logical address, */
/* 其中的`offset`称为 virtual address. */
void* va2la(PROCESS* proc, void* va)
{
	u8* p_desc = &proc->LDT[1 * DESC_SIZE]; /* Data segment descriptor */
	u32 base_low = (u32) (*(u16*) (p_desc + 2));
	u32 base_mid = (u32) *(p_desc + 4);
	u32 base_high = (u32) *(p_desc + 7);
	
	u32 base = (base_high << 24) | (base_mid << 16) | (base_low);
	u32 la = base + (u32) va; /* linear address */
	
	return (void*) la;
}

int msg_send(u32 pid_sender, u32 pid_receiver, MESSAGE* p_msg)
{
	PROCESS* sender = proc_table + pid_sender;
	PROCESS* receiver = proc_table + pid_receiver;
	
	if (deadlock(pid_sender, pid_receiver))
	{
		halt("\n>>DEADLOCK<< {pid: 0x%.4x->0x%.4x}\n", pid_sender, pid_receiver);
	}
	
	p_msg->src_pid = pid_sender;
	p_msg->dst_pid = pid_receiver;
	
	if (receiver->flag & RECEIVING)
	{
		/* 将消息复制给 receiver 并将其取消阻塞 */
		assert(receiver->p_msg);
		_memcpy(va2la(receiver, receiver->p_msg),
				va2la(sender, p_msg),
				sizeof(MESSAGE));
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
		enqueue_send(receiver, sender);
		block(sender);
	}
	
	return 0;
}

int msg_recv(u32 pid_sender, u32 pid_receiver, MESSAGE* p_msg)
{
	PROCESS* sender = 0;
	PROCESS* receiver = proc_table + pid_receiver;
	int bOk = 0;
	
	receiver->pid_sendto = NONE;
	receiver->pid_recvfrom = NONE;
	
	if (receiver->has_int_msg && (pid_sender == INTERRUPT))
	{
		MESSAGE msg;
		reset_msg(&msg);
		msg.src_pid = INTERRUPT;
		msg.dst_pid = pid_receiver;
		msg.value = HARD_INT;
		
		_memcpy(va2la(receiver, p_msg), &msg, sizeof(MESSAGE));
		
		receiver->has_int_msg = 0;
		return 0;
	}
	
	if (pid_sender == ANY)
	{
		if (!isEmpty(&receiver->send_queue))
		{
			/* 从 receiver 的发送队列里取第一个 */
			sender = dequeue_send(receiver);
			if ((sender >= &FIRST_PROC) && (sender <= &LAST_PROC))
			{
				bOk = 1;
			}
		}
	}
	else if ((pid_sender >= 0) && (pid_sender < NR_PROCS + NR_TASKS))
	{
		sender = proc_table + pid_sender;
		/* 判断 sender 是否在向 receiver 发消息 */
		if ((sender->flag & SENDING) &&
		    (sender->pid_sendto == pid_receiver))
		{
		    	bOk = 1;
		}
	}
	
	if (bOk)
	{
		/* 接收 sender 的消息, 并将 sender 取消阻塞 */
		assert(p_msg);
		assert(sender->p_msg);
		
		_memcpy(va2la(receiver, p_msg),
				va2la(sender, sender->p_msg),
				sizeof(MESSAGE));
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
void block(PROCESS* p_proc)
{
	assert(p_proc->flag);
	schedule();
}

/**
 * unblock 不做实质性工作, p_proc 是否被阻塞取决与 p_proc->flag 是否为 0,
 * 若 p_proc->flag == 0 则 p_proc 已被取消阻塞, unblock 仅检查之.
 */
void unblock(PROCESS* p_proc)
{
	assert(p_proc->flag == 0);
}

/**
 * src 如果发消息给 dst 会不会导致死锁?
 * @return 0:不会, 1:会
 */
int deadlock(int src, int dst)
{
	PROCESS* p_proc = proc_table + dst;
	int isDeadLock = 1;
	
	for (;;)
	{
		if (p_proc->flag & SENDING) {
			if (p_proc->pid_sendto == src) {
				isDeadLock = 1;
				break;
			}
		} else {
			isDeadLock = 0;
			break;
		}
		
		if (p_proc->pid_sendto < 0 || p_proc->pid_sendto >= (NR_PROCS + NR_TASKS)) {
			isDeadLock = 0;
			break;
		} else {
			p_proc = proc_table + p_proc->pid_sendto;
		}
	}
	return isDeadLock;
}

/**
 * 如果进程 pid 在等待硬件中断, inform_int() 会通知 pid: 硬件中断已到达.
 */
void inform_int(int pid)
{
	PROCESS* p_proc = proc_table + pid;
	
	if ((p_proc->flag & RECEIVING) && (p_proc->pid_recvfrom == INTERRUPT))
	{
		p_proc->p_msg->src_pid = INTERRUPT;
		p_proc->p_msg->dst_pid = pid;
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

/**
 * 等待硬件中断的到来
 */
void interrupt_wait()
{
	MESSAGE msg;
	sendrecv(RECEIVE, INTERRUPT, &msg);
}

void reset_msg(MESSAGE* p_msg)
{
	_memset(p_msg, 0, sizeof(MESSAGE));
}

void init_send_queue(PROCESS* p_proc)
{
	SEND_QUEUE* p_send_queue = &p_proc->send_queue;
	
	p_send_queue->count = 0;
	p_send_queue->p_head = p_send_queue->p_tail = p_send_queue->proc_queue;
}

/**
 * 将 p_proc 放进 p 的 send_queue.
 */
void enqueue_send(PROCESS* p, PROCESS* p_proc)
{
	if (p->send_queue.count < NR_PROCS + NR_TASKS)
	{
		*p->send_queue.p_tail++ = p_proc;
		p->send_queue.count++;
		
		assert(p->send_queue.count <= (NR_PROCS + NR_TASKS));
		
		if (p->send_queue.p_tail > &p->send_queue.proc_queue[NR_PROCS + NR_TASKS - 1])
		{
			p->send_queue.p_tail = p->send_queue.proc_queue;
		}
	}
}

/**
 * 从 p 的 send_queue 里取出.
 */
PROCESS* dequeue_send(PROCESS* p)
{
	PROCESS* p_proc = NULL;
	
	if (p->send_queue.count > 0)
	{
		p_proc = *p->send_queue.p_head++;
		p->send_queue.count--;
		
		assert(p->send_queue.count >= 0);
		
		if (p->send_queue.p_head > &p->send_queue.proc_queue[NR_PROCS + NR_TASKS - 1])
		{
			p->send_queue.p_head = p->send_queue.proc_queue;
		}
	}
	return p_proc;
}

int isEmpty(SEND_QUEUE* queue)
{
	return (queue->count == 0);
}

void dump_proc()
{
	PROCESS* p;
	for (p = &FIRST_PROC; p <= &LAST_PROC; p++)
	{
		_printf("\n[pid:%.2x,ticks:%.2x,flag:%.8x,pid_sendto:%.8x,pid_recvfrom:%.8x]",
			p->pid, p->ticks, p->flag, p->pid_sendto, p->pid_recvfrom);
	}
}

void dump_msg(MESSAGE* p_msg)
{
	_printf("\n{msg>> src=%.2x, dst=%.2x, value=%.8x}\n",
		p_msg->src_pid, p_msg->dst_pid, p_msg->value);
}

void failure(char* exp, char* file, char* base_file, int line)
{
	char buf[256] = { 0 };
	_sprintf(buf, "{exp: %s, file: %s, base_file: %s, line: 0x%.4x}",
		exp, file, base_file, line);
	/* 0x1E 蓝底黄字 */
	/* (80*4+0)*2: 4 行 2 列 */
	printmsg(buf, 0x1E, (80*4+0)*2);
	disable_int();
	for (;;);
}

void halt(const char* fmt, ...)
{
	char buf[256] = { 0 };
	va_list arg = (va_list) ((char *) &fmt + 4);	/* 4 是参数 fmt 所占堆栈的大小 */
	
	_vsprintf(buf, fmt, arg);
	/* 0x1E 蓝底黄字 */
	/* (80*3+0)*2: 3 行 2 列 */
	printmsg(buf, 0x1E, (80*3+0)*2);
	disable_int();
	for (;;);
}

