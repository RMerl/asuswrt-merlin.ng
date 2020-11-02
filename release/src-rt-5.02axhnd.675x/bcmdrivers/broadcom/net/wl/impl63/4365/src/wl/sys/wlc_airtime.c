/*
 * wlc_airtime.c
 *
 * Airtime fairness calculation utilities
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
 * $Id: wlc_airtime.c 708017 2017-06-29 14:11:45Z $
 *
 */

/**
 * @file
 * @brief
 * Airtime fairness feature was implemented is to address the issue of slower data transfers
 * throttling the higher speed ones. A transfer using a low phy rate physically takes a longer time
 * to transmit over the air. The present driver releases packets held in the AMPDU and NAR Tx
 * modules based on equal number of frames over the air causing the high throughput transfers to be
 * held up. This is magnified when using TCP as the round trip time on the high speed transfer goes
 * up significantly when there is a slow transfer going on at the same time. In order to address
 * this, the feature releases packets to meet a time target instead of a packet target over the air.
 */

/**
 * @file
 * @brief
 * XXX Twiki: [AirtimeFairness]
*/

/*
 * Include files.
 */
#include <wlc_cfg.h>
#include <osl.h>
#include <bcmutils.h>
#include <siutils.h>
#include <bcmwifi_channels.h>
#include <proto/802.11.h>
#include <d11.h>
#include <wlioctl.h>
#include <wlc_bsscfg.h>
#include <wlc.h>
#include <wlc_rate.h>
#include <wlc_airtime.h>

/* AMPDU Packet overhead timing. Assume DCF timing per sect 9.3.7 of 802.11-2012 */

/* PLCP time */
static INLINE uint
airtime_plcp_time_us(uint32 rspec, uint32 flg)
{
#define AIRTIME_PLCPMM_TIME_US		32
#define AIRTIME_PLCPGF_TIME_US		24

#define AIRTIME_PLCP_DSSSLONG_US		192
#define AIRTIME_PLCP_CCKSHORT_US	96
#define AIRTIME_PLCP_OFDM_US		20
#define AIRTIME_PLCP_HTGF_US		24
#define AIRTIME_PLCP_HTMM_US		32
#define AIRTIME_PLCP_VHT_US		32

	uint ret = 0;
	if (RSPEC_ISVHT(rspec)) {
		ret = (AIRTIME_PLCP_VHT_US);
	} else if (RSPEC_ISHT(rspec)) {
		ret = WLC_AIRTIME_MM(flg) ? (AIRTIME_PLCP_HTMM_US) : (AIRTIME_PLCP_HTGF_US);
	} else if (RSPEC_ISOFDM(rspec)) {
		ret = (AIRTIME_PLCP_OFDM_US);
	} else if (RSPEC_ISCCK(rspec)) {
		ret = (rspec & RSPEC_SHORT_PREAMBLE) ?
			(AIRTIME_PLCP_CCKSHORT_US) : (AIRTIME_PLCP_DSSSLONG_US);
	} else {
		WL_ERROR(("Unknown rspec: 0x%x\n", rspec));
		ASSERT(0);
	}

	return (ret);
}

/*
 * Slot time. Since we do VHT in 2GHz we have to consider
 * presence of non-ERP STAs, hence the long slot
 */

static INLINE uint
airtime_slot_time(uint32 flg)
{
#define AIRTIME_SHORT_SLOT_TIME_US	9
#define AIRTIME_LONG_SLOT_TIME_US	20
	return ((WLC_AIRTIME_SS(flg)) ?
		AIRTIME_SHORT_SLOT_TIME_US : AIRTIME_LONG_SLOT_TIME_US);
}

#define AIRTIME_SIFS_TIME_US		16
#define AIRTIME_MIN_SYMBOL_TIME_US	4
#define AIRTIME_ACK_BYTES		14
#define AIRTIME_BA_BYTES		32
#define AIRTIME_CTS_BYTES		14
#define AIRTIME_RTS_BYTES		20

/* Calculate minumum contention window, returns number of time slots. */
static INLINE uint
airtime_cwmin(wlc_bsscfg_t *cfg, uint ac)
{
	edcf_acparam_t *acp_ie = NULL;

	ASSERT(ac < AC_COUNT);

	if (BSSCFG_STA(cfg))
		acp_ie = &cfg->wme->wme_param_ie.acparam[ac];
	else if (BSSCFG_AP(cfg))
		acp_ie = &cfg->wme->wme_param_ie_ad->acparam[ac];

	ASSERT(acp_ie);

	return EDCF_ECW2CW(acp_ie->ECW & EDCF_ECWMIN_MASK);
}
/* Raw payload time */
/* Return packet time + SIFS given rate */
static INLINE uint
airtime_payload_time_us(uint32 flg, uint rate_kbps, uint size_in_bytes)
{
	return ROUNDUP(((8 * 1000 * size_in_bytes)/rate_kbps), AIRTIME_MIN_SYMBOL_TIME_US);
}

/* Return packet time + SIFS given rate */
static INLINE uint
airtime_packet_time_us(uint32 flg, uint rate_kbps, uint plcp_time_us, uint size_in_bytes)
{
	return airtime_payload_time_us(flg, rate_kbps, size_in_bytes) +
		AIRTIME_SIFS_TIME_US + plcp_time_us;
}

static INLINE uint
airtime_trailer_time(uint32 flg, uint ctl_rspec, uint ack_rspec)
{
	uint ack_bytes = (WLC_AIRTIME_BA(flg)) ? AIRTIME_BA_BYTES : AIRTIME_ACK_BYTES;
	uint ack_plcp_time_us = airtime_plcp_time_us(ack_rspec, flg);
	uint ack_rate_kbps = wlc_rate_rspec2rate(ack_rspec);

	uint ctl_plcp_time_us;
	uint ctl_rate_kbps;

	/* ACK or BA + SIFS */
	uint pkt_time = airtime_packet_time_us(flg, ack_rate_kbps, ack_plcp_time_us, ack_bytes);

	/*
	 * Avoid calculating PLCP time and rates multiple times if possible
	 * ACK and control rspecs are the same most of the time
	 * unless in 2.4G and protection is enabled
	 */
	if (ctl_rspec == ack_rspec)
	{
		ctl_plcp_time_us = ack_plcp_time_us;
		ctl_rate_kbps = ack_rate_kbps;
	} else {
		ctl_plcp_time_us = airtime_plcp_time_us(ctl_rspec, flg);
		ctl_rate_kbps = wlc_rate_rspec2rate(ctl_rspec);
	}

	if (WLC_AIRTIME_RTS(flg)) /* RTS + SIFS */
		pkt_time += airtime_packet_time_us(flg,
			ctl_rate_kbps, ctl_plcp_time_us, AIRTIME_RTS_BYTES);

	if (WLC_AIRTIME_CTS(flg)) /* CTS + SIFS */
		pkt_time += airtime_packet_time_us(flg,
			ctl_rate_kbps, ctl_plcp_time_us, AIRTIME_CTS_BYTES);

	return pkt_time;
}

/* Backoff is an average of the contention window */
static INLINE uint
airtime_backoff_time_us(uint cwmin, uint32 flg)
{
	return ((cwmin/2) * airtime_slot_time(flg));
}

/* DIFS per sect 9.3.7 of 802.11-2012 */
static INLINE uint
airtime_DIFS_time_us(uint32 flg)
{
	return (AIRTIME_SIFS_TIME_US + (2 * airtime_slot_time(flg)));
}

/* Header is DIFS + Backoff */
static INLINE uint
airtime_header_time_us(uint32 flg, uint cwmin)
{
	return (airtime_backoff_time_us(cwmin, flg) + airtime_DIFS_time_us(flg));
}

/*
 * Exported functions, externally visible
 */

/* Time of the PLCP header given rate.
 * Returns time in microseconds.
 */
BCMFASTPATH uint
wlc_airtime_plcp_time_us(uint32 rspec, uint32 flg)
{
	return airtime_plcp_time_us(rspec, flg);
}

/*
 * Payload packet time and PLCP excluding overhead
 * Partial calculation to help speed up AMPDU datapath
 * Returns time is microseconds.
 */
BCMFASTPATH uint
wlc_airtime_packet_time_us(uint32 flg, uint32 rspec, uint size_in_bytes)
{
	return airtime_packet_time_us(flg, wlc_rate_rspec2rate(rspec),
		airtime_plcp_time_us(rspec, flg), size_in_bytes);
}

/*
 * Packet payload only time based on number of bytes in frame and rate.
 * Returns time is microseconds.
 */

BCMFASTPATH uint
wlc_airtime_payload_time_us(uint32 flg, uint32 rspec, uint size_in_bytes)
{
	return airtime_payload_time_us(flg,  wlc_rate_rspec2rate(rspec), size_in_bytes);
}

/*
 * Packet overhead not including PLCP header of payload
 * Partial calculation to help speed up AMPDU datapath
 * Returns time is microseconds.
 */
uint BCMFASTPATH
wlc_airtime_pkt_overhead_us(uint flg, uint32 ctl_rspec,
	uint32 ack_rspec, wlc_bsscfg_t *cfg, uint ac)
{
	return (airtime_header_time_us(flg, airtime_cwmin(cfg, ac)) +
		airtime_trailer_time(flg, ctl_rspec, ack_rspec));
}

/*
 * Calculate the number of bytes of 802.11 Protocol overhead,
 * assuming a 3 address header and QoS format. Includes security wrapper and FCS.
 */
#define DOT11_QOS_HEADER_LEN	2
uint BCMFASTPATH
wlc_airtime_dot11hdrsize(uint32 wsec)
{
	uint pktbytes = DOT11_MAC_HDR_LEN + DOT11_QOS_LEN + DOT11_FCS_LEN;

	/* Add extra space for crypto headers
	 *  XXX What about WAPI and CKIP ??
	 */
	if (WSEC_AES_ENABLED(wsec)) {
			pktbytes += 16;
	} else if (WSEC_TKIP_ENABLED(wsec)) {
			pktbytes += 20;
	} else if (WSEC_TKIP_ENABLED(wsec)) {
			pktbytes += 8;
	}

	return (pktbytes);
}

uint BCMFASTPATH
airtime_rts_usec(uint32 flg, uint ctl_rspec)
{
	return airtime_packet_time_us(flg, wlc_rate_rspec2rate(ctl_rspec),
			airtime_plcp_time_us(ctl_rspec, flg), AIRTIME_RTS_BYTES);
}

uint BCMFASTPATH
airtime_cts_usec(uint32 flg, uint ctl_rspec)
{
	return airtime_packet_time_us(flg, wlc_rate_rspec2rate(ctl_rspec),
			airtime_plcp_time_us(ctl_rspec, flg), AIRTIME_CTS_BYTES);
}

uint BCMFASTPATH
airtime_ba_usec(uint32 flg, uint ctl_rspec)
{
	/* Using control rspecs for ACK as well, as they are the same
	 * most of the time unless in 2.4G and protection is enabled
	 */
	return airtime_trailer_time(flg, ctl_rspec, ctl_rspec);
}
