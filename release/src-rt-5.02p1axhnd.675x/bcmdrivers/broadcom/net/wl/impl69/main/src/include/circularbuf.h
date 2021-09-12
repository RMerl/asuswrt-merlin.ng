/*
 * PCIe full dongle related circular buffer definition
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
 * $Id: circularbuf.h 596126 2015-10-29 19:53:48Z $
 */

#ifndef __CIRCULARBUF_H_INCLUDED__
#define __CIRCULARBUF_H_INCLUDED__

#include <osl.h>
#include <typedefs.h>
#include <bcmendian.h>

/** A circular buffer always resides in host memory. Core circularbuf circular buffer structure. */
typedef struct circularbuf_s
{
	uint16 depth;	/* Depth of circular buffer */
	uint16 r_ptr;	/* Read Ptr */
	uint16 w_ptr;	/* Write Ptr */
	uint16 e_ptr;	/* End Ptr */
	uint16 wp_ptr;	/* wp_ptr/pending - scheduled for DMA. But, not yet complete. */
	uint16 rp_ptr;	/* rp_ptr/pending - scheduled for DMA. But, not yet complete. */

	uint8  *buf_addr; /* pointer into host memory */
	void  *mb_ctx;
	void  (*mb_ring_bell)(void *ctx);
} circularbuf_t;

#endif /* __CIRCULARBUF_H_INCLUDED__ */
