#ifndef STRING_H
#define STRING_H

#include "type.h"

int strlen(char *str);
void strcpy(char *str1, char *str2);
int strcmp(char *str1, char *str2);
void memcpy(void *dst, void *src, int size);
void memset(void *p, char val, int size);
int memcmp(void *p1, void *p2, int size);

#endif /* STRING_H */
