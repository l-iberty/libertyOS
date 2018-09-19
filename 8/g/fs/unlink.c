#include "proc.h"
#include "fs.h"
#include "hd.h"
#include "type.h"
#include "sysconst.h"
#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "global.h"

/**
 * unlink a file.
 *
 * @param pathname	The full path of file
 *
 * @return 0 if successful, otherwise -1
 */
int unlink(const char* pathname)
{
	struct message msg;
	
	msg.value	= FILE_UNLINK;
	msg.PATHNAME	= (void*) pathname;
	msg.NAMELEN	= strlen(pathname);
	
	sendrecv(BOTH, PID_TASK_FS, &msg);
	
	return msg.RETVAL;
}

int do_unlink()
{
	int i;
	struct proc* pcaller = proc_table + fs_msg.source;
	
	/* dump parameters */
	int namelen = fs_msg.NAMELEN;
	char filename[MAX_FILENAME_LEN];
	
	if (*(char*) fs_msg.PATHNAME != '/') 
	{
		printf("\n#ERROR#-do_unlink: invalid pathname: %s (pathname should start with root dir \'/\')",
			fs_msg.PATHNAME);
		return -1;
	}
	
	memcpy(va2la(proc_table + PID_TASK_FS, filename),
			va2la(pcaller, fs_msg.PATHNAME + 1), /* `+1` skip root dir `/` */
			namelen);
	filename[namelen] = 0;
	
	int nr_inode = find_file(filename);
	if (nr_inode == 0) 
	{
		printf("\n#ERROR#-do_unlink: file \"%s\" does not exist {PID:0x%.8x}",
				fs_msg.PATHNAME, pcaller->pid);
		return -1;
	}
	
	struct i_node* pin;
	for (pin = inode_table; pin < inode_table + NR_INODES; pin++) 
	{
		if (pin->i_nr_inode == nr_inode)
			break;
	}
	if (pin >= inode_table + NR_INODES) 
	{
		printf("\n#ERROR#-do_unlink: invalid i-node {PID:0x%.8x}",
			pcaller->pid);
		return -1;
	}
	
	if (pin->i_mode & I_MODE_DIR)
	{
		printf("\n#ERROR#-do_unlink: cannot remove a dir {PID:0x%.8x}",
			pcaller->pid);
		return -1;
	}
	
	if (pin->i_mode & I_MODE_CHARDEV) 
	{
		printf("\n#ERROR#-do_unlink: cannot remove a char-device {PID:0x%.8x}",
			pcaller->pid);
		return -1;
	}
	
	if (pin->i_mode & I_MODE_NORMAL) 
	{
		if (pin->i_cnt > 0) 
		{
			printf("\n#ERROR#-do_unlink: file \"%s\" is being used by some procs {PID:0x%.8x}",
				fs_msg.PATHNAME, pcaller->pid);
			return -1;
		}
		
		pin = get_inode(nr_inode);
		int inode_idx = nr_inode - 1;
		int sec_idx = pin->i_start_sector;
		int nr_sectors = pin->i_nr_sectors;
		
		/*****************************/
		/* clear inode_table[] entry */
		/*****************************/
		memset(pin, 0, sizeof(struct i_node));
		
		int byte_idx, bit_idx;
		
		/****************************/
		/* clear bit  in inode_map  */
		/****************************/
		READ_HD(INODE_MAP_SEC, imap_buf, sizeof(imap_buf));
		byte_idx = inode_idx / 8;
		bit_idx = inode_idx % 8;
		imap_buf[byte_idx] &= ~(1 << bit_idx);
		WRITE_HD(INODE_MAP_SEC, imap_buf, sizeof(imap_buf));
		
		
		/****************************/
		/* clear bits in sector_map */
		/****************************/
		READ_HD(SECTOR_MAP_SEC, smap_buf, sizeof(smap_buf));
		byte_idx = sec_idx / 8;
		
		for (i = 0; i < nr_sectors / 8; i++)
			smap_buf[byte_idx++] = 0; /* 1111_1111 -> 0000_0000 */
		
		for (i = 0; i < sec_idx % 8; i++)
			smap_buf[byte_idx] &= ~(1 << i);
		
		WRITE_HD(SECTOR_MAP_SEC, smap_buf, sizeof(smap_buf));
		
		
		/****************************/
		/* clear inode_array entry  */
		/****************************/
		READ_HD(INODE_ARRAY_SEC, inode_buf, sizeof(inode_buf));
		uint8_t* pch = inode_buf;
		for (i = 0; i < NR_INODES; i++, pch += INODE_DISK_SIZE) 
		{
			if (!memcmp(pch, pin, INODE_DISK_SIZE)) /* i-node found! */
			{ 
				memset(pch, 0, INODE_DISK_SIZE);
				WRITE_HD(INODE_ARRAY_SEC, inode_buf, sizeof(inode_buf));
				break;
			}
		}

		
		/****************************/
		/* clear root_dir entry     */
		/****************************/
		READ_HD(ROOTDIR_SEC, dirent_buf, sizeof(dirent_buf));
		struct dir_entry* pde = (struct dir_entry*) dirent_buf;
		for (i = 0; i < NR_FILES; i++, pde++) 
		{
			if (pde->nr_inode == nr_inode) /* dir entry found! */
			{ 
				memset(pde, 0, sizeof(struct dir_entry));
				WRITE_HD(ROOTDIR_SEC, dirent_buf, sizeof(dirent_buf));
				break;
			}
		}
	}
	
	return 0;
}
