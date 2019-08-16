#ifndef SPINLOCK
#define SPINLOCK

#include "type.h"

struct spinlock {
	uint8_t is_locked;
};

void spinlock_init(struct spinlock *lock);
void spinlock_aquire(struct spinlock *lock);
void spinlock_release(struct spinlock *lock);


#endif /* SPINLOCK */
