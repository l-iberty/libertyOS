#include "tty.h"
#include "console.h"
#include "sysconst.h"

void set_cursor_pos(u32 pos);
void disable_int();
void enable_int();
void out_byte(u16 port, u8 byte);

extern int nr_current_console;

void select_console(int nr_console)
{
	if ((nr_console < 0) || (nr_console >= NR_CONSOLES))
		return;

	nr_current_console = nr_console;
	
	set_video_start(console_table[nr_console].current_start_addr);
	set_cursor_pos(console_table[nr_console].cursor_pos);
}

void set_video_start(u32 addr)
{
	disable_int();
	out_byte(CRTC_ADDR_REG, START_ADDR_H);
	out_byte(CRTC_DATA_REG, (addr >> 8) & 0xFF);
	out_byte(CRTC_ADDR_REG, START_ADDR_L);
	out_byte(CRTC_DATA_REG, addr & 0xFF);
	enable_int();
}

