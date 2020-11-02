/*
 * d11 tx header caching module APIs - It caches the d11 header of
 * a packet and copy it to the next packet if possible.
 * This feature saves a significant amount of processing time to
 * build a packet's d11 header from scratch.
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
 * $Id: wlc_txc.h 708017 2017-06-29 14:11:45Z $
 */

#ifndef _wlc_txc_h_
#define _wlc_txc_h_

/* data APIs */
/* module states */
struct wlc_txc_info {
	bool _txc;	/* runtime enable check (non-user-configurable) */
};
/* return the enab value */
#define TXC_CACHE_ENAB(txc) ((txc)->_txc)

/* function APIs */
/* module entries */
extern wlc_txc_info_t *wlc_txc_attach(wlc_info_t *wlc);
extern void wlc_txc_detach(wlc_txc_info_t *txc);

/* update feature's runtime enab state */
extern void wlc_txc_upd(wlc_txc_info_t *txc);

/* invalidate the entry */
extern void wlc_txc_inv(wlc_txc_info_t *txc, struct scb *scb);

/* return invalidation control location */
/* XXX JQL
 * A bad idea to allow external entity to modify the location directly...
 * Fix the usage in wlc_rate_sel module to not rely on this...
 */
extern uint *wlc_txc_inv_ptr(wlc_txc_info_t *txc, struct scb *scb);

/* prepare sdu for tx - i.e. iv update */
extern void wlc_txc_prep_sdu(wlc_txc_info_t *txc, struct scb *scb,
	wlc_key_t *key, const wlc_key_info_t *key_info, void *p);

/* check if the cache has the entry */
extern bool wlc_txc_hit(wlc_txc_info_t *txc, struct scb *scb,
	void *pkt, uint pktlen, uint fifo, uint8 prio);
/* copy the header to the packet and return the address of the copy */
extern d11txh_t *wlc_txc_cp(wlc_txc_info_t *txc, struct scb *scb,
	void *pkt, uint *flags);
/* install an entry into the cache */
extern void wlc_txc_add(wlc_txc_info_t *txc, struct scb *scb,
	void *pkt, uint txhlen, uint fifo, uint8 prio, uint16 txh_off, uint d11hdr_len);

/* invalidate all entries */
extern void wlc_txc_inv_all(wlc_txc_info_t *txc);

/* get the offset of tx header start */
extern uint16 wlc_txc_get_txh_offset(wlc_txc_info_t *txc, struct scb *scb);
extern uint32 wlc_txc_get_d11hdr_len(wlc_txc_info_t *txc, struct scb *scb);

/* Get the rateinfo location in the txh when short header is enabled. */
uint8* wlc_txc_get_rate_info_shdr(wlc_txc_info_t *txc, int cache_idx);

#endif /* _wlc_txc_h_ */
