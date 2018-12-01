#ifndef EXE_H
#define EXE_H

#include "type.h"

#define PROC_IMAGE_SIZE		0x100000 /* 1 MB */

extern struct message exe_msg;

uint32_t fork();
uint32_t do_fork();


#endif // EXE_H

