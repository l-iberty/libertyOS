;================================================================
; load.inc
; 定义 MBR 加载 Loader 以及 Loader 加载 Kernel 用到的常量信息
;================================================================

KernelSeg		equ	0b40h		; KERNEL.BIN 被映射到的地址(段地址)
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
