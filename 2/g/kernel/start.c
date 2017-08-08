#include "type.h"
#include "sysconst.h"

/* 函数原型 */
void println(char* sz);				/* lib/klib.asm */
void printmsg(char* sz, u8 color, u32 pos);	/* lib/klib.asm */
void init_8259A();				/* kernel/8259A.asm */

/* 外部变量 */
extern u8	IDT[INT_NUM * DESC_SIZE];	/* kernel/kernel.asm */


PRIVATE void exp_handler_DE()
{
	/* color = 0x74 => 0111 灰底, 0100 暗红 */
	printmsg("#DE", 0x74, (80 * 23 + 0) * 2);
	__asm__ __volatile__("hlt");
}

PRIVATE void exp_handler_DB()
{
	printmsg("#DB", 0x74, (80 * 23 + 0) * 2);
	__asm__ __volatile__("hlt");
}

PRIVATE void exp_handler_02H()
{
	printmsg("-- VECNO=02H", 0x74, (80 * 23 + 0) * 2);
	__asm__ __volatile__("hlt");
}

PRIVATE void exp_handler_BP()
{
	printmsg("#BP", 0x74, (80 * 23 + 0) * 2);
	__asm__ __volatile__("hlt");
}

PRIVATE void exp_handler_OF()
{
	printmsg("#OF", 0x74, (80 * 23 + 0) * 2);
	__asm__ __volatile__("hlt");
}

PRIVATE void exp_handler_BR()
{
	printmsg("#BR", 0x74, (80 * 23 + 0) * 2);
	__asm__ __volatile__("hlt");
}

PRIVATE void exp_handler_UD()
{
	printmsg("#UD", 0x74, (80 * 23 + 0) * 2);
	__asm__ __volatile__("hlt");
}

PRIVATE void exp_handler_NM()
{
	printmsg("#NM", 0x74, (80 * 23 + 0) * 2);
	__asm__ __volatile__("hlt");
}

PRIVATE void exp_handler_DF()
{
	printmsg("#DF", 0x74, (80 * 23 + 0) * 2);
	__asm__ __volatile__("hlt");
}

PRIVATE void exp_handler_09H()
{
	printmsg("-- VECNO=09H", 0x74, (80 * 23 + 0) * 2);
	__asm__ __volatile__("hlt");
}

PRIVATE void exp_handler_TS()
{
	printmsg("#TS", 0x74, (80 * 23 + 0) * 2);
	__asm__ __volatile__("hlt");
}

PRIVATE void exp_handler_NP()
{
	printmsg("#NP", 0x74, (80 * 23 + 0) * 2);
	__asm__ __volatile__("hlt");
}

PRIVATE void exp_handler_SS()
{
	printmsg("#SS", 0x74, (80 * 23 + 0) * 2);
	__asm__ __volatile__("hlt");
}

PRIVATE void exp_handler_GP()
{
	printmsg("#GP", 0x74, (80 * 23 + 0) * 2);
	__asm__ __volatile__("hlt");
}

PRIVATE void exp_handler_PF()
{
	printmsg("#PF", 0x74, (80 * 23 + 0) * 2);
	__asm__ __volatile__("hlt");
}

PRIVATE void exp_handler_0EH()
{
	printmsg("-- VECNO=0EH", 0x74, (80 * 23 + 0) * 2);
	__asm__ __volatile__("hlt");
}

PRIVATE void exp_handler_MF()
{
	printmsg("#MF", 0x74, (80 * 23 + 0) * 2);
	__asm__ __volatile__("hlt");
}

PRIVATE void exp_handler_AC()
{
	printmsg("#AC", 0x74, (80 * 23 + 0) * 2);
	__asm__ __volatile__("hlt");
}

PRIVATE void exp_handler_MC()
{
	printmsg("#MC", 0x74, (80 * 23 + 0) * 2);
	__asm__ __volatile__("hlt");
}

PRIVATE void exp_handler_XM()
{
	printmsg("#XM", 0x74, (80 * 23 + 0) * 2);
	__asm__ __volatile__("hlt");
}

PRIVATE void init_idt_desc(int vecno, u16 selector, u32 proc_offset, u8 attr)
{
	u8* p_desc = (u8*) &IDT[vecno * DESC_SIZE];
	*(u16*) p_desc = (u16) (proc_offset & 0xFFFF);			/* offset, low 16 bits */
	*(u16*) (p_desc + 2) = selector;				/* selector */
	*(p_desc + 4) = 0;						/* reserved */
	*(p_desc + 5) = attr;						/* attr */
	*(u16*) (p_desc + 6) = (u16) ((proc_offset >> 16) & 0xFFFF);	/* offset, high 16 bits */
}

PRIVATE void init_idt_descs()
{
	/* 初始化前 20 个 IDT 描述符 */

	init_idt_desc(INT_VECTOR_DE, SELECTOR_FLATC, (u32) exp_handler_DE, DA_IGATE);

	init_idt_desc(INT_VECTOR_DB, SELECTOR_FLATC, (u32) exp_handler_DB, DA_IGATE);

	init_idt_desc(INT_VECTOR_02H, SELECTOR_FLATC, (u32) exp_handler_02H, DA_IGATE);

	init_idt_desc(INT_VECTOR_BP, SELECTOR_FLATC, (u32) exp_handler_BP, DA_IGATE);

	init_idt_desc(INT_VECTOR_OF, SELECTOR_FLATC, (u32) exp_handler_OF, DA_IGATE);

	init_idt_desc(INT_VECTOR_BR, SELECTOR_FLATC, (u32) exp_handler_BR, DA_IGATE);

	init_idt_desc(INT_VECTOR_UD, SELECTOR_FLATC, (u32) exp_handler_UD, DA_IGATE);

	init_idt_desc(INT_VECTOR_NM, SELECTOR_FLATC, (u32) exp_handler_NM, DA_IGATE);

	init_idt_desc(INT_VECTOR_DF, SELECTOR_FLATC, (u32) exp_handler_DF, DA_IGATE);

	init_idt_desc(INT_VECTOR_09H, SELECTOR_FLATC, (u32) exp_handler_09H, DA_IGATE);

	init_idt_desc(INT_VECTOR_TS, SELECTOR_FLATC, (u32) exp_handler_TS, DA_IGATE);

	init_idt_desc(INT_VECTOR_NP, SELECTOR_FLATC, (u32) exp_handler_NP, DA_IGATE);

	init_idt_desc(INT_VECTOR_SS, SELECTOR_FLATC, (u32) exp_handler_SS, DA_IGATE);

	init_idt_desc(INT_VECTOR_GP, SELECTOR_FLATC, (u32) exp_handler_GP, DA_IGATE);

	init_idt_desc(INT_VECTOR_PF, SELECTOR_FLATC, (u32) exp_handler_PF, DA_IGATE);
	
	init_idt_desc(INT_VECTOR_0EH, SELECTOR_FLATC, (u32) exp_handler_0EH, DA_IGATE);	

	init_idt_desc(INT_VECTOR_MF, SELECTOR_FLATC, (u32) exp_handler_MF, DA_IGATE);

	init_idt_desc(INT_VECTOR_AC, SELECTOR_FLATC, (u32) exp_handler_AC, DA_IGATE);

	init_idt_desc(INT_VECTOR_MC, SELECTOR_FLATC, (u32) exp_handler_MC, DA_IGATE);

	init_idt_desc(INT_VECTOR_XM, SELECTOR_FLATC, (u32) exp_handler_XM, DA_IGATE);
}

PUBLIC void cstart()
{
	println("------------cstart------------");
	
	println("initialize 8259A and IDT...");
	init_8259A();

	init_idt_descs();

	println("done!");
}

