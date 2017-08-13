#include "type.h"
#include "sysconst.h"
#include "protect.h"

/* 函数原型 */
/* 库函数 */
void println(char* sz);				/* lib/klib.asm */
void printmsg(char* sz, u8 color, u32 pos);	/* lib/klib.asm */
void init_8259A();				/* kernel/8259A.asm */
void itoa(char* str, int v, int len, u8 flag);	/* lib/klib.asm */
/* 0 ~ 19 中断处理例程 */
void divide_error();
void reserved();
void nmi();
void breakpoint();
void overflow();
void out_of_bound();
void invalid_opcode();
void no_math_coprocessor();
void double_fault();
void coprocessor_error();
void invalid_tss();
void not_present();
void stack_error();
void general_protect();
void page_fault();
void intel_reserved();
void mcu_fault();
void align_check();
void mechine();
void simd_error();
/* 8259A 中断处理例程 */
/* 主 8259A */
void irq00_handler();
void irq01_handler();
void irq02_handler();
void irq03_handler();
void irq04_handler();
void irq05_handler();
void irq06_handler();
void irq07_handler();
void irq08_handler();
void irq09_handler();
void irq10_handler();
void irq11_handler();
void irq12_handler();
void irq13_handler();
void irq14_handler();

void sys_call();	/* system call */

/* 外部变量 */
extern u8	GDT[GDT_DESC_NUM * DESC_SIZE];	/* kernel/kernel.asm */
extern u8	IDT[IDT_DESC_NUM * DESC_SIZE];	/* kernel/kernel.asm */
extern u8	LDT[LDT_DESC_NUM * DESC_SIZE];	/* kernel/kernel.asm */
extern fpPROC	TaskA;				/* kernel/main.c */
extern u8	TSS[TSS_SIZE];			/* kernel/kernel.asm */

 
#define ERR_MSG_LEN 18 /* 每条 err_msg 的固定长度 */
char* err_msg_list[] = {
		"#DE               ",
		"#DB               ",
		"--NMI             ",
		"#BP               ",
		"#OF               ",
		"#BR               ",
		"#UD               ",
		"#NM               ",
		"#DF               ",
		"--CO-PROCESS ERROR",
		"#TS               ",
		"#NP               ",
		"#SS               ",
		"#GP               ",
		"#PF               ",
		"-INTEL RESERVED   ",
		"#MF               ",
		"#AC               ",
		"#MC               ",
		"#XM               "
		};


void excep_handler(int vecno, u32 err_code, u32 eip, u16 cs, u32 eflags)
{
	/* err_msg 格式: "err_msg CS:xxxx EIP:xxxxxxxx EFLAGS:xxxxxxxx ErrorCode:xxxxxxxx" 共 74 字符*/
	char* err_msg = err_msg_list[vecno];
	char err_info[75 - ERR_MSG_LEN] = "CS:xxxx EIP:xxxxxxxx EFLAGS:xxxxxxxx ErrorCode:xxxxxxxx";
	itoa(err_info + 3, cs, 4, 0);
	itoa(err_info + 12, eip, 8, 0);
	itoa(err_info + 28, eflags, 8, 0);
	itoa(err_info + 47, err_code, 8, 0);

	/* 0x74 => 0111 灰底, 0100 暗红 */
	printmsg(err_msg, 0x74, (80 * 23 + 0) * 2);
	printmsg(err_info, 0x74, (80 * 24 + 0) * 2);
}

void irq_handler(int irqno)
{
	char msg[18] = "__8259A__ IRQ: ";
	itoa(msg + 15, irqno, 2, 1);
	/* 0000 黑底, 1111 白字 */
	printmsg(msg, 0x0F, (80 * 22 + 0) * 2);
}

void init_idt_desc(int vecno, u16 selector, u32 proc_offset, u8 attr)
{
	u8* p_desc = (u8*) &IDT[vecno * DESC_SIZE];
	*(u16*) p_desc = (u16) (proc_offset & 0xFFFF);			/* offset, low 16 bits */
	*(u16*) (p_desc + 2) = selector;				/* selector */
	*(p_desc + 4) = 0;						/* reserved */
	*(p_desc + 5) = attr;						/* attr */
	*(u16*) (p_desc + 6) = (u16) ((proc_offset >> 16) & 0xFFFF);	/* offset, high 16 bits */
}

void cstart()
{
	println("------------cstart------------");
	
	println("initialize 8259A and IDT...");
	init_8259A();

	/* 初始化前 20 个 IDT 描述符 */

	init_idt_desc(INT_VECTOR_DE, SELECTOR_FLATC, (u32) divide_error, DA_IGATE);

	init_idt_desc(INT_VECTOR_DB, SELECTOR_FLATC, (u32) reserved, DA_IGATE);

	init_idt_desc(INT_VECTOR_02H, SELECTOR_FLATC, (u32) nmi, DA_IGATE);

	init_idt_desc(INT_VECTOR_BP, SELECTOR_FLATC, (u32) breakpoint, DA_IGATE);

	init_idt_desc(INT_VECTOR_OF, SELECTOR_FLATC, (u32) overflow, DA_IGATE);

	init_idt_desc(INT_VECTOR_BR, SELECTOR_FLATC, (u32) out_of_bound, DA_IGATE);

	init_idt_desc(INT_VECTOR_UD, SELECTOR_FLATC, (u32) invalid_opcode, DA_IGATE);

	init_idt_desc(INT_VECTOR_NM, SELECTOR_FLATC, (u32) no_math_coprocessor, DA_IGATE);

	init_idt_desc(INT_VECTOR_DF, SELECTOR_FLATC, (u32) double_fault, DA_IGATE);

	init_idt_desc(INT_VECTOR_09H, SELECTOR_FLATC, (u32) coprocessor_error, DA_IGATE);

	init_idt_desc(INT_VECTOR_TS, SELECTOR_FLATC, (u32) invalid_tss, DA_IGATE);

	init_idt_desc(INT_VECTOR_NP, SELECTOR_FLATC, (u32) not_present, DA_IGATE);

	init_idt_desc(INT_VECTOR_SS, SELECTOR_FLATC, (u32) stack_error, DA_IGATE);

	init_idt_desc(INT_VECTOR_GP, SELECTOR_FLATC, (u32) general_protect, DA_IGATE);

	init_idt_desc(INT_VECTOR_PF, SELECTOR_FLATC, (u32) page_fault, DA_IGATE);
	
	init_idt_desc(INT_VECTOR_0EH, SELECTOR_FLATC, (u32) intel_reserved, DA_IGATE);	

	init_idt_desc(INT_VECTOR_MF, SELECTOR_FLATC, (u32) mcu_fault, DA_IGATE);

	init_idt_desc(INT_VECTOR_AC, SELECTOR_FLATC, (u32) align_check, DA_IGATE);

	init_idt_desc(INT_VECTOR_MC, SELECTOR_FLATC, (u32) mechine, DA_IGATE);

	init_idt_desc(INT_VECTOR_XM, SELECTOR_FLATC, (u32) simd_error, DA_IGATE);

	/* 8259A */
	init_idt_desc(INT_VECTOR_IRQ00, SELECTOR_FLATC,
		(u32) irq00_handler, DA_IGATE);

	init_idt_desc(INT_VECTOR_IRQ01, SELECTOR_FLATC,
		(u32) irq01_handler, DA_IGATE);

	init_idt_desc(INT_VECTOR_IRQ02, SELECTOR_FLATC,
		(u32) irq02_handler, DA_IGATE);

	init_idt_desc(INT_VECTOR_IRQ03, SELECTOR_FLATC,
		(u32) irq03_handler, DA_IGATE);

	init_idt_desc(INT_VECTOR_IRQ04, SELECTOR_FLATC,
		(u32) irq04_handler, DA_IGATE);

	init_idt_desc(INT_VECTOR_IRQ05, SELECTOR_FLATC,
		(u32) irq05_handler, DA_IGATE);

	init_idt_desc(INT_VECTOR_IRQ06, SELECTOR_FLATC,
		(u32) irq06_handler, DA_IGATE);

	init_idt_desc(INT_VECTOR_IRQ07, SELECTOR_FLATC,
		(u32) irq07_handler, DA_IGATE);

	init_idt_desc(INT_VECTOR_IRQ08, SELECTOR_FLATC,
		(u32) irq08_handler, DA_IGATE);

	init_idt_desc(INT_VECTOR_IRQ09, SELECTOR_FLATC,
		(u32) irq09_handler, DA_IGATE);

	init_idt_desc(INT_VECTOR_IRQ10, SELECTOR_FLATC,
		(u32) irq10_handler, DA_IGATE);

	init_idt_desc(INT_VECTOR_IRQ11, SELECTOR_FLATC,
		(u32) irq11_handler, DA_IGATE);

	init_idt_desc(INT_VECTOR_IRQ12, SELECTOR_FLATC,
		(u32) irq12_handler, DA_IGATE);

	init_idt_desc(INT_VECTOR_IRQ13, SELECTOR_FLATC,
		(u32) irq13_handler, DA_IGATE);

	init_idt_desc(INT_VECTOR_IRQ14, SELECTOR_FLATC,
		(u32) irq14_handler, DA_IGATE);
		
	/* system call */
	init_idt_desc(INT_VECTOR_SYSCALL, SELECTOR_FLATC,
		(u32) sys_call, DA_IGATE | DPL_3);
		
	println("done!");
	
	/* 初始化 GDT 中的 TSS 描述符 */
	init_desc(&GDT[INDEX_TSS_DESC * DESC_SIZE], (u32) TSS, sizeof(TSS) - 1, DA_TSS);

}

