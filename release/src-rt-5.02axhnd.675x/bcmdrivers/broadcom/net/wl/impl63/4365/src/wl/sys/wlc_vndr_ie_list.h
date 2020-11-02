/*
 * Vendor IE list manipulation functions for
 * Broadcom 802.11abg Networking Device Driver
 *
 * Copyright 2020 Broadcom
 *
 * This program is the proprietary software of Broadcom and/or
 * its licensors, and may only be used, duplicated, modified or distributed
 * pursuant to the terms and conditions of a separate, written license
 * agreement executed between you and Broadcom (an "Authorized License").
 * Except as set forth in an Authorized License, Broadcom grants no license
 * (express or implied), right to use, or waiver of any kind with respect to
 * the Software, and Broadcom expressly reserves all rights in and to the
 * Software and all intellectual property rights therein.  IF YOU HAVE NO
 * AUTHORIZED LICENSE, THEN YOU HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY
 * WAY, AND SHOULD IMMEDIATELY NOTIFY BROADCOM AND DISCONTINUE ALL USE OF
 * THE SOFTWARE.
 *
 * Except as expressly set forth in the Authorized License,
 *
 * 1. This program, including its structure, sequence and organization,
 * constitutes the valuable trade secrets of Broadcom, and you shall use
 * all reasonable efforts to protect the confidentiality thereof, and to
 * use this information only in connection with your use of Broadcom
 * integrated circuit products.
 *
 * 2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED
 * "AS IS" AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES,
 * REPRESENTATIONS OR WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR
 * OTHERWISE, WITH RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY
 * DISCLAIMS ANY AND ALL IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY,
 * NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE, LACK OF VIRUSES,
 * ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION OR
 * CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING
 * OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
 *
 * 3. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL
 * BROADCOM OR ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL,
 * SPECIAL, INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR
 * IN ANY WAY RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN
 * IF BROADCOM HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii)
 * ANY AMOUNT IN EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF
 * OR U.S. $1, WHICHEVER IS GREATER. THESE LIMITATIONS SHALL APPLY
 * NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
 *
 * $Id: wlc_vndr_ie_list.h 708017 2017-06-29 14:11:45Z $
 */

#ifndef _wlc_vndr_ie_list_h_
#define _wlc_vndr_ie_list_h_

#include <wlc_types.h>

/* Vendor IE definitions */
#define VNDR_IE_EL_HDR_LEN	(sizeof(void *))
#define VNDR_IE_MAX_TOTLEN	256	/* Space for vendor IEs in Beacons and Probe Responses */

struct vndr_ie_listel {
	struct vndr_ie_listel *next_el;
	vndr_ie_info_t vndr_ie_infoel;
};

int wlc_vndr_ie_buflen(const vndr_ie_buf_t *ie_buf, int len, int *bcn_ielen, int *prbrsp_ielen);

typedef bool (*vndr_ie_list_filter_fn_t)(void *arg, const vndr_ie_t *ie);

int wlc_vndr_ie_list_getlen_ext(const vndr_ie_listel_t *vndr_ie_listp,
	vndr_ie_list_filter_fn_t filter, void *arg, uint32 pktflag, int *totie);

#define wlc_vndr_ie_list_getlen(list, pktflag, totie) \
	wlc_vndr_ie_list_getlen_ext(list, NULL, NULL, pktflag, totie)

typedef bool (*vndr_ie_list_write_filter_fn_t)(void *arg, uint type, const vndr_ie_t *ie);

uint8 *wlc_vndr_ie_list_write_ext(const vndr_ie_listel_t *vndr_ie_listp,
	vndr_ie_list_write_filter_fn_t filter, void *arg, uint type, uint8 *cp,
	int buflen, uint32 pktflag);

#define wlc_vndr_ie_list_write(list, cp, buflen, pktflag) \
	wlc_vndr_ie_list_write_ext(list, NULL, NULL, -1, cp, buflen, pktflag)

vndr_ie_listel_t *wlc_vndr_ie_list_add_elem(osl_t *osh, vndr_ie_listel_t **vndr_ie_listp,
	uint32 pktflag, vndr_ie_t *vndr_iep);

int wlc_vndr_ie_list_add(osl_t *osh, vndr_ie_listel_t **vndr_ie_listp,
	const vndr_ie_buf_t *ie_buf, int len);

int wlc_vndr_ie_list_del(osl_t *osh, vndr_ie_listel_t **vndr_ie_listp,
	const vndr_ie_buf_t *ie_buf, int len);

void wlc_vndr_ie_list_free(osl_t *osh, vndr_ie_listel_t **vndr_ie_listpp);

int wlc_vndr_ie_list_set(osl_t *osh, const void *vndr_ie_setbuf,
	int vndr_ie_setbuf_len, vndr_ie_listel_t **vndr_ie_listp,
	bool *bcn_upd, bool *prbresp_upd);

int wlc_vndr_ie_list_get(const vndr_ie_listel_t *vndr_ie_listp,
	vndr_ie_buf_t *ie_buf, int len, uint32 pktflag);

vndr_ie_listel_t *wlc_vndr_ie_list_mod_elem(osl_t *osh, vndr_ie_listel_t **vndr_ie_listp,
	vndr_ie_listel_t *old_listel, uint32 pktflag, vndr_ie_t *vndr_iep);

int wlc_vndr_ie_list_mod_elem_by_type(osl_t *osh, vndr_ie_listel_t **vndr_ie_listp,
	uint8 type, uint32 pktflag, vndr_ie_t *vndr_iep);

int wlc_vndr_ie_list_del_by_type(osl_t *osh, vndr_ie_listel_t **vndr_ie_listp, uint8 type);

uint8 *wlc_vndr_ie_list_find_by_type(vndr_ie_listel_t *vndr_ie_listp, uint8 type);

#endif /* _wlc_vndr_ie_list_h_ */
