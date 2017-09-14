#define PRINTF

#include "type.h"
#include "string.h"
#include "stdio.h"

#define BUFSIZE 1024

void _vsprintf(char* buf, const char* fmt, va_list arg)
{
	char* p;
	char tmp[BUFSIZE];
	va_list p_arg = arg;
	
	for (p = buf; *fmt; fmt++)
	{
		if (*fmt == '%')
		{
			switch (*++fmt)
			{
				case '.':
				{
					int len = (int) (*++fmt - '0');
					if (*++fmt == 'x')
					{
						int v = *(int*) p_arg;
						itoa(tmp, v, len, 1);
						_strcpy(p, tmp);
						p += _strlen(tmp);
						p_arg += 4; /* 下一个参数 */
					}
					break;
				}
				case 's':
				{
					_strcpy(tmp, (char*) (*(int*) p_arg));
					p_arg += 4;
					_strcpy(p, tmp);
					p += _strlen(tmp);
					
					break;				
				}
			}
		}
		else
		{
			*p++ = *fmt;
		}
	}
}
