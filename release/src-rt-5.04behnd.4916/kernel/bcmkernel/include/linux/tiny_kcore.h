/*
* <:copyright-BRCM:2023:DUAL/GPL:standard
*
*    Copyright (c) 2023 Broadcom
*    All Rights Reserved
*
* Unless you and Broadcom execute a separate written software license
* agreement governing use of this software, this software is licensed
* to you under the terms of the GNU General Public License version 2
* (the "GPL"), available at http://www.broadcom.com/licenses/GPLv2.php,
* with the following added to such license:
*
*    As a special exception, the copyright holders of this software give
*    you permission to link this software with independent modules, and
*    to copy and distribute the resulting executable under terms of your
*    choice, provided that you also meet, for each linked independent
*    module, the terms and conditions of the license of that module.
*    An independent module is a module which is not derived from this
*    software.  The special exception does not apply to any modifications
*    of the software.
*
* Not withstanding the above, under no circumstances may you combine
* this software in any way with any other Broadcom software provided
* under a license other than the GPL, without Broadcom's express prior
* written consent.
*
:>
*/
#ifndef __TINY_KCORE_H
#define __TINY_KCORE_H

/**
 * struct tkcore_alloc_entry - memory allocation information
 *
 * @addr: address of allocated memory
 * @size: size of allocated memory
 *
 * used by tkcore as argument calling the alloc iterator callback
 * function which should fill in the fields.
 */
struct tkcore_alloc_entry {
	void *addr;
	size_t size;
};

/* forward declarations */
struct module;
struct task_struct;
struct pt_regs;

#if defined(CONFIG_BCM_TINY_KCORE)
void tkcore_save_cpu_state(struct pt_regs *regs, uint cpu);
#else
static inline void tkcore_save_cpu_state(struct pt_regs *regs, uint cpu) {}
#endif
#if defined(CONFIG_BCM_TINY_KCORE)
void tkcore_add_module(struct module *mod);
void tkcore_remove_module(struct module *mod);
void tkcore_add_task(struct task_struct *tsk);
void tkcore_remove_task(struct task_struct *tsk);
void tkcore_add_alloc_iterator(void *ctx,
	int (*alloc_iter_fn)(void *ctx, struct tkcore_alloc_entry *entry));
void tkcore_remove_alloc_iterator(void *ctx);
#else
static inline void tkcore_add_module(struct module *mod) {}
static inline void tkcore_remove_module(struct module *mod) {}
static inline void tkcore_add_task(struct task_struct *tsk) {}
static inline void tkcore_remove_task(struct task_struct *tsk) {}
static inline void tkcore_add_alloc_iterator(void *ctx,
	int (*alloc_iter_fn)(void *ctx, struct tkcore_alloc_entry *entry)) {}
static inline void tkcore_remove_alloc_iterator(void *ctx) {}
#endif /* CONFIG_BCM_TINY_KCORE */

#endif /* __TINY_KCORE_H */