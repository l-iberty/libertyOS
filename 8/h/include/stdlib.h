#ifndef STDLIB_H
#define STDLIB_H

#include "type.h"

extern uint32_t print_pos;
extern uint32_t main_print_pos;

void	memcpy(void* pDst, void* pSrc, int len);
void	memset(void* pDst, uint8_t value, int len);
int	memcmp(void* p1, void* p2, int len);
uint8_t in_byte(uint16_t port);
void	out_byte(uint16_t port, uint8_t byte);
void	port_read(uint16_t port, void* buf, int len);
void	port_write(uint16_t port, void* buf, int len);
void	disable_int();
void	enable_int();
uint32_t getcr2();
void	load_cr3(uint32_t cr3);

#endif /* STDLIB */
