#define PRINTF

#include "type.h"
#include "string.h"
#include "stdio.h"

#define BUFSIZE 1024

int pow(int x, int e)
{
	if (e == 0) return 1;
	if (e == 1) return x;
	return x * pow(x, e - 1);
}

/**
 * @breif 将十进制 v 转换为十六进制
 *
 * @param len 输出参数: v 被转换为多少位十六进制数?
 */
int tohex(int v, int *len)
{
	int s[8];
	int i;
	int hex;
	
	i = 0;
	do
	{
		s[i++] = v % 10;
		v = v / 10;
	} while (v);
	
	hex = 0;
	*len = i--;
	for(; i >= 0; i--)
	{
		hex += s[i] * pow(16, i);
	}
	return hex;
}

void vsprintf(char* buf, const char* fmt, va_list arg)
{
	char* p;
	char tmp[BUFSIZE];
	int v, len;
	va_list p_arg = arg;
	
	for (p = buf; *fmt; fmt++) 
	{
		if (*fmt == '%') 
		{
			switch (*++fmt)
			{
				case '.':
				{
					len = (int) (*++fmt - '0');
					if (*++fmt == 'x')
					{
						v = *(int*) p_arg;
						itoa(tmp, v, len, 1);
						strcpy(p, tmp);
						p += strlen(tmp);
						p_arg += 4; /* 下一个参数 */
					}
					break;
				}
				case 'd':
				{
					v = *(int*) p_arg;
					v = tohex(v, &len);
					itoa(tmp, v, len, 1);
					strcpy(p, tmp);
					p += strlen(tmp);
					p_arg += 4;
					
					break;
				}
				case 's':
				{
					strcpy(tmp, (char*) (*(int*) p_arg));
					p_arg += 4;
					strcpy(p, tmp);
					p += strlen(tmp);
					
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
