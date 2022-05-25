/*
 * Copyright (C) 2022, Broadcom. All Rights Reserved.
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
 * $Id: epivers.h.in 799514 2021-06-01 13:47:24Z $
 *
*/

#ifndef _epivers_h_
#define _epivers_h_

#define	EPI_MAJOR_VERSION	17

#define	EPI_MINOR_VERSION	10

#define	EPI_RC_NUMBER		188

#define	EPI_INCREMENTAL_NUMBER	6401

#define	EPI_BUILD_NUMBER	0

#define	EPI_VERSION		17, 10, 188, 6401

#define	EPI_VERSION_NUM		0x110abc19

#define EPI_VERSION_DEV		17.10.188

#define EPI_CUSTOM_VER ""
#if defined __has_include
#  if __has_include ("epicustomver.h")
#    include "epicustomver.h"
#  endif
#endif /* EPI_CUSTOM_VER */

/* Driver Version String, ASCII, (32 char limit for ethtool) */
#if defined(WLTEST)
#define	EPI_VERSION_STR		"17.10.188.6401 (r808804 WLTEST)" EPI_CUSTOM_VER
#elif defined(BCMDBG)
#define	EPI_VERSION_STR		"17.10.188.6401 (r808804 BCMDBG)" EPI_CUSTOM_VER
#else
#define	EPI_VERSION_STR		"17.10.188.6401 (r808804)" EPI_CUSTOM_VER
#endif

#endif /* _epivers_h_ */
