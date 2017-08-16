#include "type.h"

void _vsprintf(char* buf, const char* fmt, va_list arg);
void println(char* sz);

void _printf(const char* fmt, ...)
{
	char buf[256] = { 0 }; /* 清零, 防止对下一次输出造成影响 */
	va_list arg = (va_list) ((char *) &fmt + 4);	/* 4 是参数 fmt 所占堆栈的大小 */
	
	_vsprintf(buf, fmt, arg);
	println(buf); /* 自动换行 */
}
