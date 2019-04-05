#include "type.h"
#include "keyboard.h"
#include "keymap.h"
#include "tty.h"
#include "console.h"
#include "proc.h"
#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "global.h"
#include "sysconst.h"

STATIC void	init_console(struct tty* p_tty);
STATIC int	is_current_console(struct tty* p_tty);
STATIC void	tty_dev_read(struct tty* p_tty, uint8_t *key);
STATIC void	tty_dev_write(struct tty* p_tty, uint8_t key);

struct tty tty_table[NR_CONSOLE];


void TaskTTY()
{
	struct tty* p_tty;
	uint8_t key;
	
	for (p_tty = tty_table; p_tty < tty_table + NR_CONSOLE; p_tty++) 
	{
		init_tty(p_tty);
	}

	select_console(0); /* 系统启动时使用 0 号控制台 */

	/**
	 * TTY 任务的主循环, 轮询每个 TTY, 如果 p_tty 对应的控制台是当前控制台
	 * 则读取对应的键盘输入缓冲并显示. 注意: 键盘是所有 TTY 共享的, 但每个
	 * TTY 拥有独立的键盘输入缓冲.
	 */
	while (1) 
	{
		for (p_tty = tty_table; p_tty < tty_table + NR_CONSOLE; p_tty++) 
		{
			tty_dev_read(p_tty, &key);
			tty_dev_write(p_tty, key);
		}
	}
}

void init_tty(struct tty* p_tty)
{
	int nr_tty;
	
	reset_kb_buf(p_tty);
	
	nr_tty = p_tty - tty_table;
	p_tty->p_console = console_table + nr_tty;
	init_console(p_tty);
}

void tty_printstr(struct tty* p_tty, char* str)
{
	int len = strlen(str);
	for (int i = 0; i < len; i++)
	{
		tty_printchar(p_tty, str[i]);
	}
}

/* 封装 backspace, 使其适用于终端环境, 不至于将终端提示符删掉 */
void tty_backspace(struct tty* p_tty)
{
	uint32_t next_cursor = p_tty->p_console->cursor_pos - 1; /* 退格后的光标位置, 不一定采用 */
	uint32_t min_cursor =
	        (p_tty->p_console->current_start_addr % SCREEN_WIDTH ) + 2; /* 终端提示符有 2 个字符 */
        
        if (p_tty->kb_in.rear > 0)
        {
        	p_tty->kb_in.buf_text[--(p_tty->kb_in.rear)] = 0;
        }
        
	if ((next_cursor % SCREEN_WIDTH) > min_cursor) 
	{
		backspace();
		p_tty->p_console->cursor_pos--;
		set_cursor_pos(print_pos >> 1);
	}
}

/* 将可打印字符放进缓冲区 */
void tty_put_text_ch(struct tty* p_tty, char ch)
{
	p_tty->kb_in.buf_text[p_tty->kb_in.rear++ % KB_BUFSIZE] = ch;
}

/* 打印终端提示符 */
void disp_tips(struct tty* p_tty)
{
	int nr_tty = p_tty - tty_table;
	
	tty_printchar(p_tty, nr_tty + '0');
	tty_printchar(p_tty, '$');
	tty_printchar(p_tty, ' ');
}

void reset_kb_buf(struct tty* p_tty)
{
	p_tty->kb_in.p_head = p_tty->kb_in.p_rear = p_tty->kb_in.buf_queue;
	p_tty->kb_in.count = 0;
	p_tty->kb_in.rear  = 0;
	memset(p_tty->kb_in.buf_queue, 0, KB_BUFSIZE);	
	memset(p_tty->kb_in.buf_text,  0, KB_BUFSIZE);
}

void parse_input(struct tty* p_tty)
{
	char *input = p_tty->kb_in.buf_text;
	
	char s[] = "hello";
	if (!strcmp(s, input)) 
	{
		tty_printstr(p_tty, "hello,world!");
	}
	else 
	{
		tty_printstr(p_tty, input);
	}
}

void tty_printchar(struct tty* p_tty, char ch)
{
	print_pos = p_tty->p_console->cursor_pos << 1;
	printchar(ch);
	p_tty->p_console->cursor_pos = print_pos >> 1;
	set_cursor_pos(p_tty->p_console->cursor_pos);
}

STATIC void init_console(struct tty* p_tty)
{
	int nr_tty = p_tty - tty_table;
	
	int con_v_mem_size			= V_MEM_SCREEN; /* V_MEM_SIZE = 2 * SCREEN_SIZE, 一个 console 有两个屏幕的显示空间 */
	p_tty->p_console->orig_addr		= nr_tty * con_v_mem_size;	
	p_tty->p_console->v_mem_limit		= con_v_mem_size;
	p_tty->p_console->current_start_addr	= p_tty->p_console->orig_addr;
	
	if (nr_tty == 0) /* 0 号控制台沿用原来的光标位置 */
	{ 
		p_tty->p_console->cursor_pos = main_print_pos >> 1;
	} 
	else 
	{
		p_tty->p_console->cursor_pos = p_tty->p_console->orig_addr;
		disp_tips(p_tty);
	}
}

STATIC int is_current_console(struct tty* p_tty)
{
	int nr_console;
	nr_console = p_tty->p_console - console_table;
	
	return (nr_console == nr_current_console);
}

STATIC void tty_dev_read(struct tty* p_tty, uint8_t *key)
{
	*key = 0;
	if (is_current_console(p_tty))
	{
		keyboard_read(p_tty, key);
	}
}

STATIC void tty_dev_write(struct tty* p_tty, uint8_t key)
{
	if (key == 0) return;
	
	int nr_console = p_tty - tty_table;
	
	switch (key & MAKE_MASK) 
	{
		case MC_BACKSPACE:
		{
			if (nr_console > 0) /* 0 号控制台不响应 Backspace */
			{ 
				tty_backspace(p_tty);
			}
			break;
		}
		case MC_SHIFT_L:
		{
			scroll_screen(p_tty->p_console, SCROLL_DOWN);
			break;
		}
		case MC_SHIFT_R:
		{
			scroll_screen(p_tty->p_console, SCROLL_UP);
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
			nr_console = (key & MAKE_MASK) - MC_F1;
			select_console(nr_console);
			break;
		}
		case MC_ESC:
		{
			init_video();
			for (p_tty = tty_table; p_tty < tty_table + NR_CONSOLE; p_tty++) 
			{
				init_tty(p_tty);
			}
			break;
		}
		default:
		{
			if (nr_console > 0)
			{
				char c = keymap[key & MAKE_MASK];
				if (c & TEXT_MASK) /* 判断是否是可打印字符 */
				{ 
					
					char ch = c & ~TEXT_MASK;
					tty_printchar(p_tty, ch);
					/* (c & ~TEXT_MASK) 得到的是去除 TEXT_MASK 掩码的 ASCII 字符 */
					if (ch == '\n') 
					{
						parse_input(p_tty);
						reset_kb_buf(p_tty);
						tty_printchar(p_tty, '\n');
						disp_tips(p_tty);
					}
					else
					{
						tty_put_text_ch(p_tty, ch);
					}
					
				}
			}
		} /* default */
	} /* switch */
}

