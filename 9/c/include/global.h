#ifndef GLOBAL_H
#define GLOBAL_H
#include "proc.h"

#define min(a,b) (a)>(b)?(b):(a)
#define max(a,b) (a)>(b)?(a):(b)


#ifndef DEBUG
#	define STATIC
#else
#	define STATIC static
#endif

/* variables */
extern uint8_t	GDT[GDT_DESC_NUM * DESC_SIZE];	/* kernel/kernel.asm */
extern uint8_t	IDT[IDT_DESC_NUM * DESC_SIZE];	/* kernel/kernel.asm */
extern uint8_t	TSS[TSS_SIZE];			/* kernel/kernel.asm */

/* syscall - RING3 */
int		get_ticks();
int		sendrecv(int func_type, int pid, struct message* p_msg);
uint32_t	getpid();
uint32_t	getppid();
void		printk(const char* sz);
int		sem_init(struct semaphore* p_sem, int value);
int		sem_post(struct semaphore* p_sem);
int		sem_wait(struct semaphore* p_sem);
uint32_t	getcr3();
void		write_process_memory(uint32_t pid, void *p_dst, void *p_src, uint32_t len);
void		sleep(int ms);
void*		get_first_chunk();
void		set_first_chunk(void* addr);

#endif /* GLOBAL_H */

