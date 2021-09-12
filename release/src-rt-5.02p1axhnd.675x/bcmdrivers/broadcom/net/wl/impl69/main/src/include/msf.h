/*
 * Common interface to MSF (multi-segment format) definitions.
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
 * $Id: msf.h 619634 2016-02-17 19:01:25Z $
 */

#ifndef _WLC_MSF_H_
#define _WLC_MSF_H_

struct wl_segment {
	uint32 type;
	uint32 offset;
	uint32 length;
	uint32 crc32;
	uint32 flags;
};
typedef struct wl_segment wl_segment_t;

struct wl_segment_info {
	uint8        magic[4];
	uint32       hdr_len;
	uint32       crc32;
	uint32       file_type;
	uint32       num_segments;
	wl_segment_t segments[1];
};
typedef struct wl_segment_info wl_segment_info_t;

typedef struct wlc_blob_segment {
	uint32 type;
	uint8  *data;
	uint32 length;
} wlc_blob_segment_t;

/** Segment types in Binary Eventlog Archive file */
enum bea_seg_type_e {
	MSF_SEG_TYP_RTECDC_BIN  = 1,
	MSF_SEG_TYP_LOGSTRS_BIN = 2,
	MSF_SEG_TYP_FW_SYMBOLS  = 3,
	MSF_SEG_TYP_ROML_BIN    = 4
};

#endif /* _WLC_MSF_H */
