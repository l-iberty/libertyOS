#ifndef TTY_H
#define TTY_H

#include "type.h"
#include "keyboard.h"
#include "console.h"

struct tty {
    struct kb_input kb_in;
    struct console* p_console;
};

extern struct tty tty_table[NR_CONSOLE];
extern int nr_current_console;

void init_tty(struct tty* p_tty);
void tty_printchar(struct tty* p_tty, char ch);
void tty_printstr(struct tty* p_tty, char* str);
void tty_backspace(struct tty* p_tty);
void tty_put_text_ch(struct tty* p_tty, char ch);
void disp_tips(struct tty* p_tty);
void reset_kb_buf(struct tty* p_tty);
void parse_input(struct tty* p_tty);

#endif /* TTY_H */
