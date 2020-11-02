/*
 * Wireless Multicast Forwarding
 *
 *   Copyright 2020 Broadcom
 *
 *   This program is the proprietary software of Broadcom and/or
 *   its licensors, and may only be used, duplicated, modified or distributed
 *   pursuant to the terms and conditions of a separate, written license
 *   agreement executed between you and Broadcom (an "Authorized License").
 *   Except as set forth in an Authorized License, Broadcom grants no license
 *   (express or implied), right to use, or waiver of any kind with respect to
 *   the Software, and Broadcom expressly reserves all rights in and to the
 *   Software and all intellectual property rights therein.  IF YOU HAVE NO
 *   AUTHORIZED LICENSE, THEN YOU HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY
 *   WAY, AND SHOULD IMMEDIATELY NOTIFY BROADCOM AND DISCONTINUE ALL USE OF
 *   THE SOFTWARE.
 *
 *   Except as expressly set forth in the Authorized License,
 *
 *   1. This program, including its structure, sequence and organization,
 *   constitutes the valuable trade secrets of Broadcom, and you shall use
 *   all reasonable efforts to protect the confidentiality thereof, and to
 *   use this information only in connection with your use of Broadcom
 *   integrated circuit products.
 *
 *   2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED
 *   "AS IS" AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES,
 *   REPRESENTATIONS OR WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR
 *   OTHERWISE, WITH RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY
 *   DISCLAIMS ANY AND ALL IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY,
 *   NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE, LACK OF VIRUSES,
 *   ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION OR
 *   CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING
 *   OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
 *
 *   3. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL
 *   BROADCOM OR ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL,
 *   SPECIAL, INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR
 *   IN ANY WAY RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN
 *   IF BROADCOM HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii)
 *   ANY AMOUNT IN EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF
 *   OR U.S. $1, WHICHEVER IS GREATER. THESE LIMITATIONS SHALL APPLY
 *   NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
 *
 *  $Id: wlc_wmf.h 708017 2017-06-29 14:11:45Z $
 */

#ifndef _wlc_wmf_h_
#define _wlc_wmf_h_

/* Packet handling decision code */
#define WMF_DROP 0
#define WMF_NOP 1
#define WMF_TAKEN 2

#define IGMPV2_HOST_MEMBERSHIP_QUERY	0x11
#define MCAST_ADDR_UPNP_SSDP(addr) ((addr) == 0xeffffffa)

/* WMF instance specific information */
struct wlc_wmf_instance {
	wlc_info_t *wlc; /* Pointer to wlc structure */
	void *emfci;	/* Pointer to emfc instance */
	void *igsci;	/* Pointer to igsc instance */
};

/* Module attach and detach functions */
extern wmf_info_t *wlc_wmf_attach(wlc_info_t *wlc);
extern void wlc_wmf_detach(wmf_info_t *wmfi);

/* Add wmf instance to a bsscfg */
extern int32 wlc_wmf_instance_add(wlc_info_t *wlc, struct wlc_bsscfg *bsscfg);

/* Delete wmf instance from bsscfg */
extern void wlc_wmf_instance_del(wlc_bsscfg_t *bsscfg);

/* Start WMF on the bsscfg */
extern int wlc_wmf_start(wlc_bsscfg_t *bsscfg);

/* Stop WMF on the bsscfg */
extern void wlc_wmf_stop(wlc_bsscfg_t *bsscfg);

/* Add a station to the WMF interface list */
extern int wlc_wmf_sta_add(wlc_bsscfg_t *bsscfg, struct scb *scb);

/* Delete a station from the WMF interface list */
extern int wlc_wmf_sta_del(wlc_bsscfg_t *bsscfg, struct scb *scb);

/* WMF packet handler */
extern int wlc_wmf_packets_handle(wlc_bsscfg_t *bsscfg, struct scb *scb, void *p, bool frombss);

/* Enable/Disable feature to send multicast packets to host */
extern int wlc_wmf_mcast_data_sendup(wlc_bsscfg_t *bsscfg, bool set, bool enable);
#endif	/* _wlc_wmf_h_ */
