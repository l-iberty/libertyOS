#include "protect.h"
#include "sysconst.h"
#include "global.h"
#include "stdlib.h"


/**
 * 初始化 GDT/LDT 描述符
 * @param p_desc	描述符指针
 * @param base		32-bit 线性基地址
 * @param limit		段界限, 低 20 bits 可用
 * @param attr		属性, 低 12 bits 可用
 */
void init_desc(u8* p_desc, u32 base, u32 limit, u16 attr)
{
	*(u16*) (p_desc) = (u16) (limit & 0xFFFF);	/* limit 0~15 */
	*(u16*) (p_desc + 2) = (u16) (base & 0xFFFF);	/* base 0~15 */
	*(p_desc + 4) = (u8) ((base >> 16) & 0xFF);	/* base 16~23 */
	*(u16*) (p_desc + 5) = (u16) ((attr & 0xF0FF) | ((limit >> 8) & 0x0F00));
							/* attr2 + limit 16~19 + attr1 */
	*(p_desc + 7) = (u8) ((base >> 24) & 0xFF);	/* base 24~31 */	
}

/**
 * 初始化 IDT 描述符
 * @param idt_base      IDT 基地址
 * @param vecno		中断向量号
 * @param selector	选择子
 * @param proc_offset	中断例程入口偏移
 * @param attr		描述符属性
 */
void init_idt_desc(u8* idt_base, int vecno, u16 selector, u32 proc_offset, u8 attr)
{
	u8* p_desc = &idt_base[vecno * DESC_SIZE];
	*(u16*) p_desc = (u16) (proc_offset & 0xFFFF);			/* offset, low 16 bits */
	*(u16*) (p_desc + 2) = selector;				/* selector */
	*(p_desc + 4) = 0;						/* reserved */
	*(p_desc + 5) = attr;						/* attr */
	*(u16*) (p_desc + 6) = (u16) ((proc_offset >> 16) & 0xFFFF);	/* offset, high 16 bits */
}

void init_prot()
{
	/* 初始化进程的 LDT 选择子, 及该选择子指向的 GDT 描述符 */
	PROCESS* p_proc = &FIRST_PROC;
	for (int i = 0; i < NR_PROCS; i++, p_proc++)
	{
		memset(p_proc, 0, sizeof(PROCESS));
		p_proc->ldt_selector = SELECTOR_LDT_FIRST + (i << 3);
		init_desc(&GDT[(INDEX_LDT_DESC_FIRST + i) * DESC_SIZE], (u32) p_proc->LDT,
			sizeof(p_proc->LDT) - 1, DA_LDT);
	}
}

u32 get_base(u8* p_desc)
{
	u32 base_low = (u32) (*(u16*) (p_desc + 2));
	u32 base_mid = (u32) *(p_desc + 4);
	u32 base_high = (u32) *(p_desc + 7);
	
	u32 base = (base_high << 24) | (base_mid << 16) | (base_low);
	return base;
}

u32 get_limit(u8* p_desc)
{
	u32 limit1 = (u32) *(u16*) p_desc;
	u32 limit2 = (u32) (*(p_desc + 6) & 0x0F);
	u32 limit = (limit2 << 16) | (limit1);
	
	return limit;
}

u32 granularity(u8* p_desc)
{
	u8 attr2 = (*(p_desc + 6) >> 4) & 0x0F;
	u32 granu = ((attr2 >> 3) & 0x01) ? 4096 : 1;
	
	return granu;
}

