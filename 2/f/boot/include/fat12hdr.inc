; 常量
RootDirSecNo	equ	19		; 根目录区起始扇区号
RootDirSecCnt	equ	14		; 根目录区占用的扇区数
RootDirItemLen	equ	32		; 根目录区每个条目的长度(字节)
DirNameLen	equ	11		; 根目录区条目的 DIR_Name 字段的长度(字节)
RootDirItemCnt	equ	224		; 根目录区的条目总数 = RootDirSecCnt * [BPB_BytsPerSec] / RootDirItemLen
OffsetFileSize	equ	28		; 根目录条目中 DIR_FileSize 在条目内的偏移
OffsetFstClus	equ	26		; 根目录条目中 DIR_FstClus 在条目内的偏移
Fat1SecNo	equ	1		; FAT1 起始扇区号
DataSecNo	equ	33		; 数据区起始扇区号


; 下面是 FAT12 磁盘的头
	BS_OEMName	DB 'l-iberty'	; OEM String, 必须 8 个字节
	BPB_BytsPerSec	DW 512		; 每扇区字节数
	BPB_SecPerClus	DB 1		; 每簇多少扇区
	BPB_RsvdSecCnt	DW 1		; Boot 记录占用多少扇区
	BPB_NumFATs	DB 2		; 共有多少 FAT 表
	BPB_RootEntCnt	DW 224		; 根目录文件数最大值
	BPB_TotSec16	DW 2880		; 逻辑扇区总数
	BPB_Media	DB 0xF0		; 媒体描述符
	BPB_FATSz16	DW 9		; 每FAT扇区数
	BPB_SecPerTrk	DW 18		; 每磁道扇区数
	BPB_NumHeads	DW 2		; 磁头数(面数)
	BPB_HiddSec	DD 0		; 隐藏扇区数
	BPB_TotSec32	DD 0		; BPB_TotSec16 为 0 时这个值记录扇区数
	BS_DrvNum	DB 0		; 中断 13 的驱动器号
	BS_Reserved1	DB 0		; 未使用
	BS_BootSig	DB 29h		; 扩展引导标记 (29h)
	BS_VolID	DD 0		; 卷序列号
	BS_VolLab	DB 'libertyOS  '; 卷标, 必须 11 个字节
	BS_FileSysType	DB 'FAT12   '	; 文件系统类型, 必须 8个字节  
