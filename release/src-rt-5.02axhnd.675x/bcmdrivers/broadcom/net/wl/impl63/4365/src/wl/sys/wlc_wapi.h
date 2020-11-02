/*
 * WAPI (WLAN Authentication and Privacy Infrastructure) public header file
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
 * $Id: wlc_wapi.h 708017 2017-06-29 14:11:45Z $
 */

#ifndef _wlc_wapi_h_
#define _wlc_wapi_h_

/*
 * We always have BCMWAPI_WPI enabled we don't have
 * other cipher alternation instead of WPI (SMS4)
 */
#if defined(BCMWAPI_WPI) || defined(BCMWAPI_WAI)

#ifdef BCMWAPI_WAI

/* WAPI public bsscfg cubby structure and access macro */
typedef struct wlc_wapi_bsscfg_cubby_pub wlc_wapi_bsscfg_cubby_pub_t;

/* Use wlc instead of wapi to save one dereference instruction time */
#define WAPI_BSSCFG_PUB(wlc, cfg) \
	((wlc_wapi_bsscfg_cubby_pub_t *)BSSCFG_CUBBY((cfg), (wlc)->wapi_cfgh))

struct wlc_wapi_bsscfg_cubby_pub {
	/* data path variables provide macro access */
	bool	wai_restrict;	/* restrict data until WAI auth succeeds */
};
#define WAPI_WAI_RESTRICT(wlc, cfg)	(WAPI_BSSCFG_PUB((wlc), (cfg))->wai_restrict)

/* Macros for lookup the unicast/multicast ciphers for SMS4 in RSN */
#define WAPI_RSN_UCAST_LOOKUP(prsn)	(wlc_rsn_ucast_lookup((prsn), WAPI_CIPHER_SMS4))
#define WAPI_RSN_MCAST_LOOKUP(prsn)	((prsn)->multicast == WAPI_CIPHER_SMS4)

/*
 * WAI LLC header,
 * DSAP/SSAP/CTL = AA:AA:03
 * OUI = 00:00:00
 * Ethertype = 0x88b4 (WAI Port Access Entity)
 */
#define WAPI_WAI_HDR	"\xAA\xAA\x03\x00\x00\x00\x88\xB4"
#define WAPI_WAI_SNAP(pbody)	(bcmp(WAPI_WAI_HDR, (pbody), DOT11_LLC_SNAP_HDR_LEN) == 0)
#endif /* BCMWAPI_WAI */

/* module */
extern wlc_wapi_info_t *wlc_wapi_attach(wlc_info_t *wlc);
extern void wlc_wapi_detach(wlc_wapi_info_t *wapi);

#ifdef BCMWAPI_WAI
extern void wlc_wapi_station_event(wlc_wapi_info_t* wapi, wlc_bsscfg_t *bsscfg,
	const struct ether_addr *addr, void *ie, uint8 *gsn, uint16 msg_type);
#if !defined(WLNOEIND)
extern void wlc_wapi_bss_wai_event(wlc_wapi_info_t *wapi, wlc_bsscfg_t * bsscfg,
	const struct ether_addr *ea, uint8 *data, uint32 len);
#endif /* !WLNOEIND */
#endif /* BCMWAPI_WAI */

#endif /* BCMWAPI_WPI || BCMWAPI_WAI */
#endif /* _wlc_wapi_h_ */
