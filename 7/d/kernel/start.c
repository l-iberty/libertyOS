#include "type.h"
#include "sysconst.h"
#include "protect.h"
#include "global.h"
#include "stdio.h"

void init_8259A(); /* kernel/8259A.asm */
void sys_call(); /* interruption routine for system call */

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



 
char* err_msg_list[] = {
		"#DE",
		"#DB",
		"--NMI",
		"#BP",
		"#OF",
		"#BR",
		"#UD",
		"#NM",
		"#DF",
		"--CO-PROCESS ERROR",
		"#TS",
		"#NP",
		"#SS",
		"#GP",
		"#PF",
		"-INTEL RESERVED",
		"#MF",
		"#AC",
		"#MC",
		"#XM"
		};


void excep_handler(int vecno, u32 err_code, u32 eip, u16 cs, u32 eflags)
{
	char* err = err_msg_list[vecno];
	char msg[160];
	sprintf(msg, "%s {CS:%.4x EIP:%.8x EFLAGS:%.8x ERROR-CODE:%.8x}",
		err, cs, eip, eflags, err_code);
	
	/* 0x74 => 0111 灰底, 0100 暗红 */
	printmsg(msg, 0x74, (80 * 24 + 0) * 2);
}

void irq_handler(int irqno)
{
	char msg[32];
	sprintf(msg, "__8259A__ IRQ: %.2x", irqno);
	/* 0000 黑底, 1111 白字 */
	printmsg(msg, 0x0F, (80 * 23 + 0) * 2);
}

void cstart()
{
	println("\n\n\n\n\n\n\n\n------------cstart------------");
	
	init_8259A();

	/* 初始化前 20 个 IDT 描述符 */

	init_idt_desc(IDT, INT_VECTOR_DE, SELECTOR_FLATC, (u32) divide_error, DA_IGATE);

	init_idt_desc(IDT, INT_VECTOR_DB, SELECTOR_FLATC, (u32) reserved, DA_IGATE);

	init_idt_desc(IDT, INT_VECTOR_02H, SELECTOR_FLATC, (u32) nmi, DA_IGATE);

	init_idt_desc(IDT, INT_VECTOR_BP, SELECTOR_FLATC, (u32) breakpoint, DA_IGATE);

	init_idt_desc(IDT, INT_VECTOR_OF, SELECTOR_FLATC, (u32) overflow, DA_IGATE);

	init_idt_desc(IDT, INT_VECTOR_BR, SELECTOR_FLATC, (u32) out_of_bound, DA_IGATE);

	init_idt_desc(IDT, INT_VECTOR_UD, SELECTOR_FLATC, (u32) invalid_opcode, DA_IGATE);

	init_idt_desc(IDT, INT_VECTOR_NM, SELECTOR_FLATC, (u32) no_math_coprocessor, DA_IGATE);

	init_idt_desc(IDT, INT_VECTOR_DF, SELECTOR_FLATC, (u32) double_fault, DA_IGATE);

	init_idt_desc(IDT, INT_VECTOR_09H, SELECTOR_FLATC, (u32) coprocessor_error, DA_IGATE);

	init_idt_desc(IDT, INT_VECTOR_TS, SELECTOR_FLATC, (u32) invalid_tss, DA_IGATE);

	init_idt_desc(IDT, INT_VECTOR_NP, SELECTOR_FLATC, (u32) not_present, DA_IGATE);

	init_idt_desc(IDT, INT_VECTOR_SS, SELECTOR_FLATC, (u32) stack_error, DA_IGATE);

	init_idt_desc(IDT, INT_VECTOR_GP, SELECTOR_FLATC, (u32) general_protect, DA_IGATE);

	init_idt_desc(IDT, INT_VECTOR_PF, SELECTOR_FLATC, (u32) page_fault, DA_IGATE);
	
	init_idt_desc(IDT, INT_VECTOR_0EH, SELECTOR_FLATC, (u32) intel_reserved, DA_IGATE);	

	init_idt_desc(IDT, INT_VECTOR_MF, SELECTOR_FLATC, (u32) mcu_fault, DA_IGATE);

	init_idt_desc(IDT, INT_VECTOR_AC, SELECTOR_FLATC, (u32) align_check, DA_IGATE);

	init_idt_desc(IDT, INT_VECTOR_MC, SELECTOR_FLATC, (u32) mechine, DA_IGATE);

	init_idt_desc(IDT, INT_VECTOR_XM, SELECTOR_FLATC, (u32) simd_error, DA_IGATE);

	/* 8259A */
	init_idt_desc(IDT, INT_VECTOR_IRQ00, SELECTOR_FLATC,
		(u32) irq00_handler, DA_IGATE);

	init_idt_desc(IDT, INT_VECTOR_IRQ01, SELECTOR_FLATC,
		(u32) irq01_handler, DA_IGATE);

	init_idt_desc(IDT, INT_VECTOR_IRQ02, SELECTOR_FLATC,
		(u32) irq02_handler, DA_IGATE);

	init_idt_desc(IDT, INT_VECTOR_IRQ03, SELECTOR_FLATC,
		(u32) irq03_handler, DA_IGATE);

	init_idt_desc(IDT, INT_VECTOR_IRQ04, SELECTOR_FLATC,
		(u32) irq04_handler, DA_IGATE);

	init_idt_desc(IDT, INT_VECTOR_IRQ05, SELECTOR_FLATC,
		(u32) irq05_handler, DA_IGATE);

	init_idt_desc(IDT, INT_VECTOR_IRQ06, SELECTOR_FLATC,
		(u32) irq06_handler, DA_IGATE);

	init_idt_desc(IDT, INT_VECTOR_IRQ07, SELECTOR_FLATC,
		(u32) irq07_handler, DA_IGATE);

	init_idt_desc(IDT, INT_VECTOR_IRQ08, SELECTOR_FLATC,
		(u32) irq08_handler, DA_IGATE);

	init_idt_desc(IDT, INT_VECTOR_IRQ09, SELECTOR_FLATC,
		(u32) irq09_handler, DA_IGATE);

	init_idt_desc(IDT, INT_VECTOR_IRQ10, SELECTOR_FLATC,
		(u32) irq10_handler, DA_IGATE);

	init_idt_desc(IDT, INT_VECTOR_IRQ11, SELECTOR_FLATC,
		(u32) irq11_handler, DA_IGATE);

	init_idt_desc(IDT, INT_VECTOR_IRQ12, SELECTOR_FLATC,
		(u32) irq12_handler, DA_IGATE);

	init_idt_desc(IDT, INT_VECTOR_IRQ13, SELECTOR_FLATC,
		(u32) irq13_handler, DA_IGATE);

	init_idt_desc(IDT, INT_VECTOR_IRQ14, SELECTOR_FLATC,
		(u32) irq14_handler, DA_IGATE);
		
	/* system call */
	init_idt_desc(IDT, INT_VECTOR_SYSCALL, SELECTOR_FLATC,
		(u32) sys_call, DA_IGATE | DPL_3);
		
	/* 初始化 GDT 中的 TSS 描述符 */
	init_desc(&GDT[INDEX_TSS_DESC * DESC_SIZE], (u32) TSS, sizeof(TSS) - 1, DA_TSS);
}

