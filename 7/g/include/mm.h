#ifndef MM_H
#define MM_H

#include "type.h"

u32 fork();
u32 do_fork();

u32 alloc_mem();

#define PROCS_BASE	0xA00000 /* 10 MB */
#define PROC_IMAGE_SIZE	0x100000 /* 1 MB */

#endif
