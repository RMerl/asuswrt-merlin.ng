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

#ifndef _NET_BATMAN_ADV_COMPAT_LINUX_NET_H_
#define _NET_BATMAN_ADV_COMPAT_LINUX_NET_H_

#include <linux/version.h>
#include_next <linux/net.h>

#if LINUX_VERSION_CODE < KERNEL_VERSION(3, 5, 0)

#ifndef net_ratelimited_function
#define net_ratelimited_function(func, ...) \
	do { \
		if (net_ratelimit()) \
			func(__VA_ARGS__); \
	} while (0)
#endif /* ifndef net_ratelimited_function */

#endif /* < KERNEL_VERSION(3, 5, 0) */

#endif	/* _NET_BATMAN_ADV_COMPAT_LINUX_NET_H_ */
