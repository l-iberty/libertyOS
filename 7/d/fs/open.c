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
	msg.NAMELEN		= strlen(pathname);
	
	sendrecv(BOTH, PID_TASK_FS, &msg);
	
	return msg.FD;
}

/**
 * handle `FILE_OPEN` message
 */
int do_open()
{
	int fd = -1;
	PROCESS* pcaller = proc_table + fs_msg.source;
	
	/* dump parameters */
	int flags = fs_msg.FLAGS;
	int namelen = fs_msg.NAMELEN;
	char filename[MAX_FILENAME_LEN];
	
	if (*(char*) fs_msg.PATHNAME != '/') {
		printf("\n#ERROR#-do_open: invalid pathname: %s (pathname should start with root dir \'/\')",
			fs_msg.PATHNAME);
		return -1;
	}
	
	memcpy(va2la(proc_table + PID_TASK_FS, filename),
			va2la(pcaller, fs_msg.PATHNAME + 1), /* `+1` skip root dir `/` */
			namelen);
	filename[namelen] = 0;
	
	
	/* find a free slot in PROCESS::filp[] */
	int i;
	for (i = 0; i < NR_FILES; i++) {
		if (pcaller->filp[i] == NULL) {
			fd = i;
			break;
		}
	}
	if (fd < 0 || fd >= NR_FILES)
		halt("\n#ERROR#-do_open: No available slot in PROCESS::filp[] {PID:0x%.8x}", pcaller->pid);
	
	/* find a free slot in f_desc_table[] */
	for (i = 0; i < NR_FILES; i++) {
		if (f_desc_table[i].fd_inode == NULL)
			break;
	}
	if (i < 0 || i >= NR_FILES)
		halt("\n#ERROR#-do_open: No available slot in f_desc_table[] {PID:0x%.8x}", pcaller->pid);

	/* Now, `i` is the index of a free slot in f_desc_table[] */		
	
	int nr_inode = find_file(filename);
	
	I_NODE* pin; /* `pin` is ptr to the slot in inode_table[] */
	
	/* check whether `pcaller` tries to open a file which has already been opened */
	for (int j = 0; j < NR_FILES; j++) {
		pin = pcaller->filp[j]->fd_inode;
		if (pin) {
			if (pin->i_nr_inode == nr_inode) {
				printf("\n#ERROR#-do_open: the file \"%s\" has already been opened {PID:0x%.8x}",
						fs_msg.PATHNAME, pcaller->pid);
				return -1;
			}
		}
	}
	
	pin = NULL;
	if (flags & O_CREAT) {
		if (nr_inode != 0) {
			printf("\n#ERROR#-do_open: File exists");
			return -1;
		} else {
			pin = create_file(filename, flags);
		}
	}
	if (flags & O_RDWR) {
		/* 读写文件时文件必须已存在 */
		if (pin == NULL && nr_inode == 0)
			return -1;
	
		/* 若 pin != NULL, 说明 flags 为 (O_CREAT | O_RDWR), 无需再次调用 get_inode() */
		if (pin == NULL)
			pin = get_inode(nr_inode);
	}
	
	if (pin) {
		/* connect PROCESS::filp[] with f_desc_table[] */
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
I_NODE* create_file(char* filename, int flags)
{
	I_NODE* pin;
	int inode_idx, sec_idx, nr_inode;
    
	inode_idx = alloc_imap_bit();
	nr_inode = inode_idx + 1;
    
	sec_idx = alloc_smap_bits(NR_DEFAULT_FILE_SECS);
	
	pin = alloc_inode(nr_inode,
					I_MODE_NORMAL,			/* normal file */
					0,						/* file size*/
					sec_idx,				/* start_sector */
					NR_DEFAULT_FILE_SECS);	/* num of sectors occupied */
	
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

	for (i = 0; i < NR_IMAP_SECTORS; i++) {
		read_hd(INODE_MAP_SEC + i, imap_buf, SECTOR_SIZE);
		
		for (j = 0; j < SECTOR_SIZE; j++) {
			if (imap_buf[j] == 0xFF)
				continue; /* skip "1111_1111" */
			
			/* skip "1" bit */
			for (k = 0; (imap_buf[j] >> k) & 0x1; k++) {}
			imap_buf[j] |= (1 << k);
			
			idx = (i * SECTOR_SIZE + j) * 8 + k; /* 1 Byte = 8 bit */
			
			write_hd(INODE_MAP_SEC + i, imap_buf, SECTOR_SIZE);
			
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
int alloc_smap_bits(int nr_sectors)
{
    int idx = -1;

    int i; /* sector index */
    int j; /* byte index */
    int k; /* bit index */

    for (i = 0; i < NR_SMAP_SECTORS && nr_sectors > 0; i++) {
        read_hd(SECTOR_MAP_SEC + i, smap_buf, SECTOR_SIZE);
        
        for (j = 0; j < SECTOR_SIZE && nr_sectors > 0; j++) {
        	if (smap_buf[j] == 0xFF)
				continue; /* skip "1111_1111" */
		
			/* skip "1" bit */
			for (k = 0; (smap_buf[j] >> k) & 0x1; k++) {}
			
			if (idx == -1) {
				/* set `idx`, and lock it */
				idx = (i * SECTOR_SIZE + j) * 8 + k; /* 1 Byte = 8 bit */
			}
			
			/* fill each byte bit by bit */
			for (; k < 8; k++) {
				smap_buf[j] |= (1 << k);
				if (--nr_sectors == 0)
					break;
			}
        }
        
		write_hd(SECTOR_MAP_SEC + i, smap_buf, SECTOR_SIZE);
		
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
I_NODE* alloc_inode(int nr_inode, u32 mode, u32 size, u32 start_sector, u32 nr_sectors)
{
	I_NODE *pin, *pin2;
	
	read_hd(INODE_ARRAY_SEC, inode_buf, sizeof(inode_buf));
	
	/* find a free slot in inode_array */
	u8* pch = inode_buf;
	for (int i = 0; i < NR_INODES; i++, pch += INODE_DISK_SIZE) {
        pin = (I_NODE*) pch;
		/* if I_NODE::i_mode == 0, it's a free slot in inode_array */
		if (pin->i_mode == 0) {
			pin->i_mode			= mode;
			pin->i_size			= size;
			pin->i_start_sector	= start_sector;
			pin->i_nr_sectors	= nr_sectors;
			
			/* get a free slot in inode_table[] */
			pin2 = get_inode(0);
			/* 填充 inode_table[] 的槽位 */
			assert(pin2);
			memcpy(pin2, pin, INODE_DISK_SIZE);
			pin2->i_nr_inode = nr_inode;
			pin2->i_cnt = 1;
			
			write_hd(INODE_ARRAY_SEC, inode_buf, sizeof(inode_buf));
			
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
void alloc_dir_entry(int nr_inode, char* filename)
{
	DIR_ENTRY* pde;
	
	read_hd(ROOTDIR_SEC, dirent_buf, sizeof(dirent_buf));
	
	/* find a free slot in root dir */
	pde = (DIR_ENTRY*) dirent_buf;
	for (int i = 0; i < NR_FILES; i++, pde++) {
		/* if DIR_ENTRY::nr_inode == 0, it's a free slot in root dir */
		if (pde->nr_inode == 0) {
			pde->nr_inode = nr_inode;
			assert(strlen(filename) < MAX_FILENAME_LEN);
			strcpy(pde->name, filename);
			
			write_hd(ROOTDIR_SEC, dirent_buf, sizeof(dirent_buf));
			
			/* clear dirent_buf */
			memset(dirent_buf, 0, sizeof(dirent_buf));
			break;
		}
	}
}

/**
 * @return	ptr to the slot in inode_table[], or NULL if failed
 */
I_NODE* get_inode(int nr_inode)
{
	I_NODE* pin = NULL;
	
	for (int i = 0; i < NR_INODES; i++) {
		if (nr_inode == 0) { /* A file is being created, a free slot is needed. */
			if (inode_table[i].i_mode == 0) {
				pin = &inode_table[i];
				break;
			}
		} else { /* The file has already been created, find its i-node. */
			if (inode_table[i].i_nr_inode == nr_inode) {
				pin = &inode_table[i];
				break;
			}
		}
	}
	
	if (pin == NULL) {
		printf("\n#ERROR#-get_inode: inode_table[] is full");
	}
	else {
		pin->i_cnt++;
	}
	return pin;
}


