/**
 * Frame information structure
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
 * $Id: wlc_frmutil.h 708017 2017-06-29 14:11:45Z $
*/
#ifndef _WLC_FRMUTIL_H_
#define _WLC_FRMUTIL_H_

#include <wlc_key.h>

struct wlc_frminfo {
	struct dot11_header *h;		/* pointer to d11 header */
	uchar *pbody;			/* pointer to body of frame */
	uint body_len;			/* length of body after d11 hdr */
	uint len;			/* length of first pkt in chain */
	uint totlen;			/* length of entire pkt chain */
	void *p;			/* pointer to pkt */
	d11rxhdr_t *rxh;		/* pointer to rxhdr */
	uint16 fc;			/* frame control field */
	uint16 type;			/* frame type */
	uint16 subtype;			/* frame subtype */
	uint8 prio;			/* frame priority */
	int ac;				/* access category of frame */
	bool apsd_eosp;			/* TRUE if apsd eosp set */
	bool wds;			/* TRUE for wds frame */
	bool qos;			/* TRUE for qos frame */
	bool ht;			/* TRUE for frame with HT control field */
	bool ismulti;			/* TRUE for multicast frame */
	bool isamsdu;			/* TRUE for amsdu frame */
	bool htma;			/* TRUE for ht frame with embedded mgmt act */
	bool istdls;			/* TRUE for frame recd thru dpt link */
	bool bssid_match;		/* TRUE if bssid match */
	bool promisc_frame;		/* TRUE if promiscuous frame */
	int rx_wep;			/* wep frame */
	wlc_key_t *key;		/* key */
	wlc_key_info_t	key_info;	/* cached key info */
	uint32 WPA_auth;			/* WPA auth enabled */
	struct ether_header *eh;	/* pointer to ether header */
	struct ether_addr *sa;		/* pointer to source address */
	struct ether_addr *da;		/* pointer to dest address */
	uint8 plcp[D11_PHY_HDR_LEN];
	uint16 seq;			/* sequence number in host endian */
	wlc_d11rxhdr_t *wrxh;		/* pointer to rxhdr */
};

#endif /* _WLC_FRMUTIL_H_ */
