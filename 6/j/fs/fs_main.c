#include "proc.h"
#include "fs.h"
#include "sysconst.h"
#include "global.h"
#include "stdio.h"
#include "stdlib.h"
#include "string.h"

/* Ring1 */
void Task_fs()
{
	_printf("\n-----Task_fs-----");

	init_fs();
	
	for (;;) {
		sendrecv(RECEIVE, ANY, &fs_msg);
		switch (fs_msg.value)
		{
			case FILE_OPEN:
			{
				fs_msg.FD = do_open();
				break;
			}
			case FILE_CLOSE:
			{
				fs_msg.RETVAL = do_close();
				break;
			}
			case FILE_READ:
			case FILE_WRITE:
			{
				fs_msg.RETVAL = do_rdwt();
				break;
			}
			default:
			{
				_printf("\nFS-{unknown message value}");
				dump_msg(&fs_msg);
			}
		}
		sendrecv(SEND, fs_msg.src_pid, &fs_msg);
	}
}

void init_fs()
{
	/* initialize f_desc_table & inode_table */
	_memset(f_desc_table, 0, sizeof(FILE_DESC) * NR_FILES);
	_memset(inode_table, 0, sizeof(I_NODE) * NR_FILES);

	/* open hd driver */	
	MESSAGE msg;
	msg.value = DEV_OPEN;
	sendrecv(BOTH, PID_TASK_HD, &msg);

	mkfs();
}

void mkfs()
{
	int i, k, idx, _idx;
	
	/***************************/
	/*	Super Block	   */
	/***************************/
	SUPER_BLOCK sb;
	sb.nr_inodes		= NR_INODES;
	sb.nr_inode_sectors	= NR_INODE_SECTORS;
	sb.nr_sectors		= NR_SECTORS;
	sb.nr_imap_sectors	= NR_IMAP_SECTORS;
	sb.nr_smap_sectors	= NR_SMAP_SECTORS;
	sb.first_sector		= FIRST_SECTOR;
	sb.root_inode		= ROOT_INODE;
	
	_memset(fsbuf, 0, BUFSIZE);
	_memcpy(fsbuf, &sb, sizeof(sb));
	

	/***************************/
	/*	inode map	   */
	/***************************/
	idx = SECTOR_SIZE;
	for (i = 0; i < (NR_CONSOLES + 1); i++)
		fsbuf[idx] |= (1 << i);

/************************************************************

	fsbuf[idx] should be 0001 1111
			      | ||||
			      | |||`---- bit 0 : root dir `\`
			      | ||`----- bit 1 : `dev_tty0`
			      | |`------ bit 2 : `dev_tty1`
			      | `------- bit 3 : `dev_tty2`
			      `--------- bit 4 : `dev_tty3`

************************************************************/

	/***************************/
	/*	sector map	   */
	/***************************/
	idx += SECTOR_SIZE;
	_idx = idx;
	int nr_sectors_used = 1 + sb.nr_imap_sectors + sb.nr_smap_sectors + sb.nr_inode_sectors + 1;
	/* super_block, inode_map, sector_map, inode_array, root_dir */
	for (i = 0; i < nr_sectors_used / 8; i++)
		fsbuf[idx++] = 0xFF;
		
	for (k = 0; k < nr_sectors_used % 8; k++)
		fsbuf[idx] |= (1 << k);
	
	
	/***************************/
	/*	inode array	   */
	/***************************/
	_idx += sb.nr_smap_sectors * SECTOR_SIZE;
	idx = _idx;
	I_NODE* pin = (I_NODE*) &fsbuf[idx];
	
	/* inode for `/` */
	pin->i_mode = I_MODE_DIR;
	pin->i_size = NR_ROOTDIR_SECTORS * SECTOR_SIZE;
	pin->i_start_sector = ROOTDIR_SEC;
	pin->i_nr_sectors = 1;
	
	idx += INODE_DISK_SIZE; 
	
	/* inodes for dev_tty0~3 */
	u8* pch = &fsbuf[idx];
	for (i = 0; i < NR_CONSOLES; i++, pch += INODE_DISK_SIZE) {
		pin = (I_NODE*) pch;
		pin->i_mode = I_MODE_CHARDEV;
		pin->i_size = 0; /* dev_tty 不存在于磁盘上 */
		pin->i_start_sector = (u32) (-1);
		pin->i_nr_sectors = 0;
	}
	
	
	/***************************/
	/*	根目录文件	   */
	/***************************/
	_idx += sb.nr_inode_sectors * SECTOR_SIZE;
	idx = _idx;
	
	/* `.` */
	DIR_ENTRY* pde = (DIR_ENTRY*) &fsbuf[idx];
	pde->nr_inode = 1;
	_strcpy(pde->name, ".");
	
	/* `dev_tty0~3` */
	for (i = 0; i < NR_CONSOLES; i++) {
		pde++;
		pde->nr_inode = i + 2;
		_sprintf(pde->name, "dev_tty%.1x", i);		
	}
	
	write_hd(0, fsbuf, BUFSIZE);
}

/**
 * @param sector	绝对扇区号
 * @param buf		缓冲区, 待写入的数据存放在此
 * @param len		写入多少字节数据
 */
void write_hd(int sector, void* buf, int len)
{
	MESSAGE msg;
	msg.value = DEV_WRITE;
	msg.DRIVE = 0;
	msg.SECTOR = sector;
	msg.LEN = len;
	msg.BUF = buf;
	
	sendrecv(BOTH, PID_TASK_HD, &msg);
}

/**
 * @param sector	绝对扇区号
 * @param buf		缓冲区, 数据读入到此
 * @param len		读入多少字节数据
 */
void read_hd(int sector, void* buf, int len)
{
	MESSAGE msg;
	msg.value = DEV_READ;
	msg.DRIVE = 0;
	msg.SECTOR = sector;
	msg.LEN = len;
	msg.BUF = buf;
	
	sendrecv(BOTH, PID_TASK_HD, &msg);
}

