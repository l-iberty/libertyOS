#ifndef PROC_H
#define PROC_H

#include "type.h"
#include "sysconst.h"
#include "fs.h"

#define TASK_STACK_SIZE         1024
#define PROC_START_BRK          0x0C000000
#define PROC_DATA_SEG_MAXSIZE   0x04000000

struct stack_frame {
    /* offset */
    /* 0  */ uint32_t gs;       /* \							*/
    /* 4  */ uint32_t fs;       /* |							*/
    /* 8  */ uint32_t es;       /* | pushed manually.			*/
    /* 12 */ uint32_t ds;       /* /							*/
    /* 16 */ uint32_t edi;      /* \							*/
    /* 20 */ uint32_t esi;      /* |							*/
    /* 24 */ uint32_t ebp;      /* |							*/
    /* 28 */ uint32_t k_esp;    /* | pushed by `pushad`.		*/
    /* 32 */ uint32_t ebx;      /* |							*/
    /* 36 */ uint32_t edx;      /* |							*/
    /* 40 */ uint32_t ecx;      /* |							*/
    /* 44 */ uint32_t eax;      /* /							*/
    /* 48 */ uint32_t err_code;
    /* 52 */ uint32_t eip;      /* \							*/
    /* 56 */ uint32_t cs;       /* |							*/
    /* 60 */ uint32_t eflags;   /* | pushed by processor when 	*/
    /* 64 */ uint32_t esp;      /* | interruption happens.		*/
    /* 68 */ uint32_t ss;       /* /							*/
};

struct mess1 {
    int m1i1;
    int m1i2;
    int m1i3;
    int m1i4;
    int m1i5;
    int m1i6;
    int m1i7;
    int m1i8;
};

struct mess2 {
    void* m2p1;
    void* m2p2;
    void* m2p3;
    void* m2p4;
    void* m2p5;
    void* m2p6;
    void* m2p7;
    void* m2p8;
};

struct message {
    int source;
    int dest;
    int value;
    union {
        struct mess1 m1;
        struct mess2 m2;
    } u;
};

/* definition for message */
#define DRIVE       u.m1.m1i1
#define SECTOR		u.m1.m1i2
#define LEN			u.m1.m1i3
#define BUF			u.m2.m2p1
#define PATHNAME	u.m2.m2p2
#define FLAGS		u.m1.m1i4
#define NAMELEN		u.m1.m1i5
#define FD			u.m1.m1i6
#define RETVAL		u.m1.m1i7
#define FORK_PID	u.m1.m1i8
#define VM_ADDR		u.m2.m2p1
#define VM_SIZE		u.m1.m1i2
#define VM_PROTECT	u.m1.m1i3
#define VM_BASE		u.m2.m2p4
#define PD_BASE		u.m1.m1i5
#define PT_BASE		u.m1.m1i6
#define BRK_ADDR	u.m2.m2p1
#define BRK_INC		u.m1.m1i1
#define NEW_BRK		u.m2.m2p2

struct proc_queue {
    int count;
    struct proc** p_head;
    struct proc** p_rear;
    struct proc* procs[NR_PROCS];
};

struct mm_desc {
    uint32_t start_brk;
    uint32_t brk;
    uint32_t mlimit;
    void* first_chunk;
};

struct proc {
    /**********************************************************************/
    /******************** 以下成员的位置不能随意改动!!! ********************/
    struct stack_frame regs;
    uint16_t ldt_selector;
    uint8_t LDT[LDT_DESC_NUM * DESC_SIZE];
    uint32_t page_dir_base;
    /**********************************************************************/

    uint32_t pid;
    uint32_t pid_parent;
    int ticks;
    int priority;
    int interval_ms;

    struct message* p_msg;
    int flag;
    uint32_t pid_recvfrom; /* Whom does the process wanna receive message from? */
    uint32_t pid_sendto; /* Whom does the process wanna send message to? */
    struct proc_queue send_queue;
    int has_int_msg; /* Does the process have a message from INTERRUPT to handle? */

    struct file_desc* filp[NR_FILES];
    struct mm_desc mm;
};

struct task {
    TASK_ENTRY task_entry;
    uint8_t* task_stack;
};

struct semaphore {
    int value;
    struct proc_queue wait_queue;
};

#define FIRST_PROC proc_table[0]
#define LAST_PROC proc_table[NR_PROCS - 1]

/* pid */
#define PID_INIT		0
#define PID_TASK_A		1
#define PID_TASK_B		2
#define PID_TASK_C		3
#define PID_TASK_D		4
#define PID_TASK_E		5
#define PID_TASK_TTY	6
#define PID_TASK_HD		7
#define PID_TASK_FS		8
#define PID_TASK_MM		9
#define PID_TASK_EXE	10

/* flag */
/* 进程状态 flag 的二进制串不能有重叠的 "1" */
#define READY		0
#define SENDING		1
#define RECEIVING	(1 << 1)
#define WAITING		(1 << 2)
#define SLEEPING	(1 << 3)
#define FREE_SLOT	0x10

/* pid_recvfrom & pid_sendto */
#define ANY (uint32_t)(-1)
#define INVALID_PID (uint32_t)(-2)
#define INTERRUPT (uint32_t)(-3)

/* message value */
#define DEV_OPEN	1001
#define DEV_READ	1002
#define DEV_WRITE	1003
#define FILE_OPEN	1004
#define FILE_CLOSE	1005
#define FILE_READ	1006
#define FILE_WRITE	1007
#define FILE_UNLINK	1008
#define FORK		1009
#define VM_ALLOC	1010
#define VM_FREE		1011
#define VM_ALLOC_EX	1012
#define BRK			1013
#define SBRK		1014
#define HARD_INT	2001

extern uint8_t task_stack_init[TASK_STACK_SIZE];
extern uint8_t task_stack_a[TASK_STACK_SIZE];
extern uint8_t task_stack_b[TASK_STACK_SIZE];
extern uint8_t task_stack_c[TASK_STACK_SIZE];
extern uint8_t task_stack_tty[TASK_STACK_SIZE];
extern uint8_t task_stack_hd[TASK_STACK_SIZE];
extern uint8_t task_stack_fs[TASK_STACK_SIZE];
extern uint8_t task_stack_mm[TASK_STACK_SIZE];
extern uint8_t task_stack_exe[TASK_STACK_SIZE];

extern struct proc proc_table[NR_PROCS];
extern struct task task_table[NR_PROCS];
extern SYSCALL syscall_table[NR_SYSCALL];
extern struct proc* p_current_proc;
extern int ticks; /* get_ticks()的返回值,记录时钟中断的次数 */
extern int re_enter; /* 是否发生中断重入?如果为0,则没有发生;如果大于0,则发生 */

void Init();
void TaskA();
void TaskB();
void TaskC();
void TaskD();
void TaskE();
void TaskTTY();
void TaskHD();
void TaskFS();
void TaskMM();
void TaskEXE();

/* syscall - RING0 */
int sys_get_ticks();
int sys_sendrecv(int func_type, int pid, struct message* p_msg);
uint32_t sys_getpid();
uint32_t sys_getppid();
void sys_printk(const char* sz);
int sys_sem_init(struct semaphore* p_sem, int value);
int sys_sem_post(struct semaphore* p_sem);
int sys_sem_wait(struct semaphore* p_sem);
uint32_t sys_getcr3();
void sys_phymemcpy(void* p_dst, void* p_src, int size);
void sys_sleep(int ms);
void* sys_get_first_chunk();
void sys_set_first_chunk(void* addr);

void schedule();
void inform_int(int pid);
void interrupt_wait();
void init_send_queue(struct proc* p_proc);
void* va2la(struct proc* proc, void* va);
void* va2pa(struct proc* proc, void* va);
void* la2pa(struct proc* proc, void* la);
void dump_msg(struct message* p_msg);
void failure(char* exp, char* file, char* base_file, int line);
void panic(const char* fmt, ...);

#define assert(exp) \
    if (exp)        \
        ;           \
    else            \
    failure(#exp, __FILE__, __BASE_FILE__, __LINE__)

#define _BEGIN_PHY_MEM_RW_  \
    disable_int();          \
    disable_paging();

#define _END_PHY_MEM_RW_    \
    enable_paging();        \
    enable_int();

#endif /* PROC_H */
