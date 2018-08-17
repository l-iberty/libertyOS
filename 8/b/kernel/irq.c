#include "irq.h"
#include "stdlib.h"

IRQ_HANDLER irq_table[NR_IRQ];

void put_irq_handler(int irq, IRQ_HANDLER handler)
{
	disable_irq(irq);
	irq_table[irq] = handler;
}

void enable_irq(int irq)
{
	if (irq < 8) /* master */
	{
		out_byte(INT_M_CTLMASK, in_byte(INT_M_CTLMASK) & ~(1 << irq));
	} 
	else /* slave */
	{ 
		out_byte(INT_S_CTLMASK, in_byte(INT_S_CTLMASK) & ~(1 << (irq % 8)));
	}
}

void disable_irq(int irq)
{
	if (irq < 8) /* master */
	{
		out_byte(INT_M_CTLMASK, in_byte(INT_M_CTLMASK) | (1 << irq));
	}
	else /* slave */
	{
		out_byte(INT_S_CTLMASK, in_byte(INT_S_CTLMASK) | (1 << (irq % 8)));
	}
}

