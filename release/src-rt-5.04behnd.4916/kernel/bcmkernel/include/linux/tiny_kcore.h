/*
* <:copyright-BRCM:2023:DUAL/GPL:standard
* 
*    Copyright (c) 2023 Broadcom 
*    All Rights Reserved
* 
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License, version 2, as published by
* the Free Software Foundation (the "GPL").
* 
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
* 
* 
* A copy of the GPL is available at http://www.broadcom.com/licenses/GPLv2.php, or by
* writing to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
* Boston, MA 02111-1307, USA.
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