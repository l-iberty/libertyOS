#include "keyboard.h"
#include "console.h"
#include "global.h"
#include "irq.h"
#include "keymap.h"
#include "proc.h"
#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "tty.h"
#include "type.h"

char keymap[KEYMAP_SIZE] = {
    // Make Code
    /* 0x00 */ 0,
    /* 0x01 */ ESC,
    /* 0x02 */ ('1' | TEXT_MASK),
    /* 0x03 */ ('2' | TEXT_MASK),
    /* 0x04 */ ('3' | TEXT_MASK),
    /* 0x05 */ ('4' | TEXT_MASK),
    /* 0x06 */ ('5' | TEXT_MASK),
    /* 0x07 */ ('6' | TEXT_MASK),
    /* 0x08 */ ('7' | TEXT_MASK),
    /* 0x09 */ ('8' | TEXT_MASK),
    /* 0x0A */ ('9' | TEXT_MASK),
    /* 0x0B */ ('0' | TEXT_MASK),
    /* 0x0C */ ('-' | TEXT_MASK),
    /* 0x0D */ ('=' | TEXT_MASK),
    /* 0x0E */ BACKSPACE,
    /* 0x0F */ TAB,
    /* 0x10 */ ('q' | TEXT_MASK),
    /* 0x11 */ ('w' | TEXT_MASK),
    /* 0x12 */ ('e' | TEXT_MASK),
    /* 0x13 */ ('r' | TEXT_MASK),
    /* 0x14 */ ('t' | TEXT_MASK),
    /* 0x15 */ ('y' | TEXT_MASK),
    /* 0x16 */ ('u' | TEXT_MASK),
    /* 0x17 */ ('i' | TEXT_MASK),
    /* 0x18 */ ('o' | TEXT_MASK),
    /* 0x19 */ ('p' | TEXT_MASK),
    /* 0x1A */ ('[' | TEXT_MASK),
    /* 0x1B */ (']' | TEXT_MASK),
    /* 0x1C */ ENTER,
    /* 0x1D */ CTRL_L,
    /* 0x1E */ ('a' | TEXT_MASK),
    /* 0x1F */ ('s' | TEXT_MASK),
    /* 0x20 */ ('d' | TEXT_MASK),
    /* 0x21 */ ('f' | TEXT_MASK),
    /* 0x22 */ ('g' | TEXT_MASK),
    /* 0x23 */ ('h' | TEXT_MASK),
    /* 0x24 */ ('j' | TEXT_MASK),
    /* 0x25 */ ('k' | TEXT_MASK),
    /* 0x26 */ ('l' | TEXT_MASK),
    /* 0x27 */ (';' | TEXT_MASK),
    /* 0x28 */ (',' | TEXT_MASK),
    /* 0x29 */ ('`' | TEXT_MASK),
    /* 0x2A */ SHIFT_L,
    /* 0x2B */ ('\\' | TEXT_MASK),
    /* 0x2C */ ('z' | TEXT_MASK),
    /* 0x2D */ ('x' | TEXT_MASK),
    /* 0x2E */ ('c' | TEXT_MASK),
    /* 0x2F */ ('v' | TEXT_MASK),
    /* 0x30 */ ('b' | TEXT_MASK),
    /* 0x31 */ ('n' | TEXT_MASK),
    /* 0x32 */ ('m' | TEXT_MASK),
    /* 0x33 */ (',' | TEXT_MASK),
    /* 0x34 */ ('.' | TEXT_MASK),
    /* 0x35 */ ('/' | TEXT_MASK),
    /* 0x36 */ SHIFT_R,
    /* 0x37 */ 0, /* 没有这个扫描码 */
    /* 0x38 */ ALT_L,
    /* 0x39 */ (' ' | TEXT_MASK),
    /* 0x3A */ CAPSLK,
    /* 0x3B */ F1,
    /* 0x3C */ F2,
    /* 0x3D */ F3,
    /* 0x3E */ F4,
    /* 0x3F */ F5,
    /* 0x40 */ F6,
    /* 0x41 */ F7,
    /* 0x42 */ F8,
    /* 0x43 */ F9,
    /* 0x44 */ F10};

struct tty *p_current_tty;

void keyboard_handler(int irq) {
  uint8_t scan_code = in_byte(0x60);

  p_current_tty = tty_table + nr_current_console;

  if (scan_code & BREAK_MASK) /* 仅接收 Break Code */
  {
    if (p_current_tty->kb_in.count < KB_BUFSIZE) {
      *(p_current_tty->kb_in.p_rear++) = scan_code;
      p_current_tty->kb_in.count++;
      if (p_current_tty->kb_in.p_rear >= p_current_tty->kb_in.buf_queue + KB_BUFSIZE) {
        /* 队列已满, 回绕 */
        p_current_tty->kb_in.p_rear = p_current_tty->kb_in.buf_queue;
        p_current_tty->kb_in.count = 0;
      }
    }
  }
}

void keyboard_read(struct tty *p_tty, uint8_t *key) {
  *key = 0;
  if (p_tty->kb_in.count > 0) {
    *key = *(p_tty->kb_in.p_head++);
    p_tty->kb_in.count--;
    if (p_tty->kb_in.p_head == p_tty->kb_in.buf_queue + KB_BUFSIZE) {
      p_tty->kb_in.p_head = p_tty->kb_in.buf_queue;
    }
  }
}

void init_keyboard() {
  p_current_tty = NULL;
  put_irq_handler(IRQ_KEYBOARD, keyboard_handler);
  enable_irq(IRQ_KEYBOARD);
}
