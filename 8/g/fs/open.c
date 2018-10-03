#include "proc.h"
#include "fs.h"
#include "hd.h"
#include "type.h"
#include "sysconst.h"
#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "global.h"

STATIC int		alloc_imap_bit();
STATIC int		alloc_smap_bits(int nr_sectors);
STATIC void		alloc_dir_entry(int nr_inode, char* filename);
STATIC struct i_node*	alloc_inode(int nr_inode, uint32_t mode, uint32_t size, uint32_t start_sector, uint32_t nr_sector);
STATIC struct i_node*	create_file(char* filename, int flags);

/**
 * open/create a file.
 *
 * @param pathname	The full path of file
 * @param flags		O_CREAT, O_RDWR, etc
 *
 * @return file descriptor if successful, otherwise -1
 */
int open(const char* pathname, int flags)
{
	struct message msg;
	
	msg.value	= FILE_OPEN;
	msg.PATHNAME	= (void*) pathname;
	msg.FLAGS	= flags;
	msg.NAMELEN	= strlen(pathname);
	
	sendrecv(BOTH, PID_TASK_FS, &msg);
	
	return msg.FD;
}

int do_open()
{
	int i, j;
	int fd = -1;
	struct proc* pcaller = proc_table + fs_msg.source;
	
	/* dump parameters */
	int flags = fs_msg.FLAGS;
	int namelen = fs_msg.NAMELEN;
	char filename[MAX_FILENAME_LEN];
	
	if (*(char*) fs_msg.PATHNAME != '/') 
	{
		printf("\n#ERROR#-do_open: invalid pathname: %s (pathname should start with root dir \'/\')",
			fs_msg.PATHNAME);
		return -1;
	}
	
	memcpy(va2la(proc_table + PID_TASK_FS, filename),
	       va2la(pcaller, fs_msg.PATHNAME + 1), /* `+1` skip root dir `/` */
	       namelen);
	filename[namelen] = 0;
	
	/* find a free slot in struct proc::filp[] */
	for (i = 0; i < NR_FILES; i++) 
	{
		if (pcaller->filp[i] == NULL) 
		{
			fd = i;
			break;
		}
	}
	if (fd < 0 || fd >= NR_FILES)
		panic("\n#ERROR#-do_open: No available slot in struct proc::filp[] {PID:%d}", pcaller->pid);
	
	/* find a free slot in f_desc_table[] */
	for (i = 0; i < NR_FILES; i++) 
	{
		if (f_desc_table[i].fd_inode == NULL)
			break;
	}
	if (i < 0 || i >= NR_FILES)
		panic("\n#ERROR#-do_open: No available slot in f_desc_table[] {PID:%d}", pcaller->pid);

	/* Now, `i` is the index of a free slot in f_desc_table[] */		
	
	int nr_inode = find_file(filename);
	
	struct i_node* pin; /* `pin` is ptr to the slot in inode_table[] */
	
	/* check whether `pcaller` tries to open a file which has already been opened */
	for (j = 0; j < NR_FILES; j++) 
	{
		if (pcaller->filp[j])
		{
			pin = pcaller->filp[j]->fd_inode;
			if (pin) 
			{
				if (pin->i_nr_inode == nr_inode) 
				{
					printf("\n#ERROR#-do_open: the file \"%s\" has already been opened {PID:%d}",
						fs_msg.PATHNAME, pcaller->pid);
					return -1;
				}
			}
		}
		
	}

	pin = NULL;
	if (flags & O_CREAT) 
	{
		if (nr_inode != 0) 
		{
			printf("\n#ERROR#-do_open: File exists");
			return -1;
		} 
		else 
		{
			pin = create_file(filename, flags);
		}
	}
	if (flags & O_RDWR) 
	{
		/* 读写文件时文件必须已存在 */
		if (pin == NULL && nr_inode == 0)
			return -1;
	
		/* 若 pin != NULL, 说明 flags 为 (O_CREAT | O_RDWR), 无需再次调用 get_inode() */
		if (pin == NULL)
			pin = get_inode(nr_inode);
	}
	
	if (pin) 
	{
		/* connect struct proc::filp[] with f_desc_table[] */
		pcaller->filp[fd] = &f_desc_table[i];
		
		/* connect f_desc_table[] with inode_table[] */
		f_desc_table[i].fd_inode = pin;
		
		f_desc_table[i].fd_mode = flags;
		f_desc_table[i].fd_pos = 0;
	}
	
	return fd;
}

/**
 * 创建一个文件的步骤:
 * 1. 在 inode_map 中分配一位 - alloc_imap_bit()
 * 2. 在 sector_map 中分配多位, 从而为文件数据分配扇区 - alloc_smap_bits()
 * 3. 在 inode_array 中创建一个 inode - alloc_inode()
 * 4. 在 root dir 中创建一个目录项 - alloc_dir_entry()
 */
STATIC struct i_node* create_file(char* filename, int flags)
{
	struct i_node* pin;
	int inode_idx, sec_idx, nr_inode;
    
	inode_idx = alloc_imap_bit();
	nr_inode = inode_idx + 1;
    
	sec_idx = alloc_smap_bits(NR_DEFAULT_FILE_SECS);
	
	pin = alloc_inode(nr_inode,
			 I_MODE_NORMAL,		/* normal file */
			 0,			/* file size*/
			 sec_idx,		/* start_sector */
			 NR_DEFAULT_FILE_SECS);	/* num of sectors occupied */
	
	alloc_dir_entry(nr_inode, filename);
	
	return pin;
}

/**
 * Allocate a bit in inode_map.
 *
 * @return index of the allocated bit in inode_map, or (-1) if failed.
 */
STATIC int alloc_imap_bit()
{
	int idx = -1;
	
	int i; /* sector index */
	int j; /* byte index */
	int k; /* bit index */

	for (i = 0; i < NR_IMAP_SECTORS; i++) 
	{
		READ_HD(INODE_MAP_SEC + i, imap_buf, SECTOR_SIZE);
		
		for (j = 0; j < SECTOR_SIZE; j++) 
		{
			if (imap_buf[j] == 0xFF)
				continue; /* skip "1111_1111" */
			
			/* skip "1" bit */
			for (k = 0; (imap_buf[j] >> k) & 0x1; k++) {}
			imap_buf[j] |= (1 << k);
			
			idx = (i * SECTOR_SIZE + j) * 8 + k; /* 1 Byte = 8 bit */
			
			WRITE_HD(INODE_MAP_SEC + i, imap_buf, SECTOR_SIZE);
			
			/* clear imap_buf */
			memset(imap_buf, 0, sizeof(imap_buf));
			
			return idx;
		}
	}
	return idx;
}


/**
 * Allocate `nr_sectors` bits in sector_map.
 *
 * @return index of the 1st allocated bit in sector_map, or (-1) if failed.
 */
STATIC int alloc_smap_bits(int nr_sectors)
{
    int idx = -1;

    int i; /* sector index */
    int j; /* byte index */
    int k; /* bit index */

    for (i = 0; i < NR_SMAP_SECTORS && nr_sectors > 0; i++) 
    {
        READ_HD(SECTOR_MAP_SEC + i, smap_buf, SECTOR_SIZE);
        
        for (j = 0; j < SECTOR_SIZE && nr_sectors > 0; j++) 
        {
        	if (smap_buf[j] == 0xFF)
				continue; /* skip "1111_1111" */
		
		/* skip "1" bit */
		for (k = 0; (smap_buf[j] >> k) & 0x1; k++) {}
		
		if (idx == -1) 
		{
			/* set `idx`, and lock it */
			idx = (i * SECTOR_SIZE + j) * 8 + k; /* 1 Byte = 8 bit */
		}
		
		/* fill each byte bit by bit */
		for (; k < 8; k++) 
		{
			smap_buf[j] |= (1 << k);
			if (--nr_sectors == 0)
				break;
		}
        }
	WRITE_HD(SECTOR_MAP_SEC + i, smap_buf, SECTOR_SIZE);
	
	/* clear smap_buf */
	memset(smap_buf, 0, sizeof(smap_buf));
    }
    return idx;
}

/**
 * Use the parameters to create a new inode in inode_array.
 *
 * @return ptr to the slot in inode_table[]
 */
STATIC struct i_node* alloc_inode(int nr_inode, uint32_t mode, uint32_t size, uint32_t start_sector, uint32_t nr_sectors)
{
	struct i_node *pin, *pin2;
	
	READ_HD(INODE_ARRAY_SEC, inode_buf, sizeof(inode_buf));
	
	/* find a free slot in inode_array */
	uint8_t* pch = inode_buf;
	for (int i = 0; i < NR_INODES; i++, pch += INODE_DISK_SIZE) 
	{
		pin = (struct i_node*) pch;
		/* if struct i_node::i_mode == 0, it's a free slot in inode_array */
		if (pin->i_mode == 0) 
		{
			pin->i_mode		= mode;
			pin->i_size		= size;
			pin->i_start_sector	= start_sector;
			pin->i_nr_sectors	= nr_sectors;
			
			/* get a free slot in inode_table[] */
			pin2 = get_inode(0);
			/* 填充 inode_table[] 的槽位 */
			assert(pin2);
			memcpy(pin2, pin, INODE_DISK_SIZE);
			pin2->i_nr_inode = nr_inode;
			pin2->i_cnt = 1;
			
			WRITE_HD(INODE_ARRAY_SEC, inode_buf, sizeof(inode_buf));
			
			/* clear inode_buf */
			memset(inode_buf, 0, sizeof(inode_buf));
			
			return pin2;
		}
	}
	return NULL;
}

/**
 * Use the parameters to create a dir_entry in root dir.
 */
STATIC void alloc_dir_entry(int nr_inode, char* filename)
{
	struct dir_entry* pde;
	
	READ_HD(ROOTDIR_SEC, dirent_buf, sizeof(dirent_buf));
	
	/* find a free slot in root dir */
	pde = (struct dir_entry*) dirent_buf;
	for (int i = 0; i < NR_FILES; i++, pde++) 
	{
		/* if struct dir_entry::nr_inode == 0, it's a free slot in root dir */
		if (pde->nr_inode == 0) 
		{
			pde->nr_inode = nr_inode;
			assert(strlen(filename) < MAX_FILENAME_LEN);
			strcpy(pde->name, filename);
			
			WRITE_HD(ROOTDIR_SEC, dirent_buf, sizeof(dirent_buf));
			
			/* clear dirent_buf */
			memset(dirent_buf, 0, sizeof(dirent_buf));
			break;
		}
	}
}


