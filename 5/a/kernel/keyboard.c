#include "type.h"
#include "keyboard.h"
#include "keymap.h"

void keyboard_handler()
{
	u8 scan_code = in_byte(0x60);	
	
	if (scan_code & BREAK_MASK) /* 仅接收 Break Code */
	{
		if (kb_in.count < KB_BUFSIZE)
		{
			*kb_in.p_tail++ = scan_code;
			kb_in.count++;
			if (kb_in.p_tail >= kb_in.buf_queue + KB_BUFSIZE)
			{
				/* 队列已满, 回绕 */
				kb_in.p_tail = kb_in.buf_queue;
			}
		}
	}
}

void init_keyboard()
{
	/* 初始化 kb_in */
	kb_in.p_head = kb_in.p_tail = kb_in.buf_queue;
	kb_in.count = 0;
}

void keyboard_read()
{
	u8 scan_code;
	//char msg[] = "<0x**>";
	
	if (kb_in.count > 0)
	{
		scan_code = *kb_in.p_head++;
		kb_in.count--;
		if (kb_in.p_head == kb_in.buf_queue + KB_BUFSIZE)
		{
			kb_in.p_head = kb_in.buf_queue;
		}
		//itoa(msg + 3, scan_code, 2, 0);
		//println(msg);
		printchar(keymap[scan_code & MAKE_MASK]);		
	}	
}


