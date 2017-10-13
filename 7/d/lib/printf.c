#define PRINTF

#include "type.h"
#include "stdio.h"
#include "stdlib.h"

#define BUFSIZE 512

extern u32 MainPrintPos;

void printf(const char* fmt, ...)
{
	char buf[BUFSIZE] = { 0 }; /* 清零, 防止对下一次输出造成影响 */
	va_list arg = (va_list) ((char *) &fmt + 4);	/* 4 是参数 fmt 所占堆栈的大小 */
	
	vsprintf(buf, fmt, arg);
	print(buf); /* 不自动换行 */
	set_cursor_pos(MainPrintPos >> 1);
}

void sprintf(char* buf, const char* fmt, ...)
{
	va_list arg = (va_list) ((char *) &fmt + 4);	/* 4 是参数 fmt 所占堆栈的大小 */
	
	vsprintf(buf, fmt, arg);
}
