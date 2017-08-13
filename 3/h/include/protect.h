/**
 * protect.h 与保护模式相关的函数声明
 */

#ifndef PROTECT_H
#define PROTECT_H

#include "type.h"

void init_desc(u8* p_desc, u32 base, u32 limit, u16 attr);

#endif /* PROTECT_H */
