#include "proc.h"
#include "fs.h"
#include "type.h"
#include "sysconst.h"
#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "irq.h"

/**
 * 查找根目录下的文件
 *
 * @param	filename 不含路径的纯文件名
 * @return	struct dir_entry::nr_inode, or (0) if not found
 */
int find_file(char* filename)
{
	int nr_inode = 0;
	
	/* 读入根目录 ( 1 个扇区 ) */
	read_hd(ROOTDIR_SEC, dirent_buf, sizeof(dirent_buf));
	
	struct dir_entry* pde = (struct dir_entry*) dirent_buf;
	for (int i = 0; i < NR_FILES; i++, pde++) 
	{
		if (!memcmp(filename, pde->name, strlen(filename) + 1)) 
		{
			nr_inode = pde->nr_inode;
			break;
		}
	}
	
	return nr_inode;
}

