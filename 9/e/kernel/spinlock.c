#include "spinlock.h"

void spinlock_init(struct spinlock *lock)
{
	lock->is_locked = 0;
}

void spinlock_aquire(struct spinlock *lock)
{
	while (lock->is_locked) {
		/* busy-waiting */
	}
	lock->is_locked = 1;
}

void spinlock_release(struct spinlock *lock)
{
	lock->is_locked = 0;
}
