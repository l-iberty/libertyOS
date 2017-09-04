#include "proc.h"
#include "fs.h"
#include "type.h"
#include "sysconst.h"
#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "global.h"

/**
 * open/create a file.
 *
 * @param pathname	The full path of file
 * @param flags		O_CREAT, O_RDWR, etc
 *
 * @return file descriptor if successful, otherwise -1
 */
int open(const char* pathname, int flags)
{
	MESSAGE msg;
	
	msg.value	= FILE_OPEN;
	msg.PATHNAME	= (void*) pathname;
	msg.FLAGS	= flags;
	msg.NAMELEN	= _strlen(pathname);
	
	sendrecv(BOTH, PID_TASK_FS, &msg);
	
	return msg.FD;
}

/**
 * Open a file and return a fd.
 */
int do_open()
{
	int fd = -1;
	
	_printf("\npathname: %s, flags: 0x%.8x, namelen: 0x%.8x\n",
		fs_msg.PATHNAME, fs_msg.FLAGS, fs_msg.NAMELEN);
	
	return fd;
}
