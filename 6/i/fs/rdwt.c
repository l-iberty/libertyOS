#include "proc.h"
#include "fs.h"
#include "type.h"
#include "sysconst.h"
#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "global.h"

size_t write(int fd, void* buf, size_t len)
{
	MESSAGE msg;
	
	msg.FD		= fd;
	msg.BUF		= buf;
	msg.LEN		= len;
	msg.value	= FILE_WRITE;
	
	sendrecv(BOTH, PID_TASK_FS, &msg);
	
	return msg.RETVAL;
}

size_t read(int fd, void* buf, size_t len)
{
	MESSAGE msg;
	
	msg.FD		= fd;
	msg.BUF		= buf;
	msg.LEN		= len;
	msg.value	= FILE_READ;
	
	sendrecv(BOTH, PID_TASK_FS, &msg);
	
	return msg.RETVAL;
}

size_t do_rdwt()
{
	PROCESS* pcaller = proc_table + fs_msg.src_pid;
	
	size_t nr_bytes = 0; /* num of bytes RD/WR */
	
	/* dump parameters from message */
	int fd = fs_msg.FD;
	void* buf = fs_msg.BUF;
	size_t len = fs_msg.LEN;
	int type = fs_msg.value;
	
	FILE_DESC* pfd = pcaller->filp[fd];
	I_NODE* pin = pfd->fd_inode;
	int nr_inode = pin->i_nr_inode;
	int chunk; /* 每轮迭代读写多少字节? */
	
	if (!(pfd->fd_mode & O_RDWR))
		return -1;

	len = min(len, pin->i_nr_sectors << SECTOR_SIZE_SHIFT);
	/* 左移 `SECTOR_SIZE_SHIFT`　位 <=> 乘以 `SECTOR_SIZE` */ 

	/* 读写文件数据时使用 fsbuf 做缓冲 */	
	_memset(fsbuf, 0, SECTOR_SIZE);
	
	/**
	 * 读文件时:
	 * case 1: 文件无数据 -> fd_pos = i_size = 0
	 *		case 1.1: len = 0, then len = 0;
	 *		case 1.2: len > 0, then len = 0.
	 * case 2: 文件有数据, 且文件指针指向最后一个字节 -> fd_pos = i_size. (fd_pos从 1 开始)
	 *		case 2.1: len < i_size, then len = 0;
	 *		case 2.2: len >= i_size, then len = 0.
	 * case 3: 文件有数据, 且 len > i_size, 则最多只能读 (i_size - fd_pos) 字节
	 */
	if (type == FILE_READ)
	{
		if (pfd->fd_pos == pin->i_size)
			len = max(0, min(0, len - pin->i_size));
		else
			len = min(len, pin->i_size - pfd->fd_pos);
		
		/* make sure "len <= i_size" */
		len = min(len, pin->i_size);
	}
	/* 读写长度 len 按 SECTOR_SIZE 向上取整 */
	int nr_sectors = (len + SECTOR_SIZE - 1) >> SECTOR_SIZE_SHIFT;
	/* 右移 `SECTOR_SIZE_SHIFT` 位 <=> 除以 `SECTOR_SIZE` */
	
	int start_sector = pin->i_start_sector +
				(pfd->fd_pos >> SECTOR_SIZE_SHIFT);
		
	for (int i = 0; i < nr_sectors && nr_bytes < len; i++)
	{
		/* read file data from disk */
		read_hd(start_sector + i, fsbuf, SECTOR_SIZE);
		
		int offset = pfd->fd_pos % SECTOR_SIZE;
		
		if (i == nr_sectors - 1) /* the last sector */
			chunk = len - nr_bytes;
		else	/* not the last sector */
			chunk = SECTOR_SIZE - offset;
		
		if (type == FILE_WRITE)
		{
			/* write file data */
            _memcpy(fsbuf + offset, buf, chunk);
            write_hd(start_sector + i, fsbuf, SECTOR_SIZE);
		}
		else if (type == FILE_READ)
		{
			/* dump file data, so thar `pcaller` can get it */
            _memcpy(buf, fsbuf + offset, chunk);
		}
		/* clear fsbuf */
		_memset(fsbuf, 0, SECTOR_SIZE);
		
		nr_bytes += chunk;
		pfd->fd_pos += chunk;
		buf += chunk;
	}
	
    if (pfd->fd_pos > pin->i_size)
    {
    	/**
    	 * update file size (INODE::i_size), which is
    	 * needed when calling `write` routine.
    	 */
    	pin->i_size = pfd->fd_pos;
    	
    	/* write inode_array back to the disk */
	    read_hd(INODE_ARRAY_SEC, inode_buf, sizeof(inode_buf));
	    assert(nr_inode > 0);
	    _memcpy(inode_buf + INODE_DISK_SIZE * (nr_inode - 1),
	            va2la(pcaller, pcaller->filp[fd]->fd_inode),
	            INODE_DISK_SIZE);

	    write_hd(INODE_ARRAY_SEC, inode_buf, sizeof(inode_buf));
	    _memset(inode_buf, 0, sizeof(inode_buf));
    }
	
	return nr_bytes;
}
