#ifndef STDLIB_H
#define STDLIB_H

#include "type.h"

void	_memcpy(void* pDst, void* pSrc, int len);
void	_memset(void* pDst, u8 value, int len);
int	_memcmp(void* p1, void* p2, int len);
u8 	in_byte(u16 port);
void	out_byte(u16 port, u8 byte);
void	port_read(u16 port, void* buf, int len);
void	port_write(u16 port, void* buf, int len);
void	disable_int();
void	enable_int();

#endif /* STDLIB */
