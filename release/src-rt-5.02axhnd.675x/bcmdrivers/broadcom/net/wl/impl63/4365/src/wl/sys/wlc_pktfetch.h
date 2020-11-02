/*
 * Header for the common Pktfetch use cases in WLC
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
 * $Id: wlc_pktfetch.h 467328 2014-04-03 01:23:40Z $
 */

#ifndef _wlc_pktfetch_h_
#define _wlc_pktfetch_h_

#ifdef BCMSPLITRX
#include <wlc_types.h>
#include <rte_pktfetch.h>
#include <wlc_frmutil.h>
#include <bcmendian.h>

typedef struct wlc_eapol_pktfetch_ctx {
	wlc_info_t *wlc;
	struct scb *scb;
	wlc_frminfo_t f;
	bool ampdu_path;
	bool ordered;
	struct pktfetch_info *pinfo;
	bool promisc;
} wlc_eapol_pktfetch_ctx_t;

#define PKTBODYOFFSZ            4
#define LLC_SNAP_HEADER_CHECK(lsh) \
			lsh->dsap == 0xaa && \
			lsh->ssap == 0xaa && \
			lsh->ctl == 0x03 && \
			lsh->oui[0] == 0 && \
			lsh->oui[1] == 0 && \
			lsh->oui[2] == 0

#define EAPOL_PKTFETCH_REQUIRED(lsh) \
	(ntoh16(lsh->type) == ETHER_TYPE_802_1X && \
		LLC_SNAP_HEADER_CHECK(lsh))

#ifdef WLNDOE
#define ICMP6_MIN_BODYLEN	(DOT11_LLC_SNAP_HDR_LEN + sizeof(struct ipv6_hdr)) + \
				sizeof(((struct icmp6_hdr *)0)->icmp6_type)
#define ICMP6_NEXTHDR_OFFSET	(sizeof(struct dot11_llc_snap_header) + \
				OFFSETOF(struct ipv6_hdr, nexthdr))
#define ICMP6_TYPE_OFFSET	(sizeof(struct dot11_llc_snap_header) + \
				sizeof(struct ipv6_hdr) + \
				OFFSETOF(struct icmp6_hdr, icmp6_type))

#define NDOE_PKTFETCH_REQUIRED(wlc, lsh, pbody, body_len) \
	(lsh->type == hton16(ETHER_TYPE_IPV6) && \
	LLC_SNAP_HEADER_CHECK(lsh) && \
	body_len >= (((uint8 *)lsh - (uint8 *)pbody) + ICMP6_MIN_BODYLEN) && \
	*((uint8 *)lsh + ICMP6_NEXTHDR_OFFSET) == ICMPV6_HEADER_TYPE && \
	(*((uint8 *)lsh + ICMP6_TYPE_OFFSET) == ICMPV6_PKT_TYPE_NS || \
	*((uint8 *)lsh + ICMP6_TYPE_OFFSET) == ICMPV6_PKT_TYPE_RA) && \
	NDOE_ENAB(wlc->pub))
#endif /* WLNDOE */

#ifdef WLTDLS
#define	WLTDLS_PKTFETCH_REQUIRED(wlc, lsh)	\
	(TDLS_ENAB(wlc->pub) && ntoh16(lsh->type) == ETHER_TYPE_89_0D)
#endif // endif

extern void wlc_recvdata_schedule_pktfetch(wlc_info_t *wlc, struct scb *scb,
	wlc_frminfo_t *f, bool promisc_frame, bool ordered);
extern bool wlc_pktfetch_required(wlc_info_t *wlc, void *p, uchar *pbody, uint body_len,
	wlc_key_info_t *key_info, bool skip_iv);
#if defined(PKTC) || defined(PKTC_DONGLE)
extern void wlc_sendup_schedule_pktfetch(wlc_info_t *wlc, void *pkt, uint32 body_offset);
extern bool wlc_sendup_chain_pktfetch_required(wlc_info_t *wlc, void *p, uint16 body_offset);
#endif // endif
#endif /* BCMSPLITRX */

#endif	/* _wlc_pktfetch_h_ */
