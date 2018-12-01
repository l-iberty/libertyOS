#include "sysconst.h"
#include "type.h"
#include "stdlib.h"

extern uint32_t main_print_pos;
extern uint32_t print_pos;

/* 设置管标位置 */
void set_cursor_pos(uint32_t pos)
{
	disable_int();
	out_byte(CRTC_ADDR_REG, CURSOR_LOCATION_H);
	out_byte(CRTC_DATA_REG, (pos >> 8) & 0xFF);
	out_byte(CRTC_ADDR_REG, CURSOR_LOCATION_L);
	out_byte(CRTC_DATA_REG, pos & 0xFF);
	enable_int();
}

/* 设置显示起始位置 */
void set_video_start(uint32_t addr)
{
	disable_int();
	out_byte(CRTC_ADDR_REG, START_ADDR_H);
	out_byte(CRTC_DATA_REG, (addr >> 8) & 0xFF);
	out_byte(CRTC_ADDR_REG, START_ADDR_L);
	out_byte(CRTC_DATA_REG, addr & 0xFF);
	enable_int();
}

/* 初始化显示参数 */
void init_video()
{
	print_pos = main_print_pos = 0;
	uint8_t* p_vmem = (uint8_t*) (V_MEM_BASE);
	
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
