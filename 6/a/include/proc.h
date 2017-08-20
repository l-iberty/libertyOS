/**
 * proc.h 进程相关
 */

#ifndef PROC_H
#define PROC_H

#include "type.h"
#include "sysconst.h"

#define TASK_STACK_SIZE		2*1024
#define MAX_MSGSIZE		128

u8	task_stack_a[TASK_STACK_SIZE];
u8	task_stack_b[TASK_STACK_SIZE];
u8	task_stack_c[TASK_STACK_SIZE];
u8	task_stack_tty[TASK_STACK_SIZE];
u8	task_stack_hd[TASK_STACK_SIZE];

typedef struct {
	u32	gs;	/* \							*/
	u32	fs;	/* |							*/
	u32	es;	/* | pushed manually.					*/
	u32	ds;	/* /							*/
	u32	edi;	/* \							*/
	u32	esi;	/* |							*/
	u32	ebp;	/* |							*/
	u32	k_esp;	/* | pushed by `pushad`.				*/
	u32	ebx;	/* |							*/
	u32	edx;	/* |							*/
	u32	ecx;	/* |							*/
	u32	eax;	/* /							*/
	u32	eip;	/* \							*/
	u32	cs;	/* |							*/
	u32	eflags;	/* | pushed by processor when interruption happens.	*/
	u32	esp;	/* |							*/
	u32	ss;	/* /							*/

} STACK_FRAME;

typedef struct {
	int	src_pid;
	int	dst_pid;
	int	value;
} MESSAGE;

typedef struct {
	MESSAGE*	p_head;
	MESSAGE*	p_tail;
	int		count;
	MESSAGE		msg_buf[MAX_MSGSIZE];
} MESSAGE_QUEUE;

typedef struct {
	STACK_FRAME	regs;
	u16		ldt_selector;
	u8		LDT[LDT_DESC_NUM * DESC_SIZE];
	u32		pid;
	int		ticks;
	int		priority;
	MESSAGE_QUEUE	msg_queue;
} PROCESS;

typedef struct {
	fpPROC		task_entry;
	u8*		task_stack;
} TASK;

PROCESS		proc_table[NR_PROCS + NR_TASKS];

PROCESS*	p_current_proc;


void TaskA();
void TaskB();
void TaskC();
void Task_tty();
void Task_hd();

int sys_get_ticks();
int sys_sendrecv(int func_type, int pid, MESSAGE* p_msg);
int msg_send(PROCESS* p_proc, MESSAGE* p_msg);
int msg_recv(MESSAGE* p_msg);
void reset_msg(MESSAGE* p_msg);
void init_message_queue(MESSAGE_QUEUE* p_msg_queue);

#endif /* PROC_H */
