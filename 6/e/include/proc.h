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
u8	task_stack_fs[TASK_STACK_SIZE];

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

struct mess1 {
	int	i1;
	int	i2;
	int	i3;
	int	i4;
	int	i5;
};

struct mess2 {
	void*	v1;
	void*	v2;
	void*	v3;
	void*	v4;
	void*	v5;
};

typedef struct {
	int	src_pid;
	int	dst_pid;
	int	value;
	union {
		struct mess1 m1;
		struct mess2 m2;
	} u;
} MESSAGE;

/* definition for Task_hd & Task_fs */
#define DEVICE	u.m1.i1
#define SECTOR	u.m1.i2
#define LEN	u.m1.i3
#define BUF	u.m2.v1

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

/* pid */
#define PID_TASK_A	0
#define	PID_TASK_B	1
#define PID_TASK_C	2
#define PID_TASK_HD	3
#define PID_TASK_FS	4
#define PID_TASK_TTY	5

/* message value */
#define DEV_OPEN	1001
#define DEV_READ	1002
#define DEV_WRITE	1003
#define HD_INT_DONE	2001

void TaskA();
void TaskB();
void TaskC();
void Task_hd();
void Task_fs();
void Task_tty();

int sys_get_ticks();
int sys_sendrecv(int func_type, int pid, MESSAGE* p_msg);

u32 va2la(PROCESS* proc, void* va);

int msg_send(PROCESS* p_proc, MESSAGE* p_msg);
int msg_recv(MESSAGE* p_msg);
void reset_msg(MESSAGE* p_msg);
void init_message_queue(MESSAGE_QUEUE* p_msg_queue);
int isEmpty_msg_queue(PROCESS* p_proc);

#endif /* PROC_H */
