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

#define NR_PHILOSOPHERS		5

STATIC void philosopher_eat(int i);
STATIC void philosopher_think(int i);

struct semaphore forks[NR_PHILOSOPHERS];
struct semaphore mutex;

void philosopher_eat(int i)
{
	printf("\n===== philosopher_%d eating... =====", i);
	sleep(1000);
}

void philosopher_think(int i)
{
	printf("\nphilosopher_%d thinking... ", i);
	sleep(1000);
}

/******************************** process entry ********************************/

void Init()
{
	int i;
	struct message msg;
	
	// Initialize semaphores for TaskA~E
	sem_init(&mutex, 1);
	for (i = 0; i < NR_PHILOSOPHERS; i++)
	{
		sem_init(&forks[i], 1);
	}
	
	// Wake up TaskA~E
	sendrecv(SEND, PID_TASK_A, &msg);
	sendrecv(SEND, PID_TASK_B, &msg);
	sendrecv(SEND, PID_TASK_C, &msg);
	sendrecv(SEND, PID_TASK_D, &msg);
	sendrecv(SEND, PID_TASK_E, &msg);
	
	for (;;) {}
}

void TaskA()
{
	int i = 0;
	struct message msg;
	
	//printf("\n---------{Task-A} Page Dir Base: 0x%.8x---------", getcr3());
	
	sendrecv(RECEIVE, PID_INIT, &msg);
	
	for (;;)
	{
		philosopher_think(i);
		
		sem_wait(&mutex);
		sem_wait(&forks[i]);
		sleep(1000);
		sem_wait(&forks[(i + 1) % NR_PHILOSOPHERS]);
		sem_post(&mutex);
		
		philosopher_eat(i);
		
		sem_post(&forks[(i + 1) % NR_PHILOSOPHERS]);
		sem_post(&forks[i]);
	}
}

void TaskB()
{
	int i = 1;
	struct message msg;
	
	//printf("\n---------{Task-B} Page Dir Base: 0x%.8x---------", getcr3());
	
	sendrecv(RECEIVE, PID_INIT, &msg);

	for (;;)
	{
		philosopher_think(i);
		
		sem_wait(&mutex);
		sem_wait(&forks[i]);
		sleep(1000);
		sem_wait(&forks[(i + 1) % NR_PHILOSOPHERS]);
		sem_post(&mutex);
		
		philosopher_eat(i);
		
		sem_post(&forks[(i + 1) % NR_PHILOSOPHERS]);
		sem_post(&forks[i]);
	}
}

void TaskC()
{
	int i = 2;
	struct message msg;
	
	//printf("\n---------{Task-C} Page Dir Base: 0x%.8x---------", getcr3());
	
	sendrecv(RECEIVE, PID_INIT, &msg);

	for (;;)
	{
		philosopher_think(i);
		
		sem_wait(&mutex);
		sem_wait(&forks[i]);
		sleep(1000);
		sem_wait(&forks[(i + 1) % NR_PHILOSOPHERS]);
		sem_post(&mutex);
		
		philosopher_eat(i);
		
		sem_post(&forks[(i + 1) % NR_PHILOSOPHERS]);
		sem_post(&forks[i]);
	}
}

void TaskD()
{
	int i = 3;
	struct message msg;
	
	//printf("\n---------{Task-D} Page Dir Base: 0x%.8x---------", getcr3());
	
	sendrecv(RECEIVE, PID_INIT, &msg);

	for (;;)
	{
		philosopher_think(i);

		sem_wait(&mutex);
		sem_wait(&forks[i]);
		sleep(1000);
		sem_wait(&forks[(i + 1) % NR_PHILOSOPHERS]);
		sem_post(&mutex);
		
		philosopher_eat(i);
		
		sem_post(&forks[(i + 1) % NR_PHILOSOPHERS]);
		sem_post(&forks[i]);
	}
}

void TaskE()
{
	int i = 4;
	struct message msg;
	
	//printf("\n---------{Task-E} Page Dir Base: 0x%.8x---------", getcr3());
	
	sendrecv(RECEIVE, PID_INIT, &msg);

	for (;;)
	{
		philosopher_think(i);
		
		sem_wait(&mutex);
		sem_wait(&forks[i]);
		sleep(1000);
		sem_wait(&forks[(i + 1) % NR_PHILOSOPHERS]);
		sem_post(&mutex);
		
		philosopher_eat(i);
		sem_post(&forks[(i + 1) % NR_PHILOSOPHERS]);
		sem_post(&forks[i]);
	}
}

