/*
 * Custom OID/ioctl related helper functions.
 *
 * Copyright (C) 2015, Broadcom Corporation. All Rights Reserved.
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
 * <<Broadcom-WL-IPTag/Open:>>
 *
 * $Id: wlioctl_utils.h 589733 2015-09-30 14:03:04Z $
 */

#ifndef _wlioctl_utils_h_
#define _wlioctl_utils_h_

#include <wlioctl.h>

#ifndef BCMDRIVER
#define CCA_THRESH_MILLI	14
#define CCA_THRESH_INTERFERE	6

extern cca_congest_channel_req_t * cca_per_chan_summary(cca_congest_channel_req_t *input,
	cca_congest_channel_req_t *avg, bool percent);

extern int cca_analyze(cca_congest_channel_req_t *input[], int num_chans,
	uint flags, chanspec_t *answer);
#endif /* BCMDRIVER */

extern int wl_cntbuf_to_xtlv_format(void *ctx, void *cntbuf,
	int buflen, uint32 corerev);

/* Get data pointer of wlc layer counters tuple from xtlv formatted counters IOVar buffer. */
#define GET_WLCCNT_FROM_CNTBUF(cntbuf)						\
		bcm_get_data_from_xtlv_buf(((wl_cnt_info_t *)cntbuf)->data,	\
		((wl_cnt_info_t *)cntbuf)->datalen, WL_CNT_XTLV_WLC,		\
		NULL, BCM_XTLV_OPTION_ALIGN32)

#define CHK_CNTBUF_DATALEN(cntbuf, ioctl_buflen) do {					\
	if (((wl_cnt_info_t *)cntbuf)->datalen +			\
		OFFSETOF(wl_cnt_info_t, data) > ioctl_buflen)	\
		printf("%s: IOVAR buffer short!\n", __FUNCTION__);	\
} while (0)

#endif /* _wlioctl_utils_h_ */
