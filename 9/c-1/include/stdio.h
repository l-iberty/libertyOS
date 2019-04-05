#ifndef STDIO_H
#define STDIO_H

#include "type.h"

#define NULL 0

extern uint32_t main_print_pos;

void	printf(const char* fmt, ...);
void	sprintf(char* buf, const char* fmt, ...);
void	println(const char* sz);
void	print(const char* sz);
void	printmsg(const char* sz, uint8_t color, uint32_t pos);
void	printchar(char ch);
void	backspace();
void	set_cursor_pos(uint32_t pos);
void	set_video_start(uint32_t addr);
void	init_video();

void	itoa(char* str, int v, int len, uint8_t flag);
void	vsprintf(char* buf, const char* fmt, va_list arg);

#define	O_CREAT		1
#define O_RDWR		2

#endif /* STDIO */
