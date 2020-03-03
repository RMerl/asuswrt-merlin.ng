#ifndef __ASM_SPINLOCK_TYPES_H
#define __ASM_SPINLOCK_TYPES_H

#ifndef __LINUX_SPINLOCK_TYPES_H
# error "please don't include this file directly"
#endif

typedef struct {
	unsigned int lock;
} __attribute__ ((aligned (4))) arch_spinlock_t;

#define __ARCH_SPIN_LOCK_UNLOCKED { .lock = 0, }

typedef struct {
	unsigned int lock;
	unsigned int owner;
} arch_rwlock_t;

#define __ARCH_RW_LOCK_UNLOCKED		{ 0 }

#endif
