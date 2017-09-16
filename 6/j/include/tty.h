#ifndef TTY_H
#define TTY_H

#include "type.h"
#include "keyboard.h"
#include "console.h"

typedef struct {
	KB_INPUT kb_in;
	CONSOLE* p_console;
} TTY;


TTY	tty_table[NR_CONSOLES];

void	init_tty(TTY* p_tty);
void	init_console(TTY* p_tty);
int	is_current_console(TTY* p_tty);
void	tty_printchar(TTY* p_tty, char ch);
void	tty_backspace(TTY* p_tty);

#endif /* TTY_H */
