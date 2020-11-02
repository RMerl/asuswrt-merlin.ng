/*
 * Monitor Mode routines.
 * This header file housing the define and function use by DHD
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
 *
 * <<Broadcom-WL-IPTag/Open:>>
 *
 * $Id: bcmwifi_monitor.h 512698 2016-02-11 13:12:15Z $
 */
#ifndef _BCMWIFI_MONITOR_H_
#define _BCMWIFI_MONITOR_H_

#include <monitor.h>
#include <bcmwifi_radiotap.h>

#define	MON_PKT_NON_AMSDU		1
#define	MON_PKT_AMSDU_FIRST		2
#define	MON_PKT_AMSDU_N_ONE		3
#define	MON_PKT_AMSDU_INTERMEDIATE	4
#define	MON_PKT_AMSDU_LAST		5
#define	MON_PKT_AMSDU_ERROR		6

typedef struct monitor_info monitor_info_t;

typedef struct rx_monitor_chain {
	uint            pkt_count;
	void            *pkthead;
	void            *pkttail;
} rx_monitor_chain_t;

struct monitor_info {
	ratespec_t		ampdu_rspec;	/* spec value for AMPDU sniffing */
	uint16			ampdu_counter;
	uint16			amsdu_len;
	uint8*			amsdu_pkt;
	int16			headroom;
	rx_monitor_chain_t	rxchain;
	uint32                  corerev;
};

typedef struct monitor_pkt_ts {
	union {
		uint32	ts_low; /* time stamp low 32 bits */
		uint32	reserved; /* If timestamp not used */
	};
	union {
		uint32  ts_high; /* time stamp high 28 bits */
		union {
			uint32  ts_high_ext :28; /* time stamp high 28 bits */
			uint32  clk_id_ext :3; /* clock ID source  */
			uint32  phase :1; /* Phase bit */
			uint32	marker_ext;
		};
	};
} monitor_pkt_ts_t;

typedef struct monitor_pkt_info {
	uint32	marker;
	/* timestamp */
	monitor_pkt_ts_t ts;
} monitor_pkt_info_t;

extern uint16 bcmwifi_monitor_create(monitor_info_t**);
extern void bcmwifi_monitor_delete(monitor_info_t* info);
extern uint16 bcmwifi_monitor(monitor_info_t* info,
		monitor_pkt_info_t* pkt_info, void *pdata, uint16 len,
		void* pout, int16* offset, uint16 *pkt_type, uint8 dma_flags, void  *phyextract);
extern uint32 wlc_he_sig_a1_from_plcp(uint8 *plcp);
extern uint32 wlc_he_sig_a2_from_plcp(uint8 *plcp);
#endif /* _BCMWIFI_MONITOR_H_ */
