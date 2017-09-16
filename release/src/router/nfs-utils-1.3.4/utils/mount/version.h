/*
 * version.h -- get running kernel version
 *
 * Copyright (C) 2008 Oracle.  All rights reserved.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public
 * License along with this program; if not, write to the
 * Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 0211-1301 USA
 *
 */

#ifndef _NFS_UTILS_MOUNT_VERSION_H
#define _NFS_UTILS_MOUNT_VERSION_H

#include <stdio.h>
#include <limits.h>

#include <sys/utsname.h>

static inline unsigned int MAKE_VERSION(unsigned int p, unsigned int q,
					unsigned int r)
{
	return (65536 * p) + (256 * q) + r;
}

static inline unsigned int linux_version_code(void)
{
	struct utsname my_utsname;
	unsigned int p, q = 0, r = 0;

	/* UINT_MAX as backward compatibility code should not be run */
	if (uname(&my_utsname))
		return UINT_MAX;

	/* UINT_MAX as future versions might not start with an integer */
	if (sscanf(my_utsname.release, "%u.%u.%u", &p, &q, &r) < 1)
		return UINT_MAX;
	
	return MAKE_VERSION(p, q, r);
}

#endif	/* _NFS_UTILS_MOUNT_VERSION_H */
