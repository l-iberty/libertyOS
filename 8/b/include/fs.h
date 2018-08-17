#ifndef FS_H
#define FS_H

#include "type.h"

struct super_block
{
	uint32_t	nr_inodes;		/* 文件系统最多允许有多少个 i-node */
	uint32_t	nr_inode_sectors;	/* inode_array 占用多少个扇区 */
	uint32_t	nr_sectors;		/* 文件系统总共扇区数 */
	uint32_t	nr_imap_sectors;	/* inode_map 占用多少扇区 */
	uint32_t	nr_smap_sectors;	/* sector_map 占用多少扇区 */
	uint32_t	first_sector;		/* 第一个文件数据扇区的扇区号 */
	uint32_t	root_inode;		/* 根目录区的 i_node 号 */
};


#define NR_DEFAULT_FILE_SECS	64	/* 64 * 512 Bytes = 32 KB */
struct i_node
{
	uint32_t	i_mode;			/* Access mode */
	uint32_t	i_size;			/* File size */
	uint32_t	i_start_sector;		/* The first sector of the data */
	uint32_t	i_nr_sectors;		/* How many sectors does the file occupy */
        
        /* The following members only exist in memory */
        uint32_t	i_nr_inode;             /* nr_inode of the file recorded in root_dir entry */
        uint32_t	i_cnt;			/* How many procs share this i-node */
};
#define INODE_DISK_SIZE         16

#define MAX_FILENAME_LEN	12
struct dir_entry
{
	int	nr_inode;		/* 文件的 i_node 号 */
	char	name[MAX_FILENAME_LEN];
};


struct file_desc
{
	int		fd_mode;	/* Read or Write */
	int		fd_pos;		/* Current file pointer for R/W */
	struct i_node*	fd_inode;	/* Ptr to the i-node */
};


#define SECTOR_SIZE		512
#define SECTOR_SIZE_SHIFT	9
#define BUFSIZE			(SECTOR_SIZE * 17 * 8)

#define I_MODE_DIR		(1 << 31)
#define I_MODE_CHARDEV		(1 << 30)
#define I_MODE_NORMAL		(1 << 29)


/* definition for some members of super_block */
#define NR_INODES		(SECTOR_SIZE * 8) /* inode_map 占用一个扇区, 每一个二进制位映射一个 i_node, 1 Byte = 8 bit */
#define NR_INODE_SECTORS	(NR_INODES * INODE_DISK_SIZE / SECTOR_SIZE)
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

extern uint8_t fsbuf[BUFSIZE];
extern uint8_t imap_buf[NR_IMAP_SECTORS * SECTOR_SIZE];
extern uint8_t smap_buf[NR_SMAP_SECTORS * SECTOR_SIZE];
extern uint8_t inode_buf[NR_INODE_SECTORS * SECTOR_SIZE];
extern uint8_t dirent_buf[NR_ROOTDIR_SECTORS * SECTOR_SIZE];

extern struct file_desc	f_desc_table[NR_FILES];
extern struct i_node	inode_table[NR_INODES];
extern struct message   fs_msg;

/* fs/fs_main.c */
void		init_fs();
void		mkfs();
void		write_hd(int sector, void* buf, int len);
void		read_hd(int sector, void* buf, int len);

/* fs/misc.c */
int		find_file(char* filename);

/* fs/open.c */
int		open(const char* pathname, int flags);
int		do_open();
int		alloc_imap_bit();
int		alloc_smap_bits(int nr_sectors);
void		alloc_dir_entry(int nr_inode, char* filename);
struct i_node*	alloc_inode(int nr_inode, uint32_t mode, uint32_t size, uint32_t start_sector, uint32_t nr_sector);
struct i_node*	create_file(char* filename, int flags);
struct i_node*	get_inode(int nr_inode);

/* fs/close.c */
int		close(int fd);
int		do_close();
void		clear_inode(struct i_node* pin);
void		clear_fd_tbl(struct file_desc* pfd);

/* fs/rdwt.c */
size_t		read(int fd, void* buf, size_t len);
size_t		write(int fd, void* buf, size_t len);
size_t		do_rdwt();

/* fs/unlink.c */
int		unlink(const char* pathname);
int		do_unlink();

#endif /* FS_H */
