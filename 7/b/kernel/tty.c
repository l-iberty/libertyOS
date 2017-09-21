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

extern u32 PrintPos;
extern u32 MainPrintPos;

/* Ring1 终端任务, 用户交互接口 */
void Task_tty()
{
	TTY* p_tty;
	
	/* 初始化所有 TTY */
	for (p_tty = tty_table; p_tty < tty_table + NR_CONSOLES; p_tty++) {
		init_tty(p_tty);
	}
	
	select_console(0); /* 系统启动时使用 0 号控制台 */

	/**
	 * TTY 任务的主循环, 轮询每个 TTY, 如果 p_tty 对应的控制台是当前控制台
	 * 则读取对应的键盘输入缓冲并显示. 注意: 键盘是所有 TTY 共享的, 但每个
	 * TTY 拥有独立的键盘输入缓冲.
	 */
	while (1) {
		for (p_tty = tty_table; p_tty < tty_table + NR_CONSOLES; p_tty++) {
			if (is_current_console(p_tty)) {
				keyboard_read();
			}
		}
	}
}

void init_tty(TTY* p_tty)
{
	/* 初始化键盘输入缓冲 */
	p_tty->kb_in.p_head = p_tty->kb_in.p_tail = p_tty->kb_in.buf_queue;
	p_tty->kb_in.count = 0;
	
	/* 初始化控制台 */
	int nr_tty = p_tty - tty_table;
	p_tty->p_console = console_table + nr_tty;
	
	init_console(p_tty);
}

void init_console(TTY* p_tty)
{
	int nr_tty = p_tty - tty_table;
	
	int con_v_mem_size						= V_MEM_SCREEN; /* V_MEM_SIZE = 2 * SCREEN_SIZE, 一个 console 有两个屏幕的显示空间 */
	p_tty->p_console->orig_addr				= nr_tty * con_v_mem_size;	
	p_tty->p_console->v_mem_limit			= con_v_mem_size;
	p_tty->p_console->current_start_addr	= p_tty->p_console->orig_addr;
	
	if (nr_tty == 0) { /* 0 号控制台沿用原来的光标位置 */
		p_tty->p_console->cursor_pos = MainPrintPos >> 1;
	} else {
		p_tty->p_console->cursor_pos = p_tty->p_console->orig_addr;
		disp_tips(p_tty);
	}
}

/**
 * 判断 p_tty 对应的控制台是否是当前控制台
 */
int is_current_console(TTY* p_tty)
{
	int nr_console;
	nr_console = p_tty->p_console - console_table;
	
	return (nr_console == nr_current_console);
}

/**
 * 回显键盘输入.
 * tty_printchar 是对 printchar 的封装, 使其适用于不同终端
 */
void tty_printchar(TTY* p_tty, char ch)
{
	PrintPos = p_tty->p_console->cursor_pos << 1;
	printchar(ch);
	p_tty->p_console->cursor_pos = PrintPos >> 1;
	set_cursor_pos(p_tty->p_console->cursor_pos);
}

/**
 * 借助 tty_printchar 在终端打印字符串
 */
void tty_printstr(TTY* p_tty, char* str)
{
	int len = _strlen(str);
	for (int i = 0; i < len; i++)
		tty_printchar(p_tty, str[i]);
}

/**
 * 封装 backspace, 使其适用于终端环境, 不至于将终端提示符删掉
 */
void tty_backspace(TTY* p_tty)
{
	u32 next_cursor = p_tty->p_console->cursor_pos - 1; /* 退格后的光标位置, 不一定采用 */
	u32 min_cursor = p_tty->p_console->current_start_addr + 2; /* 终端提示符有 2 个字符 */
	if (next_cursor > min_cursor) {
		backspace();
		p_tty->p_console->cursor_pos--;
		set_cursor_pos(PrintPos >> 1);
	}
}

/**
 * 打印终端提示符
 */
void disp_tips(TTY* p_tty)
{
	int nr_tty = p_tty - tty_table;
	
	tty_printchar(p_tty, nr_tty + '0');
	tty_printchar(p_tty, '$');
	tty_printchar(p_tty, ' ');
}

/**
 * 重置键盘输入缓冲
 */
void reset_kb_buf(TTY* p_tty)
{
	p_tty->kb_in.p_head = p_tty->kb_in.p_tail = p_tty->kb_in.buf_queue;
	p_tty->kb_in.count = 0;
	_memset(p_tty->kb_in.buf_queue, 0, KB_BUFSIZE);	
}

/**
 * 解析键盘输入
 */
void parse_input(TTY* p_tty)
{
	char input[KB_BUFSIZE];
	
	/* 将缓冲区字符逐个拷贝到 `input`, 组装为字符串 */
	for (int i = 0; i < KB_BUFSIZE; i++) {
		char scan_code = p_tty->kb_in.buf_queue[i];
		char _ch = keymap[scan_code & MAKE_MASK];
		if (_ch & TEXT_MASK) {
			char ch = _ch & ~TEXT_MASK;
			if (ch == '\n') /* end */
				input[i] = 0;
			else
				input[i] = ch;
		}
	}
	
	char s[] = "hello";
	if (!_strcmp(s, input, max(_strlen(s), _strlen(input))))
		tty_printstr(p_tty, "hello,world!");
	else
		tty_printstr(p_tty, input);
}


