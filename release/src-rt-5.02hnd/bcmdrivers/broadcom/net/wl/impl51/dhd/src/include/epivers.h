/*
 * Copyright (C) 2017, Broadcom. All Rights Reserved.
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
 * $Id: epivers.h.in,v 13.33 2010-09-08 22:08:53 $
 *
*/

#ifndef _epivers_h_
#define _epivers_h_

#define	EPI_MAJOR_VERSION	1

#define	EPI_MINOR_VERSION	363

#define	EPI_RC_NUMBER		45

#define	EPI_INCREMENTAL_NUMBER	84015

#define	EPI_BUILD_NUMBER	0

#define	EPI_VERSION		1, 363, 45, 84015

#define	EPI_VERSION_NUM		0x0116b2d1

#define EPI_VERSION_DEV		1.363.45

/* Driver Version String, ASCII, 32 chars max */
#ifdef WLTEST
#define	EPI_VERSION_STR		"1.363.45.84015 (r676423 WLTEST)"
#else
#define	EPI_VERSION_STR		"1.363.45.84015 (r676423)"
#endif

#endif /* _epivers_h_ */
