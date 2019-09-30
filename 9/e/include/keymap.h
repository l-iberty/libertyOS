#ifndef KEYMAP_H
#define KEYMAP_H

#define TEXT_MASK 0x80 /* 用于区分 可打印/不可打印 字符 */

#define ESC			'?'
#define BACKSPACE	'?'
#define TAB			'?'
#define ENTER		('\n' | TEXT_MASK)
#define CTRL_L		'?'
#define SHIFT_L		'?'
#define SHIFT_R		'?'
#define ALT_L		'?'
#define F1			'?'
#define F2			'?'
#define F3			'?'
#define F4			'?'
#define F5			'?'
#define F6			'?'
#define F7			'?'
#define F8			'?'
#define F9			'?'
#define F10			'?'
#define CAPSLK		'?'

#define MC_ESC 0x01
#define MC_BACKSPACE 0x0E
#define MC_F1 0x3B
#define MC_F2 0x3C
#define MC_F3 0x3D
#define MC_F4 0x3E
#define MC_F5 0x3F
#define MC_F6 0x40
#define MC_F7 0x41
#define MC_F8 0x42
#define MC_F9 0x43
#define MC_F10 0x44
#define MC_SHIFT_L 0x2A
#define MC_SHIFT_R 0x36

#define KEYMAP_SIZE 0x45

extern char keymap[KEYMAP_SIZE];

#endif /* KEYMAP_H */
