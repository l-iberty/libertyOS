#include "keyboard.h"

void Task_tty()
{
	while (1)
	{
		keyboard_read();
	}
}
