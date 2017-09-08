#include "proc.h"
#include "fs.h"
#include "type.h"
#include "sysconst.h"
#include "stdio.h"
#include "stdlib.h"
#include "string.h"


void print_bytes(u8* buf, int len)
{
	for (int i = 0; i < len; i++)
	{
		_printf("%.2x ", buf[i]);
		
		if ((i + 1) % 16 == 0) _printf("\n");
	}
}

/**
 * 查找根目录下的文件
 *
 * @param	filename 不含路径的纯文件名
 * @return	DIR_ENTRY::nr_inode, or (-1) if not found
 */
int find_file(char* filename)
{
	int nr_inode = -1;
	
	_memset(fsbuf, 0, BUFSIZE);
	
	/* 读入根目录 ( 1 个扇区 ) */
	read_hd(ROOTDIR_SEC, fsbuf, NR_ROOTDIR_SECTORS * SECTOR_SIZE);
	
	DIR_ENTRY* pde = (DIR_ENTRY*) fsbuf;
	for (int i = 0; i < NR_FILES; i++, pde++)
	{
		if (!_memcmp(filename, pde->name, _strlen(filename) + 1))
		{
			nr_inode = pde->nr_inode;
			break;
		}
	}
	
	return nr_inode;
}
