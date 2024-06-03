/*
 * include/asm-microblaze/system.h -- Low-level interrupt/thread ops
 *
 *  Copyright (C) 2003	John Williams (jwilliams@itee.uq.edu.au)
 *			based upon microblaze version
 *  Copyright (C) 2001	NEC Corporation
 *  Copyright (C) 2001	Miles Bader <miles@gnu.org>
 *
 * This file is subject to the terms and conditions of the GNU General
 * Public License.  See the file COPYING in the main directory of this
 * archive for more details.
 *
 * Written by Miles Bader <miles@gnu.org>
 * Microblaze port by John Williams
 * Microblaze port by John Williams
 */

#ifndef __MICROBLAZE_SYSTEM_H__
#define __MICROBLAZE_SYSTEM_H__

#if 0
#include <linux/linkage.h>
#endif
#include <asm/ptrace.h>

#define prepare_to_switch()	do { } while (0)

/*
 * switch_to(n) should switch tasks to task ptr, first checking that
 * ptr isn't the current task, in which case it does nothing.
 */
struct thread_struct;
extern void *switch_thread (struct thread_struct *last,
			    struct thread_struct *next);
#define switch_to(prev,next,last) do {					\
	if (prev != next) {						\
		(last) = switch_thread (&prev->thread, &next->thread);	\
	}								\
} while (0)


/* Enable/disable interrupts.  */
#define __sti() \
{								\
	register unsigned tmp;					\
	__asm__ __volatile__ ("					\
			mfs	%0, rmsr;			\
			ori	%0, %0, 2;			\
			mts	rmsr, %0"			\
			: "=r" (tmp)				\
			:					\
			: "memory");				\
}

#define __cli() \
{								\
	register unsigned tmp;					\
	__asm__ __volatile__ ("					\
			mfs	%0, rmsr;			\
			andi	%0, %0, ~2;			\
			mts	rmsr, %0"			\
			: "=r" (tmp)				\
			:					\
			: "memory");				\
}

#define __save_flags(flags) \
	__asm__ __volatile__ ("mfs	%0, rmsr" : "=r" (flags))
#define __restore_flags(flags) \
	__asm__ __volatile__ ("mts	rmsr, %0" :: "r" (flags))

#define __save_flags_cli(flags) \
{								\
	register unsigned tmp;					\
	__asm__ __volatile__ ("					\
			mfs	%0, rmsr;			\
			andi	%1, %0, ~2;			\
			mts	rmsr, %1;"			\
			: "=r" (flags), "=r" (tmp)		\
			:					\
			: "memory");				\
}

#define __save_flags_sti(flags)					\
{								\
	register unsigned tmp;					\
	__asm__ __volatile__ ("					\
			mfs	%0, rmsr;			\
			ori	%1, %0, 2;			\
			mts	rmsr, %1;"			\
			: "=r" (flags) ,"=r" (tmp)		\
			:					\
			: "memory");				\
}

/* For spinlocks etc */
#define local_irq_save(flags)	__save_flags_cli (flags)
#define local_irq_set(flags)	__save_flags_sti (flags)
#define local_irq_restore(flags) __restore_flags (flags)
#define local_irq_disable()	__cli ()
#define local_irq_enable()	__sti ()

#define cli()			__cli ()
#define sti()			__sti ()
#define save_flags(flags)	__save_flags (flags)
#define restore_flags(flags)	__restore_flags (flags)
#define save_flags_cli(flags)	__save_flags_cli (flags)

/*
 * Force strict CPU ordering.
 * Not really required on microblaze...
 */
#define nop()			__asm__ __volatile__ ("nop")
#define mb()			__asm__ __volatile__ ("nop" ::: "memory")
#define rmb()			mb ()
#define wmb()			mb ()
#define set_mb(var, value)	do { var = value; mb(); } while (0)
#define set_wmb(var, value)	do { var = value; wmb (); } while (0)

#ifdef CONFIG_SMP
#define smp_mb()	mb ()
#define smp_rmb()	rmb ()
#define smp_wmb()	wmb ()
#else
#define smp_mb()	barrier ()
#define smp_rmb()	barrier ()
#define smp_wmb()	barrier ()
#endif

#define xchg(ptr, with) \
  ((__typeof__ (*(ptr)))__xchg ((unsigned long)(with), (ptr), sizeof (*(ptr))))
#define tas(ptr) (xchg ((ptr), 1))

static inline unsigned long __xchg(unsigned long with,
				    __volatile__ void *ptr, int size)
{
	unsigned long tmp, flags;

	save_flags_cli (flags);

	switch (size) {
	case 1:
		tmp = *(unsigned char *)ptr;
		*(unsigned char *)ptr = with;
		break;
	case 2:
		tmp = *(unsigned short *)ptr;
		*(unsigned short *)ptr = with;
		break;
	case 4:
		tmp = *(unsigned long *)ptr;
		*(unsigned long *)ptr = with;
		break;
	}

	restore_flags (flags);

	return tmp;
}

#endif /* __MICROBLAZE_SYSTEM_H__ */
