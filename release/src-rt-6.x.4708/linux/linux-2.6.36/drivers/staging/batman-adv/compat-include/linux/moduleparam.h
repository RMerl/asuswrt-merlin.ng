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

#ifndef _NET_BATMAN_ADV_COMPAT_LINUX_MODULEPARAM_H_
#define _NET_BATMAN_ADV_COMPAT_LINUX_MODULEPARAM_H_

#include <linux/version.h>
#include_next <linux/moduleparam.h>

#if LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 31)

#define __compat__module_param_call(p1, p2, p3, p4, p5, p6, p7) \
	__module_param_call(p1, p2, p3, p4, p5, p7)

#else

#define __compat__module_param_call(p1, p2, p3, p4, p5, p6, p7) \
	__module_param_call(p1, p2, p3, p4, p5, p6, p7)

#endif /* < KERNEL_VERSION(2, 6, 31) */

#if LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 36)

struct kernel_param_ops {
	/* Returns 0, or -errno.  arg is in kp->arg. */
	int (*set)(const char *val, const struct kernel_param *kp);
	/* Returns length written or -errno.  Buffer is 4k (ie. be short!) */
	int (*get)(char *buffer, struct kernel_param *kp);
	/* Optional function to free kp->arg when module unloaded. */
	void (*free)(void *arg);
};

#define module_param_cb(name, ops, arg, perm)				\
	static int __compat_set_param_##name(const char *val,		\
					     struct kernel_param *kp)	\
				{ return (ops)->set(val, kp); }		\
	static int __compat_get_param_##name(char *buffer,		\
					     struct kernel_param *kp)	\
				{ return (ops)->get(buffer, kp); }	\
	__compat__module_param_call(MODULE_PARAM_PREFIX, name,		\
				    __compat_set_param_##name,		\
				    __compat_get_param_##name, arg,	\
				    __same_type((arg), bool *), perm)

static inline int batadv_param_set_copystring(const char *val,
					      const struct kernel_param *kp)
{
	return param_set_copystring(val, (struct kernel_param *)kp);
}

#define param_set_copystring batadv_param_set_copystring

#endif /* < KERNEL_VERSION(2, 6, 36) */

#endif	/* _NET_BATMAN_ADV_COMPAT_LINUX_MODULEPARAM_H_ */
