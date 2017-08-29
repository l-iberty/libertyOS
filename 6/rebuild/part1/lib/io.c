#include "sysconst.h"
#include "type.h"

void disable_int();
void enable_int();
void out_byte(u16 port, u8 byte);
void _memset(void* pDst, u8 value, int len);

extern u32 MainPrintPos;
extern u32 PrintPos;

/* 设置管标位置 */
void set_cursor_pos(u32 pos)
{
	disable_int();
	out_byte(CRTC_ADDR_REG, CURSOR_LOCATION_H);
	out_byte(CRTC_DATA_REG, (pos >> 8) & 0xFF);
	out_byte(CRTC_ADDR_REG, CURSOR_LOCATION_L);
	out_byte(CRTC_DATA_REG, pos & 0xFF);
	enable_int();
}

/* 初始化显示参数 */
void init_video()
{
	PrintPos = MainPrintPos = 0;
	u8* p_vmem = (u8*) (V_MEM_BASE);
	
	for (int i = 0; i < (V_MEM_SIZE >> 1); i++)
	{
		*p_vmem++ = 0x20;
		*p_vmem++ = 0x07;
		/**
		 * 使用 Bochs 查看内存可知, "空白"显存的内容为:
		 * 0x20 0x07 0x20 0x07 0x20 0x07 ...
		 */
	}
}
