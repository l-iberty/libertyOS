/**
 * proc.h 进程相关
 */

#ifndef PROC_H
#define PROC_H

#include "type.h"
#include "sysconst.h"

void TaskA();
void TaskB();

#define TASK_STACK_SIZE		2*1024

u8	task_stack_a[TASK_STACK_SIZE];
u8	task_stack_b[TASK_STACK_SIZE];

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
	STACK_FRAME	regs;
	u16		ldt_selector;
	u8		LDT[LDT_DESC_NUM * DESC_SIZE];
	u32		pid;
} PROCESS;

typedef struct {
	fpPROC		task_entry;
	u8*		task_stack;
} TASK;

PROCESS		proc_table[NR_TASKS];

PROCESS*	p_current_proc;


#endif /* PROC_H */
