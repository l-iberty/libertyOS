#ifndef STRING_H
#define STRING_H

#include "type.h"

size_t	strlen(const char* s);
void	strcpy(char* dst, const char* src);
int	strncmp(const char* s1, const char* s2, int len);

#endif /* STRING_H */
