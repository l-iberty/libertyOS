#include "string.h"

int strlen(char* str)
{
	int len = 0;
	while (*str++ != '\0') len++;
	return len;
}

int strcmp(char* str1, char* str2)
{
	while (*str1 != '\0' && *str1 == *str2) { str1++, str2++; }
	
	if (*str1 == '\0' && *str2 == '\0') { return 0; }
	else if (*str1 > *str2) { return 1; }
	else { return -1; }
}

void strcpy(char* str1, char* str2)
{
	while ((*str1++ = *str2++) != '\0');
}

void memcpy(void* dst, void* src, int size)
{
	int chunks = size / sizeof(long);
	int slice = size % sizeof(long);

	long *d1 = (long*) dst;
	long *s1 = (long*) src;
	while (chunks-- > 0) { *d1++ = *s1++; }

	char *d2 = (char*) d1;
	char *s2 = (char*) s1;
	while (slice-- > 0) { *d2++ = *s2++; }
}

void memset(void* p, char val, int size)
{
	while (size-- > 0) { *(char*)p++ = val; }
}

int memcmp(void* p1, void* p2, int size)
{
	char *pb1 = (char*) p1;
	char *pb2 = (char*) p2;
	
	for (int i = 0; i < size; i++)
	{
		if (pb1[i] > pb2[i]) return 1;
		else if (pb1[i] < pb2[i]) return -1;
	}
	return 0;
}
