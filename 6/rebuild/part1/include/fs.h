#ifndef FS_H
#define FS_H

#include "type.h"

typedef struct {
	u32	nr_inodes;		/* 文件系统最多允许有多少个 i-node */
	u32	nr_inode_sectors;	/* inode_array 占用多少个扇区 */
	u32	nr_sectors;		/* 文件系统总共扇区数 */
	u32	nr_imap_sectors;	/* inode_map 占用多少扇区 */
	u32	nr_smap_sectors;	/* sector_map 占用多少扇区 */
	u32	first_sector;		/* 第一个数据扇区的扇区号 */
	u32	root_inode;		/* 根目录区的 i_node 号 */
} SUPER_BLOCK;

typedef struct {
	u32	i_mode;			/* Access mode */
	u32	i_size;			/* File size */
	u32	i_start_sector;		/* The first sector of the data */
	u32	i_nr_sectors;		/* How many sectors the file occupies */
} I_NODE;

#define I_MODE_DIR		(1 << 31)
#define I_MODE_CHARDEV		(1 << 30)

#define MAX_FILENAME_LEN	12

typedef struct {
	u32	nr_inode;		/* 文件的 i_node 号 */
	char	name[MAX_FILENAME_LEN];
} DIR_ENTRY;

#define ROOT_INODE	0 /* 根目录的 i_node 号 */

void mkfs();
void write_hd(int sector, void* buf, int len);

#endif /* FS_H */
