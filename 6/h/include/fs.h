#ifndef FS_H
#define FS_H

#include "type.h"

typedef struct {
	u32	nr_inodes;		/* 文件系统最多允许有多少个 i-node */
	u32	nr_inode_sectors;	/* inode_array 占用多少个扇区 */
	u32	nr_sectors;		/* 文件系统总共扇区数 */
	u32	nr_imap_sectors;	/* inode_map 占用多少扇区 */
	u32	nr_smap_sectors;	/* sector_map 占用多少扇区 */
	u32	first_sector;		/* 第一个文件数据扇区的扇区号 */
	u32	root_inode;		/* 根目录区的 i_node 号 */
} SUPER_BLOCK;


#define NR_DEFAULT_FILE_SECS	64	/* 64 * 512 Bytes = 32 KB */
typedef struct {
	u32	i_mode;			/* Access mode */
	u32	i_size;			/* File size */
	u32	i_start_sector;		/* The first sector of the data */
	u32	i_nr_sectors;		/* How many sectors do the file occupy */
} I_NODE;

#define MAX_FILENAME_LEN	12
typedef struct {
	int	nr_inode;		/* 文件的 i_node 号 */
	char	name[MAX_FILENAME_LEN];
} DIR_ENTRY;


typedef struct {
	int	fd_mode;	/* Read or Write */
	int	fd_pos;		/* Current file pointer for R/W */
	I_NODE*	fd_inode;	/* Ptr to the i-node */
} FILE_DESC;


#define SECTOR_SIZE	512
#define BUFSIZE		(SECTOR_SIZE * 17 * 8)

#define I_MODE_DIR		(1 << 31)
#define I_MODE_CHARDEV		(1 << 30)
#define I_MODE_NORMAL		(1 << 29)


/* definition for some members of super_block */
#define NR_INODES		(SECTOR_SIZE * 8) /* inode_map 占用一个扇区, 每一个二进制位映射一个 i_node, 1 Byte = 8 bit */
#define NR_INODE_SECTORS	(NR_INODES * sizeof(I_NODE) / SECTOR_SIZE)
#define NR_SECTORS		0x4800 /* hd.img 按 9M 计算, 共 0x4800 个扇区 */
#define NR_IMAP_SECTORS		1
#define NR_SMAP_SECTORS		5 /* 一个扇区可映射 0x200*8=0x1000 个扇区, 映射 0x4800 个扇区需要 5 个扇区 */

/* definition for root dir */
#define ROOT_INODE		0	/* 根目录的 i_node 号 */
#define NR_ROOTDIR_SECTORS	1	/* 根目录占几个扇区 */

/* 各部分数据结构的起始扇区号 */
#define SUPER_BLOCK_SEC		0
#define INODE_MAP_SEC		(SUPER_BLOCK_SEC + 1)
#define SECTOR_MAP_SEC		(INODE_MAP_SEC + NR_IMAP_SECTORS)
#define INODE_ARRAY_SEC		(SECTOR_MAP_SEC + NR_SMAP_SECTORS)
#define ROOTDIR_SEC		(INODE_ARRAY_SEC + NR_INODE_SECTORS)
#define FIRST_SECTOR		(ROOTDIR_SEC + NR_ROOTDIR_SECTORS)

u8 fsbuf[BUFSIZE];
u8 imap_buf[NR_IMAP_SECTORS * SECTOR_SIZE];
u8 smap_buf[NR_SMAP_SECTORS * SECTOR_SIZE];
u8 inode_buf[NR_INODE_SECTORS * SECTOR_SIZE];
u8 dirent_buf[NR_ROOTDIR_SECTORS * SECTOR_SIZE];


/* fs/fs_main.c */
void		mkfs();
void		write_hd(int sector, void* buf, int len);
void		read_hd(int sector, void* buf, int len);

/* fs/misc.c */
int		find_file(char* filename);

/* fs/open.c */
int		open(const char* pathname, int flags);
int		do_open();
I_NODE*		create_file(char* filename, int flags);
int		alloc_imap_bit();
int		alloc_smap_bits(int nr_sectors);
I_NODE*		alloc_inode(int nr_inode, u32 mode, u32 size, u32 start_sector, u32 nr_sector);
void		alloc_dir_entry(int nr_inode, char* filename);
I_NODE*		get_inode(int nr_inode);

/* fs/close.c */
int		close(int fd);
int		do_close();
void		clear_inode(I_NODE* pin);
void		clear_fd_tbl(FILE_DESC* pfd);

#endif /* FS_H */
