#if defined(CONFIG_BCM_KF_ASSERT) || !defined(CONFIG_BCM_IN_KERNEL)

/*
<:copyright-BRCM:2007:GPL/GPL:standard

   Copyright (c) 2007 Broadcom 
   All Rights Reserved

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License, version 2, as published by
the Free Software Foundation (the "GPL").

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.


A copy of the GPL is available at http://www.broadcom.com/licenses/GPLv2.php, or by
writing to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
Boston, MA 02111-1307, USA.

:>
*/

/*
 *--------------------------------------------------------------------------
 *
 * These asserts allow functions to basically (note 1) verify that the caller
 * of the function either has the lock or does not have the lock when calling
 * the function.
 * These asserts only work when "Enable Asserts" and "Enable Kernel Hacking",
 * "Enable Debug Spinlock" and "Enable Debug Mutexes" are selected from
 * the debug selection section of make menuconfig.
 *
 * Note 1: For spinlocks, the check is more actually thorough than the name
 * implies.  HAS_SPINLOCK_COND verifies the caller has the spinlock *AND*
 * the spinlock was acquired on the current CPU.  It should be impossible
 * for the caller to acquire the spinlock on a different CPU and then be
 * migrated to this CPU.
 * NOT_HAS_SPINLOCK verifies the caller does not have the spinlock *AND*
 * the spinlock is not currently held by any other process or thread context
 * on the same CPU.  If it is, the subsequent attempt by this function to
 * acquire the spinlock will deadlock.
 *--------------------------------------------------------------------------
 */

#ifndef __BCM_ASSERT_LOCKS_H__
#define __BCM_ASSERT_LOCKS_H__

#include <linux/bcm_assert.h>

#ifdef CONFIG_DEBUG_SPINLOCK
#include <linux/spinlock.h>
#include <linux/smp.h>
#define     HAS_SPINLOCK_COND(s)  (s->rlock.owner == current && s->rlock.owner_cpu == smp_processor_id())
#define NOT_HAS_SPINLOCK_COND(s)  (s->rlock.owner != current && s->rlock.owner_cpu != smp_processor_id())
#else
#define     HAS_SPINLOCK_COND(s)  (1)
#define NOT_HAS_SPINLOCK_COND(s)  (1)
#endif

#define BCM_ASSERT_HAS_SPINLOCK_C(s)       BCM_ASSERT_C(HAS_SPINLOCK_COND((s)))

#define BCM_ASSERT_HAS_SPINLOCK_V(s)       BCM_ASSERT_V(HAS_SPINLOCK_COND((s)))

#define BCM_ASSERT_HAS_SPINLOCK_R(s, ret)  BCM_ASSERT_R(HAS_SPINLOCK_COND((s)), ret)

#define BCM_ASSERT_HAS_SPINLOCK_A(s)       BCM_ASSERT_A(HAS_SPINLOCK_COND((s)))

#define BCM_ASSERT_NOT_HAS_SPINLOCK_C(s)   BCM_ASSERT_C(NOT_HAS_SPINLOCK_COND((s)))

#define BCM_ASSERT_NOT_HAS_SPINLOCK_V(s)   BCM_ASSERT_V(NOT_HAS_SPINLOCK_COND((s)))

#define BCM_ASSERT_NOT_HAS_SPINLOCK_R(s, ret)  BCM_ASSERT_R(NOT_HAS_SPINLOCK_COND((s)), ret)

#define BCM_ASSERT_NOT_HAS_SPINLOCK_A(s)   BCM_ASSERT_A(NOT_HAS_SPINLOCK_COND((s)))


#ifdef CONFIG_DEBUG_MUTEXES
#include <linux/mutex.h>
#define     HAS_MUTEX_COND(m)     (m->owner == current)
#define NOT_HAS_MUTEX_COND(m)     (m->owner != current)
#else
#define     HAS_MUTEX_COND(m)     (1)
#define NOT_HAS_MUTEX_COND(m)     (1)
#endif

#define BCM_ASSERT_HAS_MUTEX_C(m)       BCM_ASSERT_C(HAS_MUTEX_COND((m)))

#define BCM_ASSERT_HAS_MUTEX_V(m)       BCM_ASSERT_V(HAS_MUTEX_COND((m)))

#define BCM_ASSERT_HAS_MUTEX_R(m, ret)  BCM_ASSERT_R(HAS_MUTEX_COND((m)), ret)

#define BCM_ASSERT_HAS_MUTEX_A(m)       BCM_ASSERT_A(HAS_MUTEX_COND((m)))

#define BCM_ASSERT_NOT_HAS_MUTEX_C(m)   BCM_ASSERT_C(NOT_HAS_MUTEX_COND((m)))

#define BCM_ASSERT_NOT_HAS_MUTEX_V(m)   BCM_ASSERT_V(NOT_HAS_MUTEX_COND((m)))

#define BCM_ASSERT_NOT_HAS_MUTEX_R(m, ret)  BCM_ASSERT_R(NOT_HAS_MUTEX_COND((m)), ret)

#define BCM_ASSERT_NOT_HAS_MUTEX_A(m)   BCM_ASSERT_A(NOT_HAS_MUTEX_COND((m)))


#endif /* __BCM_ASSERT_LOCKS_H__ */

#endif // defined(CONFIG_BRCM_KF_ASSERT)

