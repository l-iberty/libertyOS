#include "tty.h"
#include "console.h"
#include "sysconst.h"
#include "stdio.h"
#include "stdlib.h"
#include "global.h"


void select_console(int nr_console)
{
	if ((nr_console < 0) || (nr_console >= NR_CONSOLES))
		return;

	nr_current_console = nr_console;
	
	set_video_start(console_table[nr_console].current_start_addr);
	set_cursor_pos(console_table[nr_console].cursor_pos);
}

void scroll_screen(CONSOLE* p_con, int direction)
{
	if (direction == SCROLL_UP) {
		if (p_con->current_start_addr + SCREEN_WIDTH <
				p_con->current_start_addr + p_con->v_mem_limit)
			p_con->current_start_addr += SCREEN_WIDTH;
	}
	else if (direction == SCROLL_DOWN) {
		if (p_con->current_start_addr > p_con->orig_addr)
			p_con->current_start_addr -= SCREEN_WIDTH;
	}
	
	set_video_start(p_con->current_start_addr);
}

