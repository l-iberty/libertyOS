#include "protect.h"
#include "global.h"
#include "stdlib.h"
#include "string.h"
#include "sysconst.h"

/**
 * 初始化 GDT/LDT	描述符
 * @param p_desc	描述符指针
 * @param base		32-bit 线性基地址
 * @param limit		段界限, 低 20 bits 可用
 * @param attr		属性, 低 12 bits 可用
 */
void init_desc(uint8_t *p_desc, uint32_t base, uint32_t limit, uint16_t attr) {
  *(uint16_t *)(p_desc) = (uint16_t)(limit & 0xFFFF);    /* limit 0~15 */
  *(uint16_t *)(p_desc + 2) = (uint16_t)(base & 0xFFFF); /* base 0~15 */
  *(p_desc + 4) = (uint8_t)((base >> 16) & 0xFF);        /* base 16~23 */
  *(uint16_t *)(p_desc + 5) = (uint16_t)((attr & 0xF0FF) | ((limit >> 8) & 0x0F00));
  /* attr2 + limit 16~19 + attr1 */
  *(p_desc + 7) = (uint8_t)((base >> 24) & 0xFF); /* base 24~31 */
}

/**
 * 初始化 IDT 描述符
 * @param idt_base      IDT 基地址
 * @param vecno         中断向量号
 * @param selector		选择子
 * @param proc_offset   中断例程入口偏移
 * @param attr			描述符属性
 */
void init_idt_desc(uint8_t *idt_base, int vecno, uint16_t selector, uint32_t proc_offset, uint8_t attr) {
  uint8_t *p_desc = &idt_base[vecno * DESC_SIZE];
  *(uint16_t *)p_desc = (uint16_t)(proc_offset & 0xFFFF);               /* offset, low 16 bits */
  *(uint16_t *)(p_desc + 2) = selector;                                 /* selector */
  *(p_desc + 4) = 0;                                                    /* reserved */
  *(p_desc + 5) = attr;                                                 /* attr */
  *(uint16_t *)(p_desc + 6) = (uint16_t)((proc_offset >> 16) & 0xFFFF); /* offset, high 16 bits */
}

void init_prot() {
  /* 初始化进程的 LDT 选择子, 及该选择子指向的 GDT 描述符 */
  struct proc *p_proc = &FIRST_PROC;
  for (int i = 0; i < NR_PROCS; i++, p_proc++) {
    memset(p_proc, 0, sizeof(struct proc));
    p_proc->ldt_selector = SELECTOR_LDT_FIRST + (i << 3);
    init_desc(&GDT[(INDEX_LDT_DESC_FIRST + i) * DESC_SIZE], (uint32_t)p_proc->LDT, sizeof(p_proc->LDT) - 1, DA_LDT);
  }
}

uint32_t get_base(uint8_t *p_desc) {
  uint32_t base_low = (uint32_t)(*(uint16_t *)(p_desc + 2));
  uint32_t base_mid = (uint32_t) * (p_desc + 4);
  uint32_t base_high = (uint32_t) * (p_desc + 7);

  uint32_t base = (base_high << 24) | (base_mid << 16) | (base_low);
  return base;
}

uint32_t get_limit(uint8_t *p_desc) {
  uint32_t limit1 = (uint32_t) * (uint16_t *)p_desc;
  uint32_t limit2 = (uint32_t)(*(p_desc + 6) & 0x0F);
  uint32_t limit = (limit2 << 16) | (limit1);
  return limit;
}

uint32_t granularity(uint8_t *p_desc) {
  uint8_t attr2 = (*(p_desc + 6) >> 4) & 0x0F;
  uint32_t granu = ((attr2 >> 3) & 0x01) ? 4096 : 1;
  return granu;
}
