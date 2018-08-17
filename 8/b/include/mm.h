#ifndef MM_H
#define MM_H

#include "type.h"

#define PAGE_SIZE		0x1000		/* 4K */
#define MAX_PAGE_ITEM		(PAGE_SIZE / 4)	/* 一个页目录/页表最多有几项? */
#define PAGE_MAPPING_SIZE	0x400000	/* 一个页表的内存映射范围: 4M */
#define PAGE_USED		1
#define PAGE_FREE		(PAGE_USED << 1)
#define PAGE_RESERVED		(PAGE_FREE << 1)
#define PAGE_READ		1
#define PAGE_READWRITE		(PAGE_READ << 1)

#define ROUND_UP(a,b)		((((uint32_t)a + (uint32_t)b - 1) / (uint32_t)b) * (uint32_t)b)
#define ROUND_DOWN(a,b)		(((uint32_t)a / (uint32_t)b) * (uint32_t)b)

/* 从线性地址 la 获得页目录索引、页表索引和页内偏移 */
#define PDE_INDEX(la)		((la >> 22) & 0x3FF)
#define PTE_INDEX(la)		((la >> 12) & 0x3FF)
#define PAGE_OFFSET(la)		(la & 0xFFF)

/* 从 PDE 获得页表基地址, 或从 PTE 获得物理页基地址 */
#define GET_BASE(x)		(x & 0xFFFFF000)

/* PDE/PTE 属性位 */
#define PG_P			0x1 /* Present */
#define PG_RWR			0x0 /* Read */
#define PG_RWW			0x2 /* Read/Write */
#define PG_USS			0x0 /* Supervisor */
#define PG_USU			0x4 /* User */

#define MEM_INFO_VA_BASE	0x8000 /* loader 将 struct meminfo 描述的内存信息放在该虚拟地址处 */
#define MAX_PM_BLOCK		8

struct meminfo
{
	uint32_t page_dir_base; /* 页目录物理基地址 */
	uint32_t page_tbl_base; /* 页表物理基地址 */
	int nr_pde;	   /* 页目录项个数 */
	int nr_pm_block;   /* 可用的物理内存块个数 */
	struct
	{
		uint32_t avail_base;
		uint32_t avail_size;
	} pm_block_info[MAX_PM_BLOCK];  /* 描述 nr_pm_block 个物理内存块 */
};

/* 双向循环列表 */
struct link_list
{
	struct link_list *prev;
	struct link_list *next;
};

/* 描述虚页/物理页(页框)的数据结构 */
struct page_area
{
	int ref;	/* 页面的引用计数 */
	uint32_t base;	/* 虚页的虚拟基地址 or 页框的物理基地址 */
	uint32_t type;	/* used, free, or reserved */
	uint32_t protect;	/* 保护属性: 可读, 可写, 可执行 */
};

/* 用于管理虚页/物理页(页框)的双向循环列表 */
struct page_list
{
	struct link_list link_list;
	struct page_area page_area;
};

#define NEXT	link_list.next
#define PREV	link_list.prev
#define REF	page_area.ref
#define BASE	page_area.base
#define TYPE	page_area.type
#define PROTECT	page_area.protect

extern struct message   mm_msg;
extern struct page_list *pf_list;
extern struct meminfo   *mi;

void	init_mm();
void	*vm_alloc(void *vm_addr, uint32_t vm_size, uint32_t vm_protect);
void	*do_vm_alloc();

#endif // MM_H
