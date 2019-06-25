#ifndef EXCEPTION_H
#define EXCEPTION_H

#include "type.h"

/* 0 ~ 19 号中断(异常), Intel 保留 */
#define NR_EXCEPTION	20

#define PG_ERR_CODE_P(x)	(x & 1)
#define PG_ERR_CODE_WR(x)	((x >> 1) & 1)
#define PG_ERR_CODE_US(x)	((x >> 2) & 1)

extern EXCEP_HANDLER exception_table[NR_EXCEPTION];

void put_excep_handler(int vecno, EXCEP_HANDLER handler);

/* #UD */
void do_invalid_opcode(int vecno, uint32_t err_code, uint32_t eip, uint16_t cs, uint32_t eflags);

/* #GP */
void do_general_protection(int vecno, uint32_t err_code, uint32_t eip, uint16_t cs, uint32_t eflags);

/* #PF */
void do_page_fault(int vecno, uint32_t err_code, uint32_t eip, uint16_t cs, uint32_t eflags);

#endif //EXCEPTION_H

