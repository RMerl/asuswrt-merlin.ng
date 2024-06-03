/*
<:copyright-gpl
 Copyright 2010 Broadcom Corp. All Rights Reserved.

 This program is free software; you can distribute it and/or modify it
 under the terms of the GNU General Public License (Version 2) as
 published by the Free Software Foundation.

 This program is distributed in the hope it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 for more details.

 You should have received a copy of the GNU General Public License along
 with this program; if not, write to the Free Software Foundation, Inc.,
 59 Temple Place - Suite 330, Boston MA 02111-1307, USA.
:>
 */

/*
 *--------------------------------------------------------------------------
 *
 * These functions allow binary only kernel modules to ensure that the
 * same important kernel config options used to compile the kernel module
 * did not change in the currently running kernel.
 *
 * Important kernel config options are:
 * SMP
 * PREEMPT
 * DEBUG_SPINLOCK
 * DEBUG_MUTEX
 *
 *--------------------------------------------------------------------------
 */

#ifndef __BCM_ASSERT_CONFIG_H__
#define __BCM_ASSERT_CONFIG_H__

extern int bcm_kernel_config_smp;
extern int bcm_kernel_config_preempt;
extern int bcm_kernel_config_debug_spinlock;
extern int bcm_kernel_config_debug_mutexes;

#define BCM_CONFIG_ERR0_AND_RETURN(mod, feature) \
		printk(KERN_ERR "%s compiled with %s enabled but in kernel its disabled!\n", mod, feature);\
		printk(KERN_ERR "Refusing to load %s.\n", mod); \
		return -1;

#define BCM_CONFIG_ERR1_AND_RETURN(mod, feature) \
		printk(KERN_ERR "%s compiled with %s disabled but in kernel its enabled!\n", mod, feature);\
		printk(KERN_ERR "Refusing to load %s.\n", mod); \
		return -1;

#ifdef CONFIG_SMP
#define BCM_CHECK_KERNEL_CONFIG_SMP(mod) \
	if (!bcm_kernel_config_smp) { BCM_CONFIG_ERR0_AND_RETURN(#mod, "CONFIG_SMP"); }
#else
#define BCM_CHECK_KERNEL_CONFIG_SMP(mod) \
	if (bcm_kernel_config_smp) { BCM_CONFIG_ERR1_AND_RETURN(#mod, "CONFIG_SMP"); }
#endif

#ifdef CONFIG_PREEMPT
#define BCM_CHECK_KERNEL_CONFIG_PREEMPT(mod) \
	if (!bcm_kernel_config_preempt) { BCM_CONFIG_ERR0_AND_RETURN(#mod, "CONFIG_PREEMPT"); }
#else
#define BCM_CHECK_KERNEL_CONFIG_PREEMPT(mod) \
	if (bcm_kernel_config_preempt) { BCM_CONFIG_ERR1_AND_RETURN(#mod, "CONFIG_PREEMPT"); }
#endif

#ifdef CONFIG_DEBUG_SPINLOCK
#define BCM_CHECK_KERNEL_CONFIG_DEBUG_SPINLOCK(mod) \
	if (!bcm_kernel_config_debug_spinlock) { BCM_CONFIG_ERR0_AND_RETURN(#mod, "CONFIG_DEBUG_SPINLOCK"); }
#else
#define BCM_CHECK_KERNEL_CONFIG_DEBUG_SPINLOCK(mod) \
	if (bcm_kernel_config_debug_spinlock) { BCM_CONFIG_ERR1_AND_RETURN(#mod, "CONFIG_DEBUG_SPINLOCK"); }
#endif

#ifdef CONFIG_DEBUG_MUTEXES
#define BCM_CHECK_KERNEL_CONFIG_DEBUG_MUTEXES(mod) \
	if (!bcm_kernel_config_debug_mutexes) { BCM_CONFIG_ERR0_AND_RETURN(#mod, "CONFIG_DEBUG_MUTEXES"); }
#else
#define BCM_CHECK_KERNEL_CONFIG_DEBUG_MUTEXES(mod) \
	if (bcm_kernel_config_debug_mutexes) { BCM_CONFIG_ERR1_AND_RETURN(#mod, "CONFIG_DEBUG_MUTEXES"); }
#endif


#endif /* __BCM_ASSERT_CONFIG_H__ */
