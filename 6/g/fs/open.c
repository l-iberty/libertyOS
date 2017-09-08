#include "proc.h"
#include "fs.h"
#include "type.h"
#include "sysconst.h"
#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "global.h"

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
	MESSAGE msg;
	
	msg.value		= FILE_OPEN;
	msg.PATHNAME	= (void*) pathname;
	msg.FLAGS		= flags;
	msg.NAMELEN		= _strlen(pathname);
	
	sendrecv(BOTH, PID_TASK_FS, &msg);
	
	return msg.FD;
}

/**
 * Open a file and return a fd.
 */
int do_open()
{
	int fd = -1;
	
	_printf("\npathname: %s, flags: 0x%.8x, namelen: 0x%.8x, caller pid: 0x%.4x",
		fs_msg.PATHNAME, fs_msg.FLAGS, fs_msg.NAMELEN, fs_msg.src_pid);
		
	PROCESS* pcaller = proc_table + fs_msg.src_pid;
	
	int flags = fs_msg.FLAGS;
	int namelen = fs_msg.NAMELEN;
	char filename[MAX_FILENAME_LEN];
	
	if (*(char*) fs_msg.PATHNAME != '/')
	{
		halt("invalid pathname: %s (pathname should start with root dir \'/\')", fs_msg.PATHNAME);
	}
	
	_memcpy(va2la(proc_table + PID_TASK_FS, filename),
			va2la(pcaller, fs_msg.PATHNAME + 1), /* `+1` skip root dir `/` */
			namelen);
	filename[namelen] = 0;
	
	
	/* Find a free slot in PROCESS::filp[] */
	int i;
	for (i = 0; i < NR_FILES; i++)
	{
		if (pcaller->filp[i] == NULL)
		{
			fd = i;
			break;
		}
	}
	if (fd < 0 || fd >= NR_FILES)
		halt("\nNo available slot in PROCESS::filp[] {PID:0x%.8x}", pcaller->pid);
	
	/* Find a free slot in f_desc_table[] */
	for (i = 0; i < NR_FILES; i++)
	{
		if (f_desc_table[i].fd_inode == NULL)
			break;
	}
	if (i < 0 || i >= NR_FILES)
		halt("\nNo available slot in f_desc_table[] {PID:0x%.8x}", pcaller->pid);

	/* Now, `i` is the index of a free slot in f_desc_table[] */		
	
	int nr_inode = find_file(filename);
	
	I_NODE* pin = NULL;	
	if (flags & O_CREAT)
	{
		if (nr_inode != -1)
		{
			_printf("\nFile exists");
			return -1;
		}
		else
		{
			pin = create_file(filename, flags);
		}
	}
	
	if (pin)
	{
		/* connect PROCESS::filp[] with f_desc_table[] */
		pcaller->filp[fd] = &f_desc_table[i];
		
		/* connect f_desc_table[] with inode_table[] */
		f_desc_table[i].fd_inode = pin;
		
		f_desc_table[i].fd_mode = flags;
		f_desc_table[i].fd_pos = 0;
	}
	
	return fd;
}

I_NODE* create_file(char* filename, int flags)
{
	I_NODE* pin;
	int inode_idx, sec_idx, nr_inode;
    
	inode_idx = alloc_imap_bit();
	nr_inode = inode_idx + 1;
    
	sec_idx = alloc_smap_bit();
	
	pin = alloc_inode(I_MODE_NORMAL, 0, sec_idx, 0);
	
	alloc_dir_entry(nr_inode, filename);
	
	return pin;
}

/**
 * Allocate a bit in inode_map.
 *
 * @return index of the allocated bit in inode_map, or (-1) if failed.
 */
int alloc_imap_bit()
{
	int idx = -1;
	
	int i; /* sector index */
	int j; /* byte index */
	int k; /* bit index */

	for (i = 0; i < NR_IMAP_SECTORS; i++)
	{
		read_hd(INODE_MAP_SEC + i, imap_buf, SECTOR_SIZE);
		
		for (j = 0; j < SECTOR_SIZE; j++)
		{
			if (imap_buf[j] == 0xFF)
				continue; /* skip "1111_1111" */
			
			/* skip "1" bit */
			for (k = 0; (imap_buf[j] >> k) & 0x1; k++) {}
			imap_buf[j] |= (1 << k);
			
			idx = (i * SECTOR_SIZE + j) * 8 + k; /* 1 Byte = 8 bit */
			
			write_hd(INODE_MAP_SEC + i, imap_buf, SECTOR_SIZE);
			
			/* clear imap_buf */
			_memset(imap_buf, 0, sizeof(imap_buf));
			
			return idx;
		}
	}
	return idx;
}


/**
 * Allocate a bit in sector_map.
 *
 * @return index of allocated bit in sector_map, or (-1) if failed.
 */
int alloc_smap_bit()
{
    int idx = -1;

    int i; /* sector index */
    int j; /* byte index */
    int k; /* bit index */

    for (i = 0; i < NR_SMAP_SECTORS; i++)
    {
        read_hd(SECTOR_MAP_SEC + i, smap_buf, SECTOR_SIZE);
        
        for (j = 0; j < SECTOR_SIZE; j++)
        {
        	if (smap_buf[j] == 0xFF)
				continue; /* skip "1111_1111" */
		
			/* skip "1" bit */
			for (k = 0; (smap_buf[j] >> k) & 0x1; k++) {}
			smap_buf[j] |= (1 << k);
		
			idx = (i * SECTOR_SIZE + j) * 8 + k; /* 1 Byte = 8 bit */
			
			write_hd(SECTOR_MAP_SEC + i, smap_buf, SECTOR_SIZE);
			
			/* clear smap_buf */
			_memset(smap_buf, 0, sizeof(smap_buf));
			
			return idx;
        }
    }
    return idx;
}

/**
 * Use the parameters to create a new inode in inode_array.
 *
 * @return ptr to inode
 */
I_NODE* alloc_inode(u32 mode, u32 size, u32 start_sector, u32 nr_sectors)
{
	I_NODE *pin, *pin2;
	
	read_hd(INODE_ARRAY_SEC, inode_buf, NR_INODE_SECTORS * SECTOR_SIZE);
	
	/* find a free slot in inode_array */
	pin = (I_NODE*) inode_buf;
	for (int i = 0; i < NR_INODES; i++, pin++)
	{
		/* if I_NODE::i_mode == 0, it's a free slot in inode_array */
		if (pin->i_mode == 0)
		{
			pin->i_mode			= mode;
			pin->i_size			= size;
			pin->i_start_sector	= start_sector;
			pin->i_nr_sectors	= nr_sectors;
			
			pin2 = get_inode();
			assert(pin2);
			/* 填充 inode_table[] 的槽位 */
			_memcpy(pin2, pin, sizeof(I_NODE));
			
			write_hd(INODE_ARRAY_SEC, inode_buf, NR_INODE_SECTORS * SECTOR_SIZE);
			
			/* clear inode_buf */
			_memset(inode_buf, 0, sizeof(inode_buf));
			
			return pin2;
		}
	}
	return NULL;
}

/**
 * Use the parameters to create a dir_entry in root dir.
 */
void alloc_dir_entry(int nr_inode, char* filename)
{
	DIR_ENTRY* pde;
	
	read_hd(ROOTDIR_SEC, dirent_buf, NR_ROOTDIR_SECTORS * SECTOR_SIZE);
	
	/* find a free slot in root dir */
	pde = (DIR_ENTRY*) dirent_buf;
	for (int i = 0; i < NR_FILES; i++, pde++)
	{
		/* if DIR_ENTRY::nr_inode == 0, it's a free slot in root dir */
		if (pde->nr_inode == 0)
		{
			pde->nr_inode = nr_inode;
			assert(_strlen(filename) < MAX_FILENAME_LEN);
			_strcpy(pde->name, filename);
			
			write_hd(ROOTDIR_SEC, dirent_buf, NR_ROOTDIR_SECTORS * SECTOR_SIZE);
			
			/* clear dirent_buf */
			_memset(dirent_buf, 0, sizeof(dirent_buf));
			break;
		}
	}
}

/**
 * Find a free slot in inode_table[].
 *
 * @return ptr to the available slot, or NULL if failed
 */
I_NODE* get_inode()
{
	for (int i = 0; i < NR_INODES; i++)
	{
		if (inode_table[i].i_mode == 0)
			return &inode_table[i];
	}
	return NULL;
}


