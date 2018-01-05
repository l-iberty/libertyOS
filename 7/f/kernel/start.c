#include "type.h"
#include "sysconst.h"
#include "protect.h"
#include "global.h"
#include "stdio.h"
#include "irq.h"

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

void hwint_0();
void hwint_1();
void hwint_2();
void hwint_3();
void hwint_4();
void hwint_5();
void hwint_6();
void hwint_7();
void hwint_8();
void hwint_9();
void hwint_10();
void hwint_11();
void hwint_12();
void hwint_13();
void hwint_14();
void hwint_15();


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

void default_irq_handler(int irq)
{
	char msg[32];
	sprintf(msg, "__8259A__ IRQ: %.2x", irq);
	/* 0000 黑底, 1111 白字 */
	printmsg(msg, 0x0F, (80 * 23 + 0) * 2);
}

void cstart()
{
	println("\n\n\n\n\n\n\n\n------------cstart------------");
	
	init_8259A();
	
	for (int i = 0; i < NR_IRQ; i++) {
		irq_table[i] = default_irq_handler;
	}

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
		(u32) hwint_0, DA_IGATE);

	init_idt_desc(IDT, INT_VECTOR_IRQ01, SELECTOR_FLATC,
		(u32) hwint_1, DA_IGATE);

	init_idt_desc(IDT, INT_VECTOR_IRQ02, SELECTOR_FLATC,
		(u32) hwint_2, DA_IGATE);

	init_idt_desc(IDT, INT_VECTOR_IRQ03, SELECTOR_FLATC,
		(u32) hwint_3, DA_IGATE);

	init_idt_desc(IDT, INT_VECTOR_IRQ04, SELECTOR_FLATC,
		(u32) hwint_4, DA_IGATE);

	init_idt_desc(IDT, INT_VECTOR_IRQ05, SELECTOR_FLATC,
		(u32) hwint_5, DA_IGATE);

	init_idt_desc(IDT, INT_VECTOR_IRQ06, SELECTOR_FLATC,
		(u32) hwint_6, DA_IGATE);

	init_idt_desc(IDT, INT_VECTOR_IRQ07, SELECTOR_FLATC,
		(u32) hwint_7, DA_IGATE);

	init_idt_desc(IDT, INT_VECTOR_IRQ08, SELECTOR_FLATC,
		(u32) hwint_8, DA_IGATE);

	init_idt_desc(IDT, INT_VECTOR_IRQ09, SELECTOR_FLATC,
		(u32) hwint_9, DA_IGATE);

	init_idt_desc(IDT, INT_VECTOR_IRQ10, SELECTOR_FLATC,
		(u32) hwint_10, DA_IGATE);

	init_idt_desc(IDT, INT_VECTOR_IRQ11, SELECTOR_FLATC,
		(u32) hwint_11, DA_IGATE);

	init_idt_desc(IDT, INT_VECTOR_IRQ12, SELECTOR_FLATC,
		(u32) hwint_12, DA_IGATE);

	init_idt_desc(IDT, INT_VECTOR_IRQ13, SELECTOR_FLATC,
		(u32) hwint_13, DA_IGATE);

	init_idt_desc(IDT, INT_VECTOR_IRQ14, SELECTOR_FLATC,
		(u32) hwint_14, DA_IGATE);
	
	init_idt_desc(IDT, INT_VECTOR_IRQ15, SELECTOR_FLATC,
		(u32) hwint_15, DA_IGATE);
		
	/* system call */
	init_idt_desc(IDT, INT_VECTOR_SYSCALL, SELECTOR_FLATC,
		(u32) sys_call, DA_IGATE | DPL_3);
		
	/* 初始化 GDT 中的 TSS 描述符 */
	init_desc(&GDT[INDEX_TSS_DESC * DESC_SIZE], (u32) TSS, sizeof(TSS) - 1, DA_TSS);
}

