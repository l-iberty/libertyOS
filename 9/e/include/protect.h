#ifndef PROTECT_H
#define PROTECT_H

#include "proc.h"

void init_desc(uint8_t* p_desc, uint32_t base, uint32_t limit, uint16_t attr);
void init_idt_desc(uint8_t* idt_base, int vecno, uint16_t selector, uint32_t proc_offset, uint8_t attr);
void init_prot();
uint32_t get_base(uint8_t* p_desc);
uint32_t get_limit(uint8_t* p_desc);
uint32_t granularity(uint8_t* p_desc);

#endif /* PROTECT_H */
