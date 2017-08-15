/**
 * keyboard.h 键盘相关
 */

#include "type.h"

#define KB_BUFSIZE 16
#define BREAK_MASK 0x80	/* Break Code 的最高位为 1 */

void print(char* sz);
u8 in_byte(u16 port);
void itoa(char* str, int v, int len, u8 flag);

void init_keyboard();
void keyboard_read();

typedef struct {
	u8	*p_head;		/* 队列头指针 */
	u8	*p_tail;		/* 队列尾指针 */
	int	count;			/* 缓冲区内待处理元素数 */
	u8	buf_queue[KB_BUFSIZE];	/* 缓冲区(队列), 存储 Scan Code */
} KB_INPUT;

KB_INPUT kb_in;

