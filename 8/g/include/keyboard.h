#ifndef KEYBOARD_H
#define KEYBOARD_H

#include "type.h"

#define KB_BUFSIZE	64
#define BREAK_MASK	0x80	/* Break Code 的最高位为 1 */
#define MAKE_MASK	0x7F	/* Make Code 最高位为 0 */

struct tty;

struct kb_input
{
	uint8_t	*p_head;		/* 队列 buf_queue 头指针 */
	uint8_t	*p_rear;		/* 队列 buf_queue 尾指针 */
	int	rear;			/* 指示 buf_text 当前写入位置 */
	int	count;			/* 缓冲区 buf_queue 字符数 */
	uint8_t	buf_queue[KB_BUFSIZE];	/* 字符缓冲队列 */
	uint8_t buf_text[KB_BUFSIZE];	/* 可打印字符缓冲 */
};

uint8_t	keyboard_read(struct tty* p_tty);
void	init_keyboard();

#endif /* KEYBOARD_H */
