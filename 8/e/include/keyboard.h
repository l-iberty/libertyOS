#ifndef KEYBOARD_H
#define KEYBOARD_H

#include "type.h"

#define KB_BUFSIZE	64
#define BREAK_MASK	0x80	/* Break Code 的最高位为 1 */
#define MAKE_MASK	0x7F	/* Make Code 最高位为 0 */

struct kb_input
{
	uint8_t	*p_head;		/* 队列头指针 */
	uint8_t	*p_tail;		/* 队列尾指针 */
	int	count;			/* 缓冲区内待处理元素数 */
	uint8_t	buf_queue[KB_BUFSIZE];	/* 缓冲区(队列), 存储 Scan Code */
};

void keyboard_read();
void init_keyboard();

#endif /* KEYBOARD_H */
