#ifndef CONSOLE_H
#define CONSOLE_H

#include "type.h"
#include "sysconst.h"

#define SCROLL_UP	0
#define SCROLL_DOWN	1

struct console {
    uint32_t current_start_addr; /* 当前显示到什么位置 */
    uint32_t orig_addr; /* 当前控制塔对应显存位置 */
    uint32_t v_mem_limit; /* 当前控制台占的显存大小 */
    uint32_t cursor_pos; /* 当前光标位置 */
};

extern struct console console_table[NR_CONSOLE];

void select_console(int nr_console);
void scroll_screen(struct console* p_con, int direction);

#endif // CONSOLE_H
