#include "type.h"

void _vsprintf(char* buf, const char* fmt, va_list arg);
void print(char* sz);
void println(char* sz);
void set_cursor_pos(u32 pos);

extern u32 MainPrintPos;

void _printf(const char* fmt, ...)
{
	char buf[256] = { 0 }; /* 清零, 防止对下一次输出造成影响 */
	va_list arg = (va_list) ((char *) &fmt + 4);	/* 4 是参数 fmt 所占堆栈的大小 */
	
	_vsprintf(buf, fmt, arg);
	print(buf); /* 不自动换行 */
	set_cursor_pos(MainPrintPos >> 1);
}

void _sprintf(char* buf, const char* fmt, ...)
{
	va_list arg = (va_list) ((char *) &fmt + 4);	/* 4 是参数 fmt 所占堆栈的大小 */
	
	_vsprintf(buf, fmt, arg);
}
