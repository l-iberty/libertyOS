#include "type.h"
#include "proc.h"

void _printf(const char* fmt, ...);

int sendrecv(int func_type, int pid, MESSAGE* p_msg); /* system call */

extern int is_current_proc_done;

/* Ring1 */
void Task_fs()
{
	is_current_proc_done = 0;
	
	_printf("-----Task_fs-----");
	
	MESSAGE msg;
	msg.value = DEV_OPEN;
	
	if (!sendrecv(SEND, PID_TASK_HD, &msg))
	{
		_printf("Sending message: src_pid = %.4x, dst_pid = %.4x, value = %.8x",
			msg.src_pid, msg.dst_pid, msg.value);
	}
	else
	{
		_printf("error: sendrecv [pid: %.4x]", p_current_proc->pid);
	}
	
	is_current_proc_done = 1;
	while(1) {}
}
