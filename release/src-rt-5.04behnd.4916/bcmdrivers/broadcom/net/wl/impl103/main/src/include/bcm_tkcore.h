/*
 * Interface to tiny kernel coredump facility available on some platforms
 *
 * Copyright (C) 2024, Broadcom. All Rights Reserved.
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
 * SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION
 * OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
 * CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 *
 * <<Broadcom-WL-IPTag/Open:>>
 *
 * $Id: bcm_tkcore.h 833345 2023-11-23 12:21:11Z $
 */

#ifndef __BCM_TKCORE_H
#define __BCM_TKCORE_H

#if defined(CONFIG_BCM_TINY_KCORE)
#include <linux/tiny_kcore.h>
#else
struct tkcore_alloc_entry;
struct module;
struct task_struct;

static inline void tkcore_add_module(struct module *mod) {}
static inline void tkcore_remove_module(struct module *mod) {}
static inline void tkcore_add_task(struct task_struct *tsk) {}
static inline void tkcore_remove_task(struct task_struct *tsk) {}
static inline void tkcore_add_alloc_iterator(void *ctx,
	int (*alloc_iter_fn)(void *ctx, struct tkcore_alloc_entry *entry)) {}
static inline void tkcore_remove_alloc_iterator(void *ctx) {}
#endif /* CONFIG_BCM_TINY_KCORE */

#endif /* __BCM_TKCORE_H */
