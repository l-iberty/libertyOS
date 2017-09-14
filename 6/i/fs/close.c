#include "proc.h"
#include "fs.h"
#include "type.h"
#include "sysconst.h"
#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "global.h"

/**
 * close a file.
 *
 * @return (0): if successful, or (-1) if failed
 */
int close(int fd)
{
	MESSAGE msg;
	
	msg.value = FILE_CLOSE;
	msg.FD = fd;
	
	sendrecv(BOTH, PID_TASK_FS, &msg);
	
	return msg.RETVAL;
}

/**
 * handle `FILE_CLOSE` message
 */
int do_close()
{
	
	int fd = fs_msg.FD;
	PROCESS* pcaller = proc_table + fs_msg.src_pid;
	
	if (fd < 0 || fd >= NR_FILES)
		return -1;
	
	if (!(pcaller->filp[fd]) ||
		!(pcaller->filp[fd]->fd_inode)) {
		return -1;	
	}
	
/*	assert(pcaller->filp[fd]);*/
/*	assert(pcaller->filp[fd]->fd_inode);*/
	
	clear_inode(pcaller->filp[fd]->fd_inode);	/* 1.清空 inode_table[] 里相应槽位 */
	pcaller->filp[fd]->fd_inode = NULL;			/* 2.释放指向 inode_table[] 槽位的指针 */
	clear_fd_tbl(pcaller->filp[fd]);			/* 3.清空 f_desc_table[] 里相应槽位 */
	pcaller->filp[fd] = NULL;					/* 4.释放指向 f_desc_table[] 槽位的指针 */
	
	return 0;
}

void clear_inode(I_NODE* pin)
{
	_memset(pin, 0, sizeof(I_NODE));
}

void clear_fd_tbl(FILE_DESC* pfd)
{
	_memset(pfd, 0, sizeof(FILE_DESC));
}

