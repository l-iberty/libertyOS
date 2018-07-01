/**
 * protect.h 与保护模式相关的函数声明
 */

#ifndef PROTECT_H
#define PROTECT_H

#include "proc.h"

void	init_desc(u8* p_desc, u32 base, u32 limit, u16 attr);
void	init_idt_desc(u8* idt_base, int vecno, u16 selector, u32 proc_offset, u8 attr);
void	init_prot();
u32	get_base(u8* p_desc);
u32	get_limit(u8* p_desc);
u32	granularity(u8* p_desc);

#endif /* PROTECT_H */
