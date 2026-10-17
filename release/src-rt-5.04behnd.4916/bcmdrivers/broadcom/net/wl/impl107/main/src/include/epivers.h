/*
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
 * $Id: epivers.h.in 842372 2024-07-13 06:37:32Z $
 *
 * FILE-CSTYLED
*/

#ifndef _epivers_h_
#define _epivers_h_

#define	EPI_MAJOR_VERSION	25

#define	EPI_MINOR_VERSION	438

#define	EPI_RC_NUMBER		64

#define	EPI_INCREMENTAL_NUMBER	0

#define	EPI_VERSION		25, 438, 64, 0

#define	EPI_VERSION_NUM		0x191b6400

#define EPI_VERSION_DEV		25.438.64

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
#define	EPI_VERSION_STR		"25.438.64 (r857276 WLTEST BCMDBG)" EPI_CUSTOM_VER EPI_BSP_PATCH
#elif defined(WLTEST)
#define	EPI_VERSION_STR		"25.438.64 (r857276 WLTEST)" EPI_CUSTOM_VER EPI_BSP_PATCH
#elif defined(BCMDBG)
#define	EPI_VERSION_STR		"25.438.64 (r857276 BCMDBG)" EPI_CUSTOM_VER EPI_BSP_PATCH
#else
#define	EPI_VERSION_STR		"25.438.64 (r857276)" EPI_CUSTOM_VER EPI_BSP_PATCH
#endif

#endif /* _epivers_h_ */
