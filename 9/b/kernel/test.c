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

/* test for `brk()' and `sbrk()' */

/** test1 */
	printf("\n\n===TaskE test1===");
	printf("\nTaskE prg break = 0x%.8x", sbrk(0));
	brk(sbrk(0) + 0x3000 + 0xc);
	printf("\nTaskE prg break = 0x%.8x", sbrk(0));

// Result: 分配4页, mlimit被对齐到第4页末尾(即第5页的首字节位置)
//
//  start_brk                                brk     mlimit
//     |                                      |        |      
//     +-----------+-----------+-----------+-----------+-----------+-----------+
//     |           |           |           |           |           |           |
//     |           |           |           |           |           |           |
//     +-----------+-----------+-----------+-----------+-----------+-----------+

/** test2 */
	printf("\n\n===TaskE test2===");
	sbrk(0x100);
	printf("\nTaskE prg break = 0x%.8x", sbrk(0));

// Result: brk增长0x100, 未超过mlimit, 无需映射新的页面
//
//  start_brk                                  brk   mlimit
//     |                                        |      |      
//     +-----------+-----------+-----------+-----------+-----------+-----------+
//     |           |           |           |           |           |           |
//     |           |           |           |           |           |           |
//     +-----------+-----------+-----------+-----------+-----------+-----------+

/** test3 */
	printf("\n\n===TaskE test3===");
	sbrk(0x2000);
	printf("\nTaskE prg break = 0x%.8x", sbrk(0));

// Result: brk增长0x2000后超过mlimit, 需要映射2个页面, mlimit被对齐到第6页末尾
//
//  start_brk                                                          brk   mlimit
//     |                                                                |      |      
//     +-----------+-----------+-----------+-----------+-----------+-----------+
//     |           |           |           |           |           |           |
//     |           |           |           |           |           |           |
//     +-----------+-----------+-----------+-----------+-----------+-----------+

/** test4 */
	printf("\n\n===TaskE test4===");
	brk(sbrk(0) - 0x3005);
	printf("\nTaskE prg break = 0x%.8x", sbrk(0));

// Result: brk减去0x3005后需要解除3个页面的映射, mlimit被对齐到第3页末尾
//
//  start_brk                    brk     mlimit
//     |                          |        |      
//     +-----------+-----------+-----------+-----------+-----------+-----------+
//     |           |           |           |           |           |           |
//     |           |           |           |           |           |           |
//     +-----------+-----------+-----------+-----------+-----------+-----------+

/** test5 */
	printf("\n\n===TaskE test5===");
	sbrk(-0x108);
	printf("\nTaskE prg break = 0x%.8x", sbrk(0));

// Result: brk减去0x108后刚好需要解除1个页面的映射, mlimit被对齐到第2页末尾
//
//  start_brk                brk
//     |                      |   
//     +-----------+-----------+-----------+-----------+-----------+-----------+
//     |           |           |           |           |           |           |
//     |           |           |           |           |           |           |
//     +-----------+-----------+-----------+-----------+-----------+-----------+
//                             |
//                           mlimit

/** test6 */
	printf("\n\n===TaskE test6===");
	sbrk(2);
	printf("\nTaskE prg break = 0x%.8x", sbrk(0));

// Result: brk增加2后刚好越过第2页, 需要解除1个页面的映射, mlimit被对齐到第3页末尾
//
//  start_brk                  brk       mlimit
//     |                        |          |
//     +-----------+-----------+-----------+-----------+-----------+-----------+
//     |           |           |           |           |           |           |
//     |           |           |           |           |           |           |
//     +-----------+-----------+-----------+-----------+-----------+-----------+

/** test7 */
	printf("\n\n===TaskE test7===");
	sbrk(-0x4000);
	printf("\nTaskE prg break = 0x%.8x", sbrk(0));

// Result: brk减去0x4000后将越过下界start_brk, 属于非法操作, 不会修改brk和mlimit

/** test8 */
	printf("\n\n===TaskE test8===");
	sbrk(PROC_DATA_SEG_MAXSIZE);
	printf("\nTaskE prg break = 0x%.8x", sbrk(0));

// Result: brk将越过上界, 非法操作
	
	for (;;) {}
}

