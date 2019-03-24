#include "type.h"
#include "proc.h"
#include "tty.h"
#include "fs.h"
#include "mm.h"
#include "exe.h"
#include "sysconst.h"
#include "protect.h"
#include "keyboard.h"
#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "global.h"
#include "irq.h"


/******************************** process entry ********************************/

void Init()
{
	int i, pid;
	
	for (int i = 0; i < 3; i++)
	{
		pid = fork();
		if (pid != 0)
		{
			printf("\n{Init} parent %d running, child pid: %d", getpid(), pid);
		}
		else
		{
			printf("\n{Init} child %d running, parent pid: %d", getpid(), getppid());
			for(;;) {}
		}
	}
	
	for (;;) {}
}

void TaskA()
{
	int fd;
	size_t nr;
	char buf[32];
	char *str = "hello,world!";
	struct message msg;
	
	printf("\n---------{Task-A} Page Dir Base: 0x%.8x---------", getcr3());

/* file system test */

	fd = open("/test1", O_CREAT | O_RDWR);
	assert(fd != -1);
	write(fd, str, strlen(str));
	close(fd);
	
	fd = open("/test1", O_RDWR);
	assert(fd != -1);
	nr = read(fd, buf, sizeof(buf));
	assert(nr != -1);
	buf[nr] = 0;
	close(fd);
	
	printf("\nTaskA file test1: %s", buf);
	
	sendrecv(SEND, PID_TASK_E, &msg); /* tell TaskE to open/read file */

	for (;;)
	{
		
	}
}

void TaskB()
{
	int fd;
	size_t nr;
	char buf[32];
	char *str = "hello,unix!";
	struct message msg;
	
	printf("\n---------{Task-B} Page Dir Base: 0x%.8x---------", getcr3());

/* file system test */

	fd = open("/test2", O_CREAT | O_RDWR);
	assert(fd != -1);
	write(fd, str, strlen(str));
	close(fd);
	
	fd = open("/test2", O_RDWR);
	assert(fd != -1);
	nr = read(fd, buf, sizeof(buf));
	assert(nr != -1);
	buf[nr] = 0;
	close(fd);
	
	printf("\nTaskB file test2: %s", buf);
	
	sendrecv(SEND, PID_TASK_E, &msg); /* tell TaskE to open/read file */
	
	for (;;)
	{
		
	}
}

void TaskC()
{
	int fd;
	char buf[32];
	size_t nr;
	char *str = "hello,libertyOS!";
	uint32_t *vm_base;
	struct message msg;
	
	printf("\n---------{Task-C} Page Dir Base: 0x%.8x---------", getcr3());
	
/* vm system test -- copy on write */

	vm_base = (uint32_t*)vm_alloc(0x70000000, 4096 * 3, PAGE_READWRITE);
	assert(vm_base);
	
	vm_base[0] = 0xAAAAAAAA;
	vm_base[1] = 0xBBBBBBBB;
	printf("\nTaskC read 1 ==> %.8x %.8x", vm_base[0], vm_base[1]);
	
	sendrecv(SEND, PID_TASK_D, &msg); /* tell TaskD to read */
	
	sendrecv(RECEIVE, PID_TASK_D, &msg); /* wait for TaskD to write */
	
	printf("\nTaskC read 2 ==> %.8x %.8x", vm_base[0], vm_base[1]);
	
/* file system test */

	fd = open("/test3", O_CREAT | O_RDWR);
	assert(fd != -1);
	write(fd, str, strlen(str));
	close(fd);
	
	fd = open("/test3", O_RDWR);
	assert(fd != -1);
	nr = read(fd, buf, sizeof(buf));
	assert(nr != -1);
	buf[nr] = 0;
	close(fd);
	
	printf("\nTaskC file test3: %s", buf);

	sendrecv(SEND, PID_TASK_E, &msg); /* tell TaskE to open/read file */

	for (;;)
	{

	}
}

void TaskD()
{
	struct message msg;
	uint32_t *vm_base;
	
	printf("\n---------{Task-D} Page Dir Base: 0x%.8x---------", getcr3());

/* vm system test -- copy on write */

	vm_base = (uint32_t*)vm_alloc(0x80000000, 4096 * 3, PAGE_READ);
	assert(vm_base);
	
	sendrecv(RECEIVE, PID_TASK_C, &msg); /* wait for TaskC to write */
	
	printf("\nTaskD read 1 ==> %.8x %.8x", vm_base[0], vm_base[1]);
	
	vm_base[0] = 0xCCCCCCCC;
	vm_base[1] = 0xDDDDDDDD;
	printf("\nTaskD read 2 ==> %.8x %.8x", vm_base[0], vm_base[1]);
	
	sendrecv(SEND, PID_TASK_C, &msg); /* tell TaskC to read */
	
	for (;;)
	{

	}
}

void TaskE()
{
	int fd;
	size_t nr;
	char buf[32];
	struct message msg;
	
	printf("\n---------{Task-E} Page Dir Base: 0x%.8x---------", getcr3());
	
	sendrecv(RECEIVE, PID_TASK_A, &msg);
	sendrecv(RECEIVE, PID_TASK_B, &msg);
	sendrecv(RECEIVE, PID_TASK_C, &msg);
	
	fd = open("/test1", O_RDWR);
	assert(fd != -1);
	nr = read(fd, buf, sizeof(buf));
	assert(nr != -1);
	buf[nr] = 0;
	close(fd);
	printf("\nTaskE open \"test1\" to read: %s", buf);

	fd = open("/test2", O_RDWR);
	assert(fd != -1);
	nr = read(fd, buf, sizeof(buf));
	assert(nr != -1);
	buf[nr] = 0;
	close(fd);
	printf("\nTaskE open \"test2\" to read: %s", buf);
	
	fd = open("/test3", O_RDWR);
	assert(fd != -1);
	nr = read(fd, buf, sizeof(buf));
	assert(nr != -1);
	buf[nr] = 0;
	close(fd);
	printf("\nTaskE open \"test3\" to read: %s", buf);
	
	printf("\nTaskE prg break = 0x%.8x", sbrk(0));
	brk(sbrk(0) + 4096 * 2 + 1);
	printf("\nTaskE prg break = 0x%.8x", sbrk(0));
	
	void *old_brk = sbrk(0);
	sbrk(5000);
	void *new_brk = sbrk(0);
	printf("\nTaskE old_brk = 0x%.8x, new_brk = %.8x", old_brk, new_brk);
	
	for (;;)
	{
		
	}
}

