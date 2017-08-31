#include "type.h"
#include "proc.h"
#include "fs.h"
#include "sysconst.h"
#include "global.h"
#include "stdio.h"
#include "stdlib.h"
#include "string.h"

#define SECTOR_SIZE	512
#define BUFSIZE		(SECTOR_SIZE * 17 * 8)


u8 buf[BUFSIZE];

/* Ring1 */
void Task_fs()
{
	_printf("\n-----Task_fs-----");

	MESSAGE msg;
	
	msg.value = DEV_OPEN;
	sendrecv(SEND, PID_TASK_HD, &msg);
	
	mkfs();
	
	halt("FS DONE");
}

void mkfs()
{
	int i, k, idx, _idx;
	
	/***************************/
	/*	Super Block	   */
	/***************************/
	SUPER_BLOCK sb;
	sb.nr_inodes		= SECTOR_SIZE * 8; /* inode_map 占用一个扇区, 每一个二进制位映射一个 i_node, 1 Byte = 8 bit */
	sb.nr_inode_sectors	= sb.nr_inodes * sizeof(I_NODE) / SECTOR_SIZE;
	sb.nr_sectors		= 0x4800; /* hd.img 按 9M 计算, 共 0x4800 个扇区 */
	sb.nr_imap_sectors	= 1;
	sb.nr_smap_sectors	= 5; /* 一个扇区可映射 0x200*8=0x1000 个扇区, 映射 0x4800 个扇区需要 5 个扇区 */
	sb.first_sector		= 1; /* super_block 占据 0 号扇区 */
	sb.root_inode		= ROOT_INODE;
	
	_memset(buf, 0, BUFSIZE);
	_memcpy(buf, &sb, sizeof(sb));
	

	/***************************/
	/*	inode map	   */
	/***************************/
	idx = SECTOR_SIZE;
	for (i = 0; i < (NR_CONSOLES + 1); i++)
		buf[idx] |= (1 << i);

/************************************************************

	buf[idx] should be 0001 1111
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
		buf[idx++] = 0xFF;
		
	for (k = 0; k < nr_sectors_used % 8; k++)
		buf[idx] |= (1 << k);
	
	
	/***************************/
	/*	inode array	   */
	/***************************/
	_idx += sb.nr_smap_sectors * SECTOR_SIZE;
	idx = _idx;
	I_NODE* pi = (I_NODE*) &buf[idx];
	
	/* inode for `/` */
	pi->i_mode = I_MODE_DIR;
	pi->i_size = sizeof(DIR_ENTRY) * 5; /* 5 files. `.`, `dev_tty0~3` */
	pi->i_start_sector = sb.first_sector;
	pi->i_nr_sectors = 1;
	
	idx += sizeof(I_NODE);
	pi = (I_NODE*) &buf[idx];
	
	/* inodes for dev_tty0~3 */
	for (i = 0; i < NR_CONSOLES; i++, pi++)
	{
		pi->i_mode = I_MODE_CHARDEV;
		pi->i_size = 0; /* dev_tty 不存在于磁盘上 */
		pi->i_start_sector = (u32) (-1);
		pi->i_nr_sectors = 0;
	}
	
	
	/***************************/
	/*	根目录文件	   */
	/***************************/
	_idx += sb.nr_inode_sectors * SECTOR_SIZE;
	idx = _idx;
	
	/* `.` */
	DIR_ENTRY* pde = (DIR_ENTRY*) &buf[idx];
	pde->nr_inode = 1;
	_strcpy(pde->name, ".");
	
	/* `dev_tty0~3` */
	for (i = 0; i < NR_CONSOLES; i++)
	{
		pde++;
		pde->nr_inode = i + 2;
		_sprintf(pde->name, "dev_tty%.1x", i);		
	}
	
	write_hd(0, buf, BUFSIZE);
}

void write_hd(int sector, void* buf, int len)
{
	MESSAGE msg;
	msg.value = DEV_WRITE;
	msg.DEVICE = 0;
	msg.SECTOR = sector;
	msg.LEN = len;
	msg.BUF = buf;
	
	sendrecv(SEND, PID_TASK_HD, &msg);
}
