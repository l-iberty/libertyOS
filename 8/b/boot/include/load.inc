;================================================================
; load.inc
; 定义 MBR 加载 Loader 以及 Loader 加载 Kernel 用到的常量信息
;================================================================

KernelSeg		equ	0c00h		; KERNEL.BIN 被映射到的地址(段地址)
KernelOff		equ	0		; KERNEL.BIN 被映射到的地址(偏移地址)
KernelBasePhyAddr	equ	KernelSeg << 4

KernelEntryPointPAddr	equ	1b400h		; Kernel 的入口物理地址
KernelBaseAddr		equ	1b000h		; Kernel 的加载基地址

LoaderSeg		equ	0		; LOADER.BIN 被映射到的地址(段地址)
LoaderOff		equ	0ae00h		; LOADER.BIN 被映射到的地址(偏移地址)
LoaderBasePhyAddr	equ	LoaderSeg << 4

BaseOfFatBuf		equ	7e00h		; FAT1 专用读缓冲区基地址 (len = 9 * 512 = 0x1200)
BaseOfRootBuf		equ	9000h		; 根目录区专用读缓冲区基地址 (len = 14 * 512 = 0x1c00)
BaseOfExBuf		equ	0ac00h		; 扩展缓冲区基地址 (len = 0x200)

; loader 将内存信息保存到虚拟地址 8000h 处, 便于 kernel 的内存管理模块获取.
; 一下定义需要与 include/mm.h 保持一致!

MEM_INFO_VA_BASE	equ	8000h

; 组织内存信息的结构体定义如下:
;
;	struct MEMINFO
;	{
;		u32 avail_pm_base; 可用的物理内存起始地址
;		u32 avail_pm_size; 可用的物理内存大小
;		u32 page_dir_base; 页目录物理基地址
;		u32 page_tbl_base; 页表物理基地址
;		u32 nr_pde;	   页目录项个数
;	}

; MEMINFO 成员变量的偏移:
AVAIL_PM_BASE		equ	0
AVAIL_PM_SIZE		equ	AVAIL_PM_BASE + 4
PAGE_DIR_BASE		equ	AVAIL_PM_SIZE + 4
PAGE_TBL_BASE		equ	PAGE_DIR_BASE + 4
NR_PDE			equ	PAGE_TBL_BASE + 4


