/* Copyright (C) 2007-2015 B.A.T.M.A.N. contributors:
 *
 * Marek Lindner, Simon Wunderlich
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of version 2 of the GNU General Public
 * License as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, see <http://www.gnu.org/licenses/>.
 *
 * This file contains macros for maintaining compatibility with older versions
 * of the Linux kernel.
 */

#ifndef _NET_BATMAN_ADV_COMPAT_LINUX_PERCPU_H_
#define _NET_BATMAN_ADV_COMPAT_LINUX_PERCPU_H_

#include <linux/version.h>
#include_next <linux/percpu.h>

#if LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 30)

#undef __alloc_percpu
#define __alloc_percpu(size, align) \
	percpu_alloc_mask((size), GFP_KERNEL, cpu_possible_map)

#endif /* < KERNEL_VERSION(2, 6, 30) */

#if LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 33)

#define this_cpu_add(x, c)	batadv_this_cpu_add(&(x), c)

static inline void batadv_this_cpu_add(u64 *count_ptr, size_t count)
{
	int cpu = get_cpu();
	*per_cpu_ptr(count_ptr, cpu) += count;
	put_cpu();
}

#endif /* < KERNEL_VERSION(2, 6, 33) */

#endif	/* _NET_BATMAN_ADV_COMPAT_LINUX_PERCPU_H_ */
