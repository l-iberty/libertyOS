/**
 * protect.h 与保护模式相关的函数声明
 */

#ifndef PROTECT_H
#define PROTECT_H

#include "type.h"

void init_desc(u8* p_desc, u32 base, u32 limit, u16 attr);
void init_idt_desc(u8* idt_base, int vecno, u16 selector, u32 proc_offset, u8 attr);

#endif /* PROTECT_H */
