/*
 * Copyright (C) 2023, Broadcom. All Rights Reserved.
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
 * $Id: epivers.h.in 821234 2023-02-06 14:16:52Z $
 *
 * FILE-CSTYLED
*/

#ifndef _epivers_h_
#define _epivers_h_

#define	EPI_MAJOR_VERSION	17

#define	EPI_MINOR_VERSION	10

#define	EPI_RC_NUMBER		369

#define	EPI_INCREMENTAL_NUMBER	1808

#define	EPI_BUILD_NUMBER	0

#define	EPI_VERSION		17, 10, 369, 1808

#define	EPI_VERSION_NUM		0x110a1717

#define EPI_VERSION_DEV		17.10.369

#define EPI_CUSTOM_VER ""
#if defined __has_include
#if __has_include ("epicustomver.h")
#include "epicustomver.h"
#endif
#endif /* EPI_CUSTOM_VER */

#define EPI_BSP_PATCH ""
#if defined __has_include
#if __has_include ("epibsppatch.h")
#include "epibsppatch.h"
#endif
#endif /* EPI_BSP_PATCH */

/* Driver Version String, ASCII, (32 char limit for ethtool) */
#if defined(WLTEST) && defined(BCMDBG)
#define	EPI_VERSION_STR		"17.10.369.1808 (r834385 WLTEST BCMDBG)" EPI_CUSTOM_VER EPI_BSP_PATCH
#elif defined(WLTEST)
#define	EPI_VERSION_STR		"17.10.369.1808 (r834385 WLTEST)" EPI_CUSTOM_VER EPI_BSP_PATCH
#elif defined(BCMDBG)
#define	EPI_VERSION_STR		"17.10.369.1808 (r834385 BCMDBG)" EPI_CUSTOM_VER EPI_BSP_PATCH
#else
#define	EPI_VERSION_STR		"17.10.369.1808 (r834385)" EPI_CUSTOM_VER EPI_BSP_PATCH
#endif

#endif /* _epivers_h_ */
