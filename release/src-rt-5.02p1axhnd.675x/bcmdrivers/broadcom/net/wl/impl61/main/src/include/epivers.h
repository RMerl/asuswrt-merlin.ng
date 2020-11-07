/*
 * Copyright (C) 2020, Broadcom. All Rights Reserved.
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
 * $Id: epivers.h.in 596126 2015-10-29 19:53:48Z $
 *
*/

#ifndef _epivers_h_
#define _epivers_h_

#define	EPI_MAJOR_VERSION	17

#define	EPI_MINOR_VERSION	10

#define	EPI_RC_NUMBER		99

#define	EPI_INCREMENTAL_NUMBER	27

#define	EPI_BUILD_NUMBER	0

#define	EPI_VERSION		17, 10, 99, 27

#define	EPI_VERSION_NUM		0x110a631b

#define EPI_VERSION_DEV		17.10.99

/* Driver Version String, ASCII, 32 chars max */
#ifdef WLTEST
#define	EPI_VERSION_STR		"17.10.99.27 (r783108 WLTEST)"
#else
#define	EPI_VERSION_STR		"17.10.99.27 (r783108)"
#endif // endif

#endif /* _epivers_h_ */
