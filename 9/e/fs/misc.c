#include "fs.h"
#include "hd.h"
#include "irq.h"
#include "proc.h"
#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "sysconst.h"
#include "type.h"

/**
 * 查找根目录下的文件
 *
 * @param	filename 不含路径的纯文件名
 * @return	struct dir_entry::nr_inode, or (0) if not found
 */
int find_file(char *filename) {
  int nr_inode = 0;

  /* 读入根目录 ( 1 个扇区 ) */
  READ_HD(ROOTDIR_SEC, dirent_buf, sizeof(dirent_buf));

  struct dir_entry *pde = (struct dir_entry *)dirent_buf;
  for (int i = 0; i < NR_FILES; i++, pde++) {
    if (!memcmp(filename, pde->name, strlen(filename) + 1)) {
      nr_inode = pde->nr_inode;
      break;
    }
  }

  return nr_inode;
}

/**
 * @return	ptr to the slot in inode_table[], or NULL if failed
 */
struct i_node *get_inode(int nr_inode) {
  struct i_node *pin = NULL;

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
  } else {
    pin->i_cnt++;
  }
  return pin;
}
