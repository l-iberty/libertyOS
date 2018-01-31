/**
 * type.h 数据结构和类型及其他
 */

#ifndef	TYPE_H
#define	TYPE_H

typedef unsigned char       u8;
typedef unsigned short      u16;
typedef unsigned int        u32;
typedef unsigned long long  u64;
typedef unsigned int        size_t;


typedef void (*fpPROC) ();

typedef void* SYSCALL;

typedef void (*IRQ_HANDLER)(int irq);

typedef char* va_list;

#endif /* TYPE_H */
