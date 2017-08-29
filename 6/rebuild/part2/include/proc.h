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
	int	m1i1;
	int	m1i2;
	int	m1i3;
	int	m1i4;
	int	m1i5;
};

struct mess2 {
	void*	m2p1;
	void*	m2p2;
	void*	m2p3;
	void*	m2p4;
	void*	m2p5;
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


/* definition for message */
#define VALUE	u.m1.m1i1
#define DEVICE	u.m1.m1i1
#define SECTOR	u.m1.m1i2
#define LEN	u.m1.m1i3
#define BUF	u.m2.m2p1

typedef struct S_SEND_QUEUE	SEND_QUEUE;
typedef struct S_PROCESS	PROCESS;

struct S_SEND_QUEUE {
	int		count;
	PROCESS**	p_head;
	PROCESS** 	p_tail;
	PROCESS*	proc_queue[NR_PROCS + NR_TASKS];
};

struct S_PROCESS {
	STACK_FRAME	regs;
	u16		ldt_selector;
	u8		LDT[LDT_DESC_NUM * DESC_SIZE];
	u32		pid;
	int		ticks;
	int		priority;
	
	MESSAGE*	p_msg;
	int		flag;
	u32		pid_recvfrom;	/* Whom does the process wanna receive message from? */
	u32		pid_sendto;	/* Whom does the process wanna send message to? */
	SEND_QUEUE	send_queue;
	
};

typedef struct {
	fpPROC		task_entry;
	u8*		task_stack;
} TASK;

PROCESS		proc_table[NR_PROCS + NR_TASKS];

#define FIRST_PROC proc_table[0]
//#define LAST_PROC proc_table[NR_PROCS + NR_TASKS - 1]
#define LAST_PROC proc_table[NR_PROCS]

PROCESS*	p_current_proc;

/* pid */
#define PID_TASK_A	0
#define	PID_TASK_B	1
#define PID_TASK_C	2
#define PID_TASK_HD	3
#define PID_TASK_FS	4
#define PID_TASK_TTY	5

/* flag */
/* SENDING 和 RECEVING 的二进制串不能有重叠的 "1" */
#define SENDING		1
#define RECEIVING	2 

/* pid_recvfrom & pid_sendto */
#define	ANY		(u32) (-1)
#define NONE		(u32) (-2)

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

u32	va2la(PROCESS* proc, void* va);
int	msg_send(u32 pid_sender, u32 pid_receiver, MESSAGE* p_msg);
int	msg_recv(u32 pid_sender, u32 pid_receiver, MESSAGE* p_msg);
void	block(PROCESS* p_proc);
void	unblock(PROCESS* p_proc);
void	reset_msg(MESSAGE* p_msg);
void	init_send_queue(PROCESS* p_proc);
void	enqueue_send(PROCESS* p, PROCESS* p_proc);
PROCESS* dequeue_send(PROCESS* p);
int	isEmpty(SEND_QUEUE* queue);
void	dump_proc();
void	dump_msg(MESSAGE* p_msg);
void	failure(char* exp, char* file, int line);

#define assert(exp) if(!exp) failure(#exp, __FILE__, __LINE__)

void	schedule();

#endif /* PROC_H */
