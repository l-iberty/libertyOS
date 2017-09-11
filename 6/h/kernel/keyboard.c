#include "type.h"
#include "keyboard.h"
#include "keymap.h"
#include "tty.h"
#include "console.h"
#include "proc.h"
#include "stdio.h"
#include "stdlib.h"
#include "global.h"

extern TTY	tty_table[NR_CONSOLES];
extern CONSOLE	console_table[NR_CONSOLES];

TTY* p_current_tty;

void keyboard_handler()
{
	u8 scan_code = in_byte(0x60);
	
	p_current_tty = tty_table + nr_current_console;
	
	if (scan_code & BREAK_MASK) /* 仅接收 Break Code */
	{
		if (p_current_tty->kb_in.count < KB_BUFSIZE)
		{
			*p_current_tty->kb_in.p_tail++ = scan_code;
			p_current_tty->kb_in.count++;
			if (p_current_tty->kb_in.p_tail >= p_current_tty->kb_in.buf_queue + KB_BUFSIZE)
			{
				/* 队列已满, 回绕 */
				p_current_tty->kb_in.p_tail = p_current_tty->kb_in.buf_queue;
			}
		}
	}
}

void keyboard_read()
{
	u8 scan_code;
	
	int nr_console = p_current_tty - tty_table;
	
	if (p_current_tty->kb_in.count > 0)
	{
		scan_code = *p_current_tty->kb_in.p_head++;
		p_current_tty->kb_in.count--;
		if (p_current_tty->kb_in.p_head == p_current_tty->kb_in.buf_queue + KB_BUFSIZE)
		{
			p_current_tty->kb_in.p_head = p_current_tty->kb_in.buf_queue;
		}
		
		switch (scan_code & MAKE_MASK) {
			case MC_BACKSPACE:
			{
				if (nr_console > 0) { /* 0 号控制台不响应 Backspace */
					tty_backspace(p_current_tty);
				}
				break;
			}
			case MC_F1:
			case MC_F2:
			case MC_F3:
			case MC_F4:
			case MC_F5:
			case MC_F6:
			case MC_F7:
			case MC_F8:
			case MC_F9:
			case MC_F10:
			{
				/* 切换控制台 */
				nr_console = (scan_code & MAKE_MASK) - MC_F1;
				select_console(nr_console);
				break;
			}
			case MC_ESC:
			{
				init_video();
				/* 初始化所有 TTY */
				for (TTY* p_tty = tty_table; p_tty < tty_table + NR_CONSOLES; p_tty++)
				{
					init_tty(p_tty);
				}
			}
			default:
			{
				if (nr_console > 0) {
					char ch = keymap[scan_code & MAKE_MASK];
					if (ch & TEXT_MASK) { /* 判断是否是可打印字符 */
						tty_printchar(p_current_tty, (ch & (~TEXT_MASK)));
						/* (ch & (~TEXT_MASK)) 得到的是去除 TEXT_MASK 掩码的 ASCII 字符 */
					}
				}
			}
		}
	}	
}


