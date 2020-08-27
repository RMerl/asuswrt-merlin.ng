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

#ifndef _NET_BATMAN_ADV_COMPAT_LINUX_IF_VLAN_H_
#define _NET_BATMAN_ADV_COMPAT_LINUX_IF_VLAN_H_

#include <linux/version.h>
#include_next <linux/if_vlan.h>

#if LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 33)

#define VLAN_PRIO_MASK          0xe000 /* Priority Code Point */
#define VLAN_PRIO_SHIFT         13

#endif /* < KERNEL_VERSION(2, 6, 33) */

#if LINUX_VERSION_CODE < KERNEL_VERSION(3, 0, 0)

#define vlan_insert_tag(skb, proto, vid) __vlan_put_tag(skb, vid)

#elif LINUX_VERSION_CODE < KERNEL_VERSION(3, 10, 0)

#define vlan_insert_tag(skb, proto, vid) vlan_insert_tag(skb, vid)

#endif /* < KERNEL_VERSION(3, 0, 0) */

#endif	/* _NET_BATMAN_ADV_COMPAT_LINUX_IF_VLAN_H_ */
