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

#ifndef _NET_BATMAN_ADV_COMPAT_LINUX_KERNEL_H_
#define _NET_BATMAN_ADV_COMPAT_LINUX_KERNEL_H_

#include <linux/version.h>
#include_next <linux/kernel.h>

#if LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 39)

#define kstrtou32(cp, base, v)\
({\
	unsigned long _v;\
	int _r;\
	_r = strict_strtoul(cp, base, &_v);\
	*(v) = (u32)_v;\
	if ((unsigned long)*(v) != _v)\
		_r = -ERANGE;\
	_r;\
})

#define kstrtou64(cp, base, v)\
({\
	unsigned long long _v;\
	int _r;\
	_r = strict_strtoull(cp, base, &_v);\
	*(v) = (uint64_t)_v;\
	if ((unsigned long long)*(v) != _v)\
		_r = -ERANGE;\
	_r;\
})

#define kstrtoul strict_strtoul
#define kstrtol  strict_strtol

#endif /* < KERNEL_VERSION(2, 6, 39) */

#if LINUX_VERSION_CODE < KERNEL_VERSION(3, 14, 0)

#define U8_MAX		((u8)~0U)
#define S8_MAX		((s8)(U8_MAX >> 1))
#define S8_MIN		((s8)(-S8_MAX - 1))
#define U16_MAX		((u16)~0U)
#define S16_MAX		((s16)(U16_MAX >> 1))
#define S16_MIN		((s16)(-S16_MAX - 1))
#define U32_MAX		((u32)~0U)
#define S32_MAX		((s32)(U32_MAX >> 1))
#define S32_MIN		((s32)(-S32_MAX - 1))
#define U64_MAX		((u64)~0ULL)
#define S64_MAX		((s64)(U64_MAX >> 1))
#define S64_MIN		((s64)(-S64_MAX - 1))

#endif /* < KERNEL_VERSION(3, 14, 0) */

#endif	/* _NET_BATMAN_ADV_COMPAT_LINUX_KERNEL_H_ */
