#ifndef CONSOLE_H
#define CONSOLE_H

#include "type.h"
#include "sysconst.h"

#define SCROLL_UP	0
#define SCROLL_DOWN	1


typedef struct {
	u32	current_start_addr;	/* 当前显示到什么位置 */
	u32	orig_addr;		/* 当前控制塔对应显存位置 */
	u32	v_mem_limit;		/* 当前控制台占的显存大小 */
	u32	cursor_pos;		/* 当前光标位置 */
} CONSOLE;

CONSOLE	console_table[NR_CONSOLES];

void select_console(int nr_console);
void scroll_screen(CONSOLE* p_con, int direction);

#endif /* CONSOLE_H */
