#ifndef STDIO_H
#define STDLIO_H

#include "type.h"

#define NULL 0

extern u32 MainPrintPos;

void	printf(const char* fmt, ...);
void	sprintf(char* buf, const char* fmt, ...);
void	println(const char* sz);
void	print(const char* sz);
void	printmsg(const char* sz, u8 color, u32 pos);
void	printchar(char ch);
void	backspace();
void	set_cursor_pos(u32 pos);
void	set_video_start(u32 addr);
void	init_video();

void	itoa(char* str, int v, int len, u8 flag);
void	vsprintf(char* buf, const char* fmt, va_list arg);

#define	O_CREAT		1
#define O_RDWR		2

#endif /* STDIO */
