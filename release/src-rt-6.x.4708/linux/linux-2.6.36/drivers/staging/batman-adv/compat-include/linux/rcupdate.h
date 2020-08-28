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

#ifndef _NET_BATMAN_ADV_COMPAT_LINUX_RCUPDATE_H_
#define _NET_BATMAN_ADV_COMPAT_LINUX_RCUPDATE_H_

#include <linux/version.h>
#include_next <linux/rcupdate.h>

#if LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 34)

#define rcu_dereference_protected(p, c) (p)

#define rcu_dereference_raw(p)	({ \
				 typeof(p) _________p1 = ACCESS_ONCE(p); \
				 smp_read_barrier_depends(); \
				 (_________p1); \
				 })

#endif /* < KERNEL_VERSION(2, 6, 34) */

#if LINUX_VERSION_CODE < KERNEL_VERSION(3, 0, 0)

#define kfree_rcu(ptr, rcuhead_name) \
	do { \
		void batadv_free_rcu_##ptr(struct rcu_head *rcu) \
		{ \
			void *container_ptr; \
			container_ptr = container_of(rcu, typeof(*(ptr)), rcuhead_name); \
			kfree(container_ptr); \
		} \
		call_rcu(&(ptr)->rcuhead_name, batadv_free_rcu_##ptr); \
	} while (0)

#endif /* < KERNEL_VERSION(3, 0, 0) */

#endif	/* _NET_BATMAN_ADV_COMPAT_LINUX_RCUPDATE_H_ */
