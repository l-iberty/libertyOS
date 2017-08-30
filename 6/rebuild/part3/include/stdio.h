#ifndef STDIO_H
#define STDLIO_H

#include "type.h"

#define NULL 0

void	_printf(const char* fmt, ...);
void	_sprintf(char* buf, const char* fmt, ...);
void	println(char* sz);
void	print(char* sz);
void	printmsg(char* sz, u8 color, u32 pos);
void	printchar(char ch);
void	backspace();
void	set_cursor_pos(u32 pos);
void	init_video();

void	itoa(char* str, int v, int len, u8 flag);
void	_vsprintf(char* buf, const char* fmt, va_list arg);

#endif /* STDIO */
