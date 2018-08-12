#ifndef MM_H
#define MM_H

#include "type.h"


#define PROCS_BASE		0xA00000 /* 10 MB */
#define PROC_IMAGE_SIZE		0x100000 /* 1 MB */

#define PAGE_SIZE		0x1000 /* 4K */
#define PAGE_USED		1
#define PAGE_FREE		(PAGE_USED << 1)
#define PAGE_RESERVED		(PAGE_FREE << 1)
#define PAGE_READ		1
#define PAGE_WRITE		(PAGE_READ << 1)
#define PAGE_EXECUTE		(PAGE_WRITE << 1)

#define ROUND_UP(a,b)		((((a)+(b)-1)/(b))*(b))
#define ROUND_DOWN(a,b)		(((a)/(b))*(b))

#define MEM_INFO_VA_BASE	0x8000 /* loader 将 MEMINFO 描述的内存信息放在该虚拟地址处 */

typedef struct
{
	u32 avail_pm_base; /* 可用的物理内存起始地址 */
	u32 avail_pm_size; /* 可用的物理内存大小 */
	u32 page_dir_base; /* 页目录物理基地址 */
	u32 page_tbl_base; /* 页表物理基地址 */
	u32 nr_pde;	   /* 页目录项个数 */
} MEMINFO;

/* 双向循环列表 */
typedef struct LIST_NODE
{
	struct LIST_NODE *prev;
	struct LIST_NODE *next;
} LIST_NODE, *LINK_LIST;

/* 描述虚页/物理页(页框)的数据结构 */
typedef struct
{
	int ref;	/* 页面的引用计数 */
	u32 base;	/* 虚页的虚拟基地址 or 页框的物理基地址 */
	u32 type;	/* used, free, or reserved */
	u32 protect;	/* 保护属性: 可读, 可写, 可执行 */
} PAGE_AREA;

/* 用于管理虚页/物理页(页框)的双向循环列表 */
typedef struct
{
	LIST_NODE link_list;
	PAGE_AREA page_area;
} PAGE, PAGE_FRAME;

#define NEXT	link_list.next
#define PREV	link_list.prev
#define REF	page_area.ref
#define BASE	page_area.base
#define TYPE	page_area.type
#define PROTECT	page_area.protect


void	init_mm();
u32 	fork();
void	*vm_alloc(void *vm_addr, u32 vm_size, u32 vm_protect);
u32 	do_fork();
void	*do_vm_alloc();

u32	alloc_mem();

#endif // MM_H
