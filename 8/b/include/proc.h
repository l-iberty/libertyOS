#ifndef PROC_H
#define PROC_H

#include "type.h"
#include "sysconst.h"
#include "fs.h"

#define TASK_STACK_SIZE		10*1024
#define MAX_MSGSIZE		128


struct stack_frame
{
	uint32_t	gs;	/* \							*/
	uint32_t	fs;	/* |							*/
	uint32_t	es;	/* | pushed manually.					*/
	uint32_t	ds;	/* /							*/
	uint32_t	edi;	/* \							*/
	uint32_t	esi;	/* |							*/
	uint32_t	ebp;	/* |							*/
	uint32_t	k_esp;	/* | pushed by `pushad`.				*/
	uint32_t	ebx;	/* |							*/
	uint32_t	edx;	/* |							*/
	uint32_t	ecx;	/* |							*/
	uint32_t	eax;	/* /							*/
	uint32_t	eip;	/* \							*/
	uint32_t	cs;	/* |							*/
	uint32_t	eflags;	/* | pushed by processor when interruption happens.	*/
	uint32_t	esp;	/* |							*/
	uint32_t	ss;	/* /							*/
};

struct mess1 
{
	int	m1i1;
	int	m1i2;
	int	m1i3;
	int	m1i4;
	int	m1i5;
	int	m1i6;
	int	m1i7;
	int	m1i8;
};

struct mess2 
{
	void*	m2p1;
	void*	m2p2;
	void*	m2p3;
	void*	m2p4;
	void*	m2p5;
	void*	m2p6;
	void*	m2p7;
	void*	m2p8;
};

struct message
{
	int	source;
	int	dest;
	int	value;
	union 
	{
		struct mess1 m1;
		struct mess2 m2;
	} u;
};


/* definition for message */
#define DRIVE		u.m1.m1i1
#define SECTOR		u.m1.m1i2
#define LEN		u.m1.m1i3
#define BUF		u.m2.m2p1
#define PATHNAME	u.m2.m2p2
#define FLAGS		u.m1.m1i4
#define NAMELEN		u.m1.m1i5
#define FD		u.m1.m1i6
#define RETVAL		u.m1.m1i7
#define FORK_PID        u.m1.m1i8
#define VM_ADDR		u.m2.m2p1
#define VM_SIZE		u.m1.m1i2
#define VM_PROTECT	u.m1.m1i3
#define VM_BASE		u.m2.m2p4


struct proc_queue
{
	int		count;
	struct proc**	p_head;
	struct proc** 	p_tail;
	struct proc*	procs[NR_PROCS];
};

struct proc 
{
	struct stack_frame	regs;
	uint16_t		ldt_selector;
	uint8_t			LDT[LDT_DESC_NUM * DESC_SIZE];
	uint32_t		pid;
	uint32_t		pid_parent;
	int			ticks;
	int			priority;
	
	struct message*		p_msg;
	int			flag;
	uint32_t		pid_recvfrom;	/* Whom does the process wanna receive message from? */
	uint32_t		pid_sendto;	/* Whom does the process wanna send message to? */
	struct proc_queue	send_queue;
	int			has_int_msg;	/* Does the process have a message from INTERRUPT to handle? */
	
	struct file_desc*	filp[NR_FILES];
};

struct task
{
	TASK_ENTRY	task_entry;
	uint8_t*	task_stack;
};

struct semaphore
{
	int value;
	struct proc_queue wait_queue;
};

#define FIRST_PROC	proc_table[0]
#define LAST_PROC	proc_table[NR_PROCS - 1]

/* pid */
#define PID_INIT        0
#define PID_TASK_A	1
#define	PID_TASK_B	2
#define PID_TASK_C	3
#define PID_TASK_TTY	4
#define PID_TASK_HD	5
#define PID_TASK_FS	6
#define PID_TASK_MM	7
#define PID_TASK_EXE	8

/* flag */
/* 进程状态 flag 的二进制串不能有重叠的 "1" */
#define SENDING		1
#define RECEIVING	(1 << 1)
#define WAITING		(1 << 2)
#define FREE_SLOT       0x10

/* pid_recvfrom & pid_sendto */
#define	ANY		(uint32_t) (-1)
#define NONE		(uint32_t) (-2)
#define INTERRUPT	(uint32_t) (-3)

/* message value */
#define DEV_OPEN	1001
#define DEV_READ	1002
#define DEV_WRITE	1003
#define FILE_OPEN	1004
#define FILE_CLOSE	1005
#define FILE_READ	1006
#define FILE_WRITE	1007
#define FILE_UNLINK	1008
#define FORK            1009
#define VM_ALLOC	1010
#define VM_FREE		1011
#define HARD_INT	2001

extern uint8_t	task_stack_init[TASK_STACK_SIZE];
extern uint8_t	task_stack_a[TASK_STACK_SIZE];
extern uint8_t	task_stack_b[TASK_STACK_SIZE];
extern uint8_t	task_stack_c[TASK_STACK_SIZE];
extern uint8_t	task_stack_tty[TASK_STACK_SIZE];
extern uint8_t	task_stack_hd[TASK_STACK_SIZE];
extern uint8_t	task_stack_fs[TASK_STACK_SIZE];
extern uint8_t	task_stack_mm[TASK_STACK_SIZE];
extern uint8_t	task_stack_exe[TASK_STACK_SIZE];

extern struct proc	proc_table[NR_PROCS];
extern struct task	task_table[NR_PROCS];
extern SYSCALL 		syscall_table[NR_SYSCALL];

void Init();
void TaskA();
void TaskB();
void TaskC();
void TaskTTY();
void TaskHD();
void TaskFS();
void TaskMM();
void TaskEXE();

int		sys_get_ticks();
int		sys_sendrecv(int func_type, int pid, struct message* p_msg);
uint32_t	sys_getpid();
uint32_t	sys_getppid();
void		sys_printk(const char* sz);
int		sys_sem_init(struct semaphore* p_sem, int value);
int		sys_sem_post(struct semaphore* p_sem);
int		sys_sem_wait(struct semaphore* p_sem);
void		sys_disable_paging();
void		sys_enable_paging();
void		sys_reload_cr3(uint32_t cr3);
uint32_t	sys_getcr3();

void		schedule();
int	    	msg_send(uint32_t pid_sender, uint32_t pid_receiver, struct message* p_msg);
int	    	msg_recv(uint32_t pid_sender, uint32_t pid_receiver, struct message* p_msg);
void	    	block(struct proc* p_proc);
void	    	unblock(struct proc* p_proc);
int	    	deadlock(int src, int dst);
void	    	inform_int(int pid);
void	    	interrupt_wait();
void	    	reset_msg(struct message* p_msg);
void	    	init_send_queue(struct proc* p_proc);
void	    	enqueue(struct proc_queue* queue, struct proc* p_proc);
struct proc*    dequeue(struct proc_queue* queue);
int	    	empty(struct proc_queue* queue);
void*	    	va2la(struct proc* proc, void* va);
void		dump_proc(struct proc* p_proc);
void	    	dump_msg(struct message* p_msg);
void	    	failure(char* exp, char* file, char* base_file, int line);
void	    	panic(const char* fmt, ...);

#define assert(exp) if(exp); \
			else failure(#exp, __FILE__, __BASE_FILE__, __LINE__)


#endif /* PROC_H */
