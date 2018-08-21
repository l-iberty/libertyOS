#ifndef GLOBAL_H
#define GLOBAL_H
#include "proc.h"

#define min(a,b) ((a>b)?b:a)
#define max(a,b) ((a>b)?a:b)

/* variables */
int		ticks;			/* get_ticks() 的返回值, 记录时钟中断的次数 */	
int		nr_current_console;	/* 当前控制台号 */
struct proc*	p_current_proc;		/* 当前进程 */

extern uint8_t	GDT[GDT_DESC_NUM * DESC_SIZE];	/* kernel/kernel.asm */
extern uint8_t	IDT[IDT_DESC_NUM * DESC_SIZE];	/* kernel/kernel.asm */
extern uint8_t	TSS[TSS_SIZE];			/* kernel/kernel.asm */

/* prototypes */
int		get_ticks();
int		sendrecv(int func_type, int pid, struct message* p_msg);
uint32_t	getpid();
uint32_t	getppid();
void		printk(const char* sz);
int		sem_init(struct semaphore* p_sem, int value);
int		sem_post(struct semaphore* p_sem);
int		sem_wait(struct semaphore* p_sem);
void		disable_paging();
void		enable_paging();
void		reload_cr3(uint32_t cr3);
uint32_t	getcr3();

/* init */
void init_clock();
void init_keyboard();

#endif /* GLOBAL_H */
