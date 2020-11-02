/*
 * wlc_airtime.h
 *
 * This module contains the public external definitions for the airtime fairness utilities.
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
 * $Id: wlc_airtime.h 708017 2017-06-29 14:11:45Z $
 *
 */

#if !defined(__WLC_AIRTIME_H__)
#define __WLC_AIRTIME_H__

#define WLC_AIRTIME_USERTS		0x00000001
#define WLC_AIRTIME_USECTS		0x00000002
#define WLC_AIRTIME_SHORTSLOT		0x00000004
#define WLC_AIRTIME_MIXEDMODE		0x00000008
#define WLC_AIRTIME_AMPDU		0x00000010

#define WLC_AIRTIME_RTSCTS		(WLC_AIRTIME_USERTS | WLC_AIRTIME_USECTS)
#define WLC_AIRTIME_CTS2SELF		(WLC_AIRTIME_USECTS)
#define WLC_AIRTIME_RTS(f)		((f) & WLC_AIRTIME_USERTS)
#define WLC_AIRTIME_CTS(f)		((f) & WLC_AIRTIME_USECTS)
#define WLC_AIRTIME_SS(f)		((f) & WLC_AIRTIME_SHORTSLOT)
#define WLC_AIRTIME_MM(f)		((f) & WLC_AIRTIME_MIXEDMODE)
#define WLC_AIRTIME_BA(f)		((f) & WLC_AIRTIME_AMPDU)

#define WLC_AIRTIME_PMODE		2 /* ATF Pig mode.
					   * If enabled ATF will release up to airtime limit
					   */
#ifdef WLATF_PERC
#define WLC_AIRTIME_PERC		3 /* ATF Percentage mode. If enabled, airtime
					   * percentage per SCB/BSS is reported to pcie.
					   */
#endif /* WLATF_PERC */

/*
 * Packet overhead not including PLCP header of payload
 * Partial calculation to help speed up AMPDU datapath
 * Returns time is microseconds.
 */
extern uint BCMFASTPATH wlc_airtime_pkt_overhead_us(uint flags,
	uint32 ctl_rate_kbps, uint32 ack_rate_kbps, wlc_bsscfg_t *bsscfg, uint ac);

/*
 * Payload packet time and PLCP excluding overhead
 * Partial calculation to help speed up AMPDU datapath
 * Returns time is microseconds.
 */
extern uint BCMFASTPATH wlc_airtime_packet_time_us(uint32 flg,
	uint32 rspec, uint size_in_bytes);

/*
 * Packet payload only time based on number of bytes in frame and rate.
 * Returns time is microseconds.
 */
extern uint BCMFASTPATH wlc_airtime_payload_time_us(uint32 flg,
	uint32 rspec, uint size_in_bytes);

/*
 * Calculate the number of bytes of 802.11 Protocol overhead,
 * assuming a 3 address header and QoS format. Includes security wrapper and FCS.
 * Returns Number of bytes.
 */
extern uint BCMFASTPATH wlc_airtime_dot11hdrsize(uint32 wsec);

/* Time of the PLCP header given rate.
 * Returns time in microseconds.
 */
extern uint BCMFASTPATH wlc_airtime_plcp_time_us(uint32 rspec, uint32 flg);

extern uint BCMFASTPATH airtime_rts_usec(uint32 flg, uint ctl_rspec);
extern uint BCMFASTPATH airtime_cts_usec(uint32 flg, uint ctl_rspec);
extern uint BCMFASTPATH airtime_ba_usec(uint32 flg, uint ctl_rspec);

#endif /* __WLC_AIRTIME_H__ */
