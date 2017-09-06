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

int find_file(char* filename)
{
	int bFound = 0;
	
	_memset(fsbuf, 0, BUFSIZE);
	
	/* 读入根目录 ( 1 个扇区 ) */
	read_hd(ROOTDIR_SEC, fsbuf, NR_ROOTDIR_SECS * SECTOR_SIZE);
	
	//init_video();
	//_printf("\nfilename: %s\n", filename);
	//print_bytes(fsbuf, NR_ROOTDIR_SECS * SECTOR_SIZE);
	
	DIR_ENTRY* pde = (DIR_ENTRY*) fsbuf;
	for (int i = 0; i < NR_FILES; i++, pde++)
	{
		if (!_memcmp(filename, pde->name, _strlen(filename) + 1))
			bFound = 1;
	}
	
	return bFound;
}
