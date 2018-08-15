#include "proc.h"
#include "fs.h"
#include "sysconst.h"
#include "global.h"
#include "stdio.h"
#include "stdlib.h"
#include "string.h"


void TaskFS()
{
	printf("\n-----TaskFS-----");

	init_fs();
	
	for (;;) 
	{
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
			case FILE_UNLINK:
			{
				fs_msg.RETVAL = do_unlink();
				break;
			}
			default:
			{
				printf("\nFS-{unknown message value}");
				dump_msg(&fs_msg);
			}
		}
		sendrecv(SEND, fs_msg.source, &fs_msg);
	}
}

void init_fs()
{
	printf("\n{FS}    initializing file system...");

	/* initialize f_desc_table & inode_table */
	memset(f_desc_table, 0, sizeof(FILE_DESC) * NR_FILES);
	memset(inode_table, 0, sizeof(I_NODE) * NR_FILES);

	/* open hd driver */	
	printf("\n{FS}    opening hard-disk driver...");
	MESSAGE msg;
	msg.value = DEV_OPEN;
	sendrecv(BOTH, PID_TASK_HD, &msg);
	
	printf("\n{FS}    making file system...");
	mkfs();
	
	printf("\n{FS}    file system initialization done");
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
	
	memset(fsbuf, 0, BUFSIZE);
	memcpy(fsbuf, &sb, sizeof(sb));
	

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
	I_NODE* pin2 = inode_table;
	
	/* inode for `/` */
	pin2->i_mode = pin->i_mode = I_MODE_DIR;
	pin2->i_size = pin->i_size = NR_ROOTDIR_SECTORS * SECTOR_SIZE;
	pin2->i_start_sector = pin->i_start_sector = ROOTDIR_SEC;
	pin2->i_nr_sectors = pin->i_nr_sectors = 1;
	pin2->i_nr_inode = 1;
	pin2->i_cnt = 0;
	pin2++;
	
	idx += INODE_DISK_SIZE; 
	
	/* inodes for dev_tty0~3 */
	u8* pch = &fsbuf[idx];
	for (i = 0; i < NR_CONSOLES; i++, pch += INODE_DISK_SIZE, pin2++) 
	{
		pin = (I_NODE*) pch;
		pin2->i_mode = pin->i_mode = I_MODE_CHARDEV;
		pin2->i_size = pin->i_size = 0; /* dev_tty 不存在于磁盘上 */
		pin2->i_start_sector = pin->i_start_sector = (u32) (-1);
		pin2->i_nr_sectors = pin->i_nr_sectors = 0;
		pin2->i_nr_inode = i + 2;
		pin2->i_cnt = 0;
	}
	
	
	/***************************/
	/*	根目录文件	   */
	/***************************/
	_idx += sb.nr_inode_sectors * SECTOR_SIZE;
	idx = _idx;
	
	/* `.` */
	DIR_ENTRY* pde = (DIR_ENTRY*) &fsbuf[idx];
	pde->nr_inode = 1;
	strcpy(pde->name, ".");
	
	/* `dev_tty0~3` */
	for (i = 0; i < NR_CONSOLES; i++) 
	{
		pde++;
		pde->nr_inode = i + 2;
		sprintf(pde->name, "dev_tty%.1x", i);		
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

