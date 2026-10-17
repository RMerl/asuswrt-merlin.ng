/*
 * wrapper include for dealing with location of stdarg.h as linux kernel
 * has its own incarnation of the file since v5.15.
 *
 * Copyright (C) 2025, Broadcom. All Rights Reserved.
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
 * $Id: bcmstdarg.h 834270 2023-12-14 13:44:17Z $
 */

#ifndef	_BCMSTDARG_H
#define	_BCMSTDARG_H

#if defined(__KERNEL__)
#include <linux/version.h>
#if LINUX_VERSION_CODE < KERNEL_VERSION(5, 15, 0)
#include <stdarg.h>
#else
#include <linux/stdarg.h>
#endif /* KERNEL < 5.15 */
#else
#include <stdarg.h>
#endif /* __KERNEL__ */

#endif /* _BCMSTDARG_H */
