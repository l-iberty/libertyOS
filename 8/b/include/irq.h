#ifndef IRQ_H
#define IRQ_H

#include "type.h"
#include "sysconst.h"

/* IRQ Number for 8259A */
#define IRQ_CLOCK		0
#define IRQ_KEYBOARD		1
#define IRQ_CASCADE		2
#define IRQ_AT			14

/* I/O ports for 8259A */
#define	INT_M_CTL		0x20
#define	INT_M_CTLMASK		0x21
#define	INT_S_CTL		0xA0
#define	INT_S_CTLMASK		0xA1

extern IRQ_HANDLER irq_table[NR_IRQ];

void	put_irq_handler(int irq, IRQ_HANDLER handler);
void	enable_irq(int irq);
void	disable_irq(int irq);

/* handlers */
void	clock_handler(int irq);
void	keyboard_handler(int irq);
void	hd_handler(int irq);


#endif /* IRQ_H */

