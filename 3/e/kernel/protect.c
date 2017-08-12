#include "protect.h"


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
