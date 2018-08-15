#ifndef EXE_H
#define EXE_H

#include "type.h"

#define PROCS_BASE		0xA00000 /* 10 MB */
#define PROC_IMAGE_SIZE		0x100000 /* 1 MB */

u32	fork();
u32	do_fork();
u32	alloc_mem(u32 pid, size_t size);


#endif // EXE_H

