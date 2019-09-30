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
#include "malloc.h"

#define N 8

/******************************** process entry ********************************/

void Init()
{
    int i, pid;
    struct message msg;
    msg.value = 1;
#if 1
    for (i = 0; i < 3; i++) {
        pid = fork();
        if (pid != 0) {
            printf("\n{Init} parent %d running, child pid: %d", getpid(), pid);
            msg.value++;
            sendrecv(SEND, pid, &msg);
        } else {
            printf("\n{Init} child %d running, parent pid: %d", getpid(), getppid());
            sendrecv(RECEIVE, PID_INIT, &msg);
            dump_msg(&msg);
            for (;;) {
            }
        }
    }
#else
    for (i = 0; i < 3; i++) {
        pid = fork();
        if (pid != 0) {
            printf("\n{Init} parent %d running, child pid: %d", getpid(), pid);
            sendrecv(RECEIVE, PID_INIT, &msg);
            dump_msg(&msg);
        } else {
            printf("\n{Init} child %d running, parent pid: %d", getpid(), getppid());
            msg.value++;
            sendrecv(SEND, pid, &msg);
            for (;;) {
            }
        }
    }
#endif
    for (;;) {
    }
}

void TaskA()
{
    int fd;
    size_t nr;
    char buf[32];
    char* str = "hello,world!";
    struct message msg;

//printf("\n---------{Task-A} Page Dir Base: 0x%.8x---------", getcr3());

/* malloc-free test */
#if 1
    int i;
    size_t s[N] = { 13, 100, 45, 20, 5, 28, 200, 33 };
    void* p[N];

    sbrk(3); // 先将program break设置为一个未对齐的地址
    printf("\nTaskA brk:%.8x", sbrk(0));
    printf("\nTaskA malloc-free test: ");
    for (i = 0; i < N - 1; i++) {
        p[i] = malloc(s[i]);
        printf("%.8x,", p[i]);
    }
    p[N - 1] = malloc(s[N - 1]);
    printf("%.8x", p[N - 1]);

    /* 验证进程地址空间的独立性 */
    /*printf("\nTaskA read a dword from %.8x: %.8x", p[0], *(uint32_t*)p[0]);
	*(uint32_t*)p[0] = 0xabab1234;
	printf("\nTaskA read a dword from %.8x AGAIN: %.8x", p[0], *(uint32_t*)p[0]);*/

    //print_chunks();

    free(p[0]), free(p[1]), free(p[3]), free(p[2]);
    free(p[6]), free(p[7]), free(p[4]);
    printf("\nTaskA brk:%.8x", sbrk(0));
    free(p[5]);
    printf("\nTaskA brk:%.8x", sbrk(0));
    //print_chunks();

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
#endif
    for (;;) {
    }
}

void TaskB()
{

#if 1
    int fd;
    size_t nr;
    char buf[32];
    char* str = "hello,unix!";
    struct message msg;

    //printf("\n---------{Task-B} Page Dir Base: 0x%.8x---------", getcr3());

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
#endif
    for (;;) {
    }
}

void TaskC()
{
#if 1
    int fd;
    char buf[32];
    size_t nr;
    char* str = "hello,libertyOS!";
    uint32_t* vm_base;
    struct message msg;

    //printf("\n---------{Task-C} Page Dir Base: 0x%.8x---------", getcr3());

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
#endif
    for (;;) {
    }
}

void TaskD()
{
    struct message msg;
    uint32_t* vm_base;

//printf("\n---------{Task-D} Page Dir Base: 0x%.8x---------", getcr3());

/* malloc-free test */
#if 0
	int i;
	size_t s[N] = {13,100,45,20,5,28,200,33};
	void *p[N];
	
	printf("\nTaskD malloc-free test: ");
	for (i = 0; i < N - 1; i++)
	{
		p[i] = malloc(s[i]);
		printf("%.8x,", p[i]);
	}
	p[N-1] = malloc(s[N-1]);
	printf("%.8x", p[N-1]);
	
	/* 验证进程地址空间的独立性 */
	printf("\nTaskD read a dword from %.8x: %.8x", p[0], *(uint32_t*)p[0]);
	*(uint32_t*)p[0] = 0xcdcdcdcd;
	printf("\nTaskD read a dword from %.8x AGAIN: %.8x", p[0], *(uint32_t*)p[0]);
	
	print_chunks();

	free(p[0]), free(p[1]), free(p[3]), free(p[2]);
	free(p[6]), free(p[7]), free(p[4]);
	printf("\nTaskD brk:%.8x", sbrk(0));
	free(p[5]);
	printf("\nTaskD brk:%.8x", sbrk(0));
	print_chunks();
#endif
    for (;;) {
    }
}

void TaskE()
{
#if 1
    int fd;
    size_t nr;
    char buf[32];
    struct message msg;

    //printf("\n---------{Task-E} Page Dir Base: 0x%.8x---------", getcr3());

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
#endif

/* test for `brk()' and `sbrk()' */
#if 0
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
#endif
    for (;;) {
    }
}
