#define PRINTF

#include "type.h"
#include "stdio.h"
#include "stdlib.h"
#include "global.h"

#define PRINTF_BUFSIZE 512

void printf(const char* fmt, ...)
{
    char buf[PRINTF_BUFSIZE] = { 0 }; /* 清零, 防止对下一次输出造成影响 */
    va_list arg = (va_list)((char*)&fmt + 4); /* 4 是参数 fmt 所占堆栈的大小 */

    vsprintf(buf, fmt, arg);
    printk(buf);
}

void sprintf(char* buf, const char* fmt, ...)
{
    va_list arg = (va_list)((char*)&fmt + 4);
    vsprintf(buf, fmt, arg);
}
