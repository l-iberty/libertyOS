#ifndef EXE_H
#define EXE_H

#include "type.h"

#define PROCS_BASE		0x600000 /* 10 MB */
#define PROC_IMAGE_SIZE		0x100000 /* 1 MB */

extern struct message exe_msg;

uint32_t	fork();
uint32_t	do_fork();
uint32_t	alloc_mem(uint32_t pid, size_t size);


#endif // EXE_H

