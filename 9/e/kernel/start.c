#include "type.h"
#include "sysconst.h"
#include "protect.h"
#include "global.h"
#include "stdio.h"
#include "irq.h"
#include "exception.h"

void init_8259A(); /* kernel/8259A.asm */
void sys_call(); /* interruption routine for system call */

/* 0 ~ 19 中断(异常)处理例程 */
void divide_error();
void debug_exception();
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

void cstart()
{
    println("\n\n\n\n\n\n------------cstart------------");

    init_8259A();

    /* 初始化前 20 个 IDT 描述符 */

    init_idt_desc(IDT, INT_VECTOR_DE, SELECTOR_FLATC, (uint32_t)divide_error, DA_386IGATE);

    init_idt_desc(IDT, INT_VECTOR_DB, SELECTOR_FLATC, (uint32_t)debug_exception, DA_386IGATE);

    init_idt_desc(IDT, INT_VECTOR_02H, SELECTOR_FLATC, (uint32_t)nmi, DA_386IGATE);

    init_idt_desc(IDT, INT_VECTOR_BP, SELECTOR_FLATC, (uint32_t)breakpoint, DA_386IGATE);

    init_idt_desc(IDT, INT_VECTOR_OF, SELECTOR_FLATC, (uint32_t)overflow, DA_386IGATE);

    init_idt_desc(IDT, INT_VECTOR_BR, SELECTOR_FLATC, (uint32_t)out_of_bound, DA_386IGATE);

    init_idt_desc(IDT, INT_VECTOR_UD, SELECTOR_FLATC, (uint32_t)invalid_opcode, DA_386IGATE);

    init_idt_desc(IDT, INT_VECTOR_NM, SELECTOR_FLATC, (uint32_t)no_math_coprocessor, DA_386IGATE);

    init_idt_desc(IDT, INT_VECTOR_DF, SELECTOR_FLATC, (uint32_t)double_fault, DA_386IGATE);

    init_idt_desc(IDT, INT_VECTOR_09H, SELECTOR_FLATC, (uint32_t)coprocessor_error, DA_386IGATE);

    init_idt_desc(IDT, INT_VECTOR_TS, SELECTOR_FLATC, (uint32_t)invalid_tss, DA_386IGATE);

    init_idt_desc(IDT, INT_VECTOR_NP, SELECTOR_FLATC, (uint32_t)not_present, DA_386IGATE);

    init_idt_desc(IDT, INT_VECTOR_SS, SELECTOR_FLATC, (uint32_t)stack_error, DA_386IGATE);

    init_idt_desc(IDT, INT_VECTOR_GP, SELECTOR_FLATC, (uint32_t)general_protect, DA_386IGATE);

    init_idt_desc(IDT, INT_VECTOR_PF, SELECTOR_FLATC, (uint32_t)page_fault, DA_386IGATE);

    init_idt_desc(IDT, INT_VECTOR_MF, SELECTOR_FLATC, (uint32_t)mcu_fault, DA_386IGATE);

    init_idt_desc(IDT, INT_VECTOR_AC, SELECTOR_FLATC, (uint32_t)align_check, DA_386IGATE);

    init_idt_desc(IDT, INT_VECTOR_MC, SELECTOR_FLATC, (uint32_t)mechine, DA_386IGATE);

    init_idt_desc(IDT, INT_VECTOR_XM, SELECTOR_FLATC, (uint32_t)simd_error, DA_386IGATE);

    put_excep_handler(INT_VECTOR_UD, do_invalid_opcode);

    put_excep_handler(INT_VECTOR_GP, do_general_protection);

    put_excep_handler(INT_VECTOR_PF, do_page_fault);

    /* 8259A */
    init_idt_desc(IDT, INT_VECTOR_IRQ00, SELECTOR_FLATC,
                  (uint32_t)hwint_0, DA_386IGATE);

    init_idt_desc(IDT, INT_VECTOR_IRQ01, SELECTOR_FLATC,
                  (uint32_t)hwint_1, DA_386IGATE);

    init_idt_desc(IDT, INT_VECTOR_IRQ02, SELECTOR_FLATC,
                  (uint32_t)hwint_2, DA_386IGATE);

    init_idt_desc(IDT, INT_VECTOR_IRQ03, SELECTOR_FLATC,
                  (uint32_t)hwint_3, DA_386IGATE);

    init_idt_desc(IDT, INT_VECTOR_IRQ04, SELECTOR_FLATC,
                  (uint32_t)hwint_4, DA_386IGATE);

    init_idt_desc(IDT, INT_VECTOR_IRQ05, SELECTOR_FLATC,
                  (uint32_t)hwint_5, DA_386IGATE);

    init_idt_desc(IDT, INT_VECTOR_IRQ06, SELECTOR_FLATC,
                  (uint32_t)hwint_6, DA_386IGATE);

    init_idt_desc(IDT, INT_VECTOR_IRQ07, SELECTOR_FLATC,
                  (uint32_t)hwint_7, DA_386IGATE);

    init_idt_desc(IDT, INT_VECTOR_IRQ08, SELECTOR_FLATC,
                  (uint32_t)hwint_8, DA_386IGATE);

    init_idt_desc(IDT, INT_VECTOR_IRQ09, SELECTOR_FLATC,
                  (uint32_t)hwint_9, DA_386IGATE);

    init_idt_desc(IDT, INT_VECTOR_IRQ10, SELECTOR_FLATC,
                  (uint32_t)hwint_10, DA_386IGATE);

    init_idt_desc(IDT, INT_VECTOR_IRQ11, SELECTOR_FLATC,
                  (uint32_t)hwint_11, DA_386IGATE);

    init_idt_desc(IDT, INT_VECTOR_IRQ12, SELECTOR_FLATC,
                  (uint32_t)hwint_12, DA_386IGATE);

    init_idt_desc(IDT, INT_VECTOR_IRQ13, SELECTOR_FLATC,
                  (uint32_t)hwint_13, DA_386IGATE);

    init_idt_desc(IDT, INT_VECTOR_IRQ14, SELECTOR_FLATC,
                  (uint32_t)hwint_14, DA_386IGATE);

    init_idt_desc(IDT, INT_VECTOR_IRQ15, SELECTOR_FLATC,
                  (uint32_t)hwint_15, DA_386IGATE);

    /* system call */
    init_idt_desc(IDT, INT_VECTOR_SYSCALL, SELECTOR_FLATC,
                  (uint32_t)sys_call, DA_386IGATE | DPL_3);

    /* 初始化 GDT 中的 TSS 描述符 */
    init_desc(&GDT[INDEX_TSS_DESC * DESC_SIZE], (uint32_t)TSS, sizeof(TSS) - 1, DA_386TSS);
}
