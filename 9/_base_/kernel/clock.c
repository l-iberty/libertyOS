#include "proc.h"
#include "stdio.h"
#include "stdlib.h"
#include "global.h"
#include "protect.h"
#include "irq.h"


void disp_time()
{
	uint8_t second, minute, hour, day, month, year;
	
	out_byte(CMOS_ADDR_REG, NR_SECOND);
	second = in_byte(CMOS_DATA_REG);
	
	out_byte(CMOS_ADDR_REG, NR_MINUTE);
	minute = in_byte(CMOS_DATA_REG);
	
	out_byte(CMOS_ADDR_REG, NR_HOUR);
	hour = in_byte(CMOS_DATA_REG);
	
	out_byte(CMOS_ADDR_REG, NR_DAY);
	day = in_byte(CMOS_DATA_REG);
	
	out_byte(CMOS_ADDR_REG, NR_MONTH);
	month = in_byte(CMOS_DATA_REG);
	
	out_byte(CMOS_ADDR_REG, NR_YEAR);
	year = in_byte(CMOS_DATA_REG);
	
	char buf[18];
	sprintf(buf, "%.2x/%.2x/%.2x %.2x:%.2x:%.2x",
			year, month, day, hour, minute, second);
	printmsg(buf, 0x0C, (80*0+62)*2); /* 0000 黑底, 1100 亮红 */
}

/* Ring0 */
void clock_handler(int irq)
{
	ticks++;
	p_current_proc->ticks--;
	
	disp_time();
	
	schedule();
}

void init_clock()
{
	/* Initialize 8253 PIT */
	int v = TIMER_IN_FREQ / CLK_FREQ;
	out_byte(TIMER_MODE_CTL, RATE_GENERATOR);
	out_byte(TIMER_0, (uint8_t) v); /* Low-Byte */
	out_byte(TIMER_0, (uint8_t) (v >> 8)); /* High-Byte */
	
	put_irq_handler(IRQ_CLOCK, clock_handler);
	enable_irq(IRQ_CLOCK);
}
