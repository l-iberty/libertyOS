#ifndef TYPE_H
#define TYPE_H

typedef unsigned char uint8_t;
typedef unsigned short uint16_t;
typedef unsigned int uint32_t;
typedef unsigned long size_t;

typedef void (*TASK_ENTRY)();

typedef void (*IRQ_HANDLER)(int irq);

typedef void (*EXCEP_HANDLER)(int vecno, uint32_t err_code, uint32_t eip, uint16_t cs, uint32_t eflags);

typedef void* SYSCALL;

typedef char* va_list;

#endif /* TYPE_H */
