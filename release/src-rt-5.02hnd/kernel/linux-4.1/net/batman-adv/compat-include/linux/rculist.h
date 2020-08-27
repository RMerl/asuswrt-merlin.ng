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

#ifndef _NET_BATMAN_ADV_COMPAT_LINUX_RCULIST_H_
#define _NET_BATMAN_ADV_COMPAT_LINUX_RCULIST_H_

#include <linux/version.h>
#include_next <linux/rculist.h>

#if LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 37)

#define hlist_first_rcu(head) \
	(*((struct hlist_node __rcu **)(&(head)->first)))

#define hlist_next_rcu(node) \
	(*((struct hlist_node __rcu **)(&(node)->next)))

#endif /* < KERNEL_VERSION(2, 6, 37) */

#if LINUX_VERSION_CODE < KERNEL_VERSION(3, 9, 0)

#undef hlist_for_each_entry_rcu
#define hlist_for_each_entry_rcu(pos, head, member) \
	for (pos = hlist_entry_safe(rcu_dereference_raw(hlist_first_rcu(head)),\
	typeof(*(pos)), member); \
	pos; \
	pos = hlist_entry_safe(rcu_dereference_raw(hlist_next_rcu(\
	&(pos)->member)), typeof(*(pos)), member))

#endif

#endif	/* _NET_BATMAN_ADV_COMPAT_LINUX_RCULIST_H_ */
