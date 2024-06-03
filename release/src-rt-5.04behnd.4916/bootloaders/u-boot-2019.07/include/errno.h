/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2014 Samsung Electronics
 * Przemyslaw Marczak <p.marczak@samsung.com>
 */
#ifndef _ERRNO_H
#define _ERRNO_H

#include <linux/errno.h>

extern int errno;

#define __set_errno(val) do { errno = val; } while (0)

#ifdef CONFIG_ERRNO_STR
const char *errno_str(int errno);
#else
static inline const char *errno_str(int errno)
{
	return 0;
}
#endif
#endif /* _ERRNO_H */
