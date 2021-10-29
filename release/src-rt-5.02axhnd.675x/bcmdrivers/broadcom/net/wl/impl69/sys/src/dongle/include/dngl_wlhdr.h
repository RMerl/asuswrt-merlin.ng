/*
 * Dongle WL Header definitions
 *
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
 * $Id: dngl_wlhdr.h 514727 2014-11-12 03:02:48Z $
 */

#ifndef _dngl_wlhdr_h_
#define _dngl_wlhdr_h_

typedef struct wl_header {
    uint8   type;           /* Header type */
    uint8   version;        /* Header version */
	int8	rssi;			/* RSSI */
	uint8	pad;			/* Unused */
} wl_header_t;

#define WL_HEADER_LEN   sizeof(wl_header_t)
#define WL_HEADER_TYPE  0
#define WL_HEADER_VER   1
#endif /* _dngl_wlhdr_h_ */
