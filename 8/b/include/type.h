#ifndef	TYPE_H
#define	TYPE_H

typedef unsigned char       uint8_t;
typedef unsigned short      uint16_t;
typedef unsigned int        uint32_t;
typedef unsigned int        size_t;


typedef void (*TASK_ENTRY) ();

typedef void (*IRQ_HANDLER)(int irq);

typedef void* SYSCALL;

typedef char* va_list;

#endif /* TYPE_H */
