#include "tty.h"
#include "console.h"
#include "sysconst.h"
#include "stdio.h"
#include "stdlib.h"
#include "global.h"

struct console console_table[NR_CONSOLE];
int nr_current_console;

void select_console(int nr_console)
{
	if ((nr_console < 0) || (nr_console >= NR_CONSOLE))
		return;

	nr_current_console = nr_console;
	
	set_video_start(console_table[nr_console].current_start_addr);
	set_cursor_pos(console_table[nr_console].cursor_pos);
}

/**
 * 滚动屏幕, 滚动范围在 p_con 占据的两个屏幕范围内
 */
void scroll_screen(struct console* p_con, int direction)
{
	if (direction == SCROLL_UP) 
	{
		if (p_con->current_start_addr + SCREEN_SIZE <
				p_con->orig_addr + p_con->v_mem_limit)
			p_con->current_start_addr += SCREEN_WIDTH;
	}
	else if (direction == SCROLL_DOWN) 
	{
		if (p_con->current_start_addr > p_con->orig_addr)
			p_con->current_start_addr -= SCREEN_WIDTH;
	}
	
	set_video_start(p_con->current_start_addr);
}

