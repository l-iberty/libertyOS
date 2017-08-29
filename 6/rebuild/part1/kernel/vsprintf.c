#include "type.h"

u32 _strlen(char* s);
void _strcpy(char* dst, const char* src);
void itoa(char* str, int v, int len, u8 flag);

void _vsprintf(char* buf, const char* fmt, va_list arg)
{
	char* p;
	char tmp[256];
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
