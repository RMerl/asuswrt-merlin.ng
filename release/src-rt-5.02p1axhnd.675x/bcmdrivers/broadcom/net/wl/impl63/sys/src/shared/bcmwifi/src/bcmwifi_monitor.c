/*
 * Monitor Mode routines.
 * This header file housing the Monitor Mode routines implementation.
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
 * <<Broadcom-WL-IPTag/Proprietary:>>
 *
 * $Id: bcmwifi_monitor.c 512698 2016-02-11 13:12:15Z $
 */

#ifndef WL11N
#define WL11N
#endif // endif
#ifndef WL11AC
#define WL11AC
#endif // endif
#ifndef WL11AX
#define WL11AX
#endif // endif

#include <bcmutils.h>
#include <bcmendian.h>
#include <bcmwifi_channels.h>
#include <monitor.h>
#include <bcmwifi_radiotap.h>
#include <bcmwifi_monitor.h>
#include <bcmwifi_rates.h>
#include <hndd11.h>
#include <802.11ax.h>

/* channel bandwidth */
#define WLC_10_MHZ	10	/**< 10Mhz channel bandwidth */
#define WLC_20_MHZ	20	/**< 20Mhz channel bandwidth */
#define WLC_40_MHZ	40	/**< 40Mhz channel bandwidth */
#define WLC_80_MHZ	80	/**< 80Mhz channel bandwidth */
#define WLC_160_MHZ	160	/**< 160Mhz channel bandwidth */

#define HDRCONV_PAD 2

#define D11REV_GE(var, val)     ((var) >= (val))
#define D11REV_IS(var, val)     ((var) == (val))

#define D11MINORREV_IS(var, val)        \
	                                 ((var) == (val))

#define RXS_SHORT_ENAB(rev)     (D11REV_GE(rev, 64) || \
		                                D11REV_IS(rev, 60) || \
		                                D11REV_IS(rev, 61) || \
		                                D11REV_IS(rev, 62))

#define IS_D11RXHDRSHORT(rxh, rev) ((RXS_SHORT_ENAB(rev) && \
			        ((D11RXHDR_ACCESS_VAL(rxh, rev, dma_flags)) & RXS_SHORT_MASK)) != 0)
#define RXHDR_GET_AMSDU(rxh, corerev, corerev_minor) (IS_D11RXHDRSHORT(rxh, corerev) ? \
	((D11RXHDRSHORT_ACCESS_VAL(rxh, corerev, corerev_minor, \
	mrxs) & RXSS_AMSDU_MASK) != 0) : D11REV_GE(corerev, 129) ? \
	((D11RXHDR_GE129_ACCESS_VAL(rxh, mrxs) & RXSS_AMSDU_MASK) != 0) : \
	((D11RXHDR_LT80_ACCESS_VAL(rxh, RxStatus2) & RXS_AMSDU_MASK) != 0))

#define RXHDR_GET_AGG_TYPE(rxh, corerev, corerev_minor) (IS_D11RXHDRSHORT(rxh, corerev) ? \
	((D11RXHDRSHORT_ACCESS_VAL(rxh, corerev, corerev_minor, mrxs) & \
	  RXSS_AGGTYPE_MASK) >> RXSS_AGGTYPE_SHIFT) : D11REV_GE(corerev, 128) ? \
	((D11RXHDR_GE128_ACCESS_VAL(rxh, mrxs) & RXSS_AGGTYPE_MASK) >> RXSS_AGGTYPE_SHIFT) : \
	((D11RXHDR_LT80_ACCESS_VAL(rxh, RxStatus2) & RXS_AGGTYPE_MASK) >> RXS_AGGTYPE_SHIFT))

#define RXHDR_GET_PAD_LEN(rxh, corerev, corerev_minor) (IS_D11RXHDRSHORT(rxh, corerev) ? \
	        (((D11RXHDRSHORT_ACCESS_VAL(rxh, corerev, corerev_minor, mrxs) & \
		           RXSS_PBPRES) != 0) ? HDRCONV_PAD : 0) : D11REV_GE(corerev, 128) ? \
	        (((D11RXHDR_GE128_ACCESS_VAL(rxh, mrxs) & RXSS_PBPRES) != 0) ? HDRCONV_PAD : 0) : \
	        (((D11RXHDR_LT80_ACCESS_VAL(rxh, RxStatus1) & RXS_PBPRES) != 0) ? HDRCONV_PAD : 0))

/** For accessing members of d11rxhdr_t by reference (address of members) */
#define D11PHYSTSBUF_ACCESS_REF_D(rxh, corerev, member) \
	 D11PHYSTSBUF_LT80_ACCESS_REF(rxh, member)

/** Calculate the rate of a received frame and return it as a ratespec (monitor mode) */
static ratespec_t BCMFASTPATH
wlc_recv_mon_compute_rspec(wlc_d11rxhdr_t *wrxh, uint8 *plcp, uint32 corerev)
{
	d11rxhdr_t *rxh = &wrxh->rxhdr;
	ratespec_t rspec;
	uint16 phy_ft;
	uint16 phy_ft_fmt;
	ASSERT(corerev < 128);
	plcp += D11_PHY_RXPLCP_OFF(corerev);

	phy_ft = *D11PHYSTSBUF_ACCESS_REF_D(rxh, corerev, PhyRxStatus_0) & PRXS0_FT_MASK;

	switch (phy_ft) {
		case FT_CCK:
			rspec = CCK_RSPEC(CCK_PHY2MAC_RATE(((cck_phy_hdr_t *)plcp)->signal));
			rspec |= WL_RSPEC_BW_20MHZ;
			break;
		case FT_OFDM:
			rspec = OFDM_RSPEC(OFDM_PHY2MAC_RATE(((ofdm_phy_hdr_t *)plcp)->rlpt[0]));
			rspec |= WL_RSPEC_BW_20MHZ;
			break;
		case FT_HT: {
			uint ht_sig1, ht_sig2;
			uint8 stbc;

			ht_sig1 = plcp[0];	/* only interested in low 8 bits */
			ht_sig2 = plcp[3] | (plcp[4] << 8); /* only interested in low 10 bits */

			rspec = HT_RSPEC((ht_sig1 & HT_SIG1_MCS_MASK));
			if (ht_sig1 & HT_SIG1_CBW) {
				/* indicate rspec is for 40 MHz mode */
				rspec |= WL_RSPEC_BW_40MHZ;
			} else {
				/* indicate rspec is for 20 MHz mode */
				rspec |= WL_RSPEC_BW_20MHZ;
			}
			if (ht_sig2 & HT_SIG2_SHORT_GI)
				rspec |= WL_RSPEC_SGI;
			if (ht_sig2 & HT_SIG2_FEC_CODING)
				rspec |= WL_RSPEC_LDPC;
			stbc = ((ht_sig2 & HT_SIG2_STBC_MASK) >> HT_SIG2_STBC_SHIFT);
			if (stbc != 0) {
				rspec |= WL_RSPEC_STBC;
			}
			break;
		}
		case FT_VHT:
			rspec = wf_vht_plcp_to_rspec(plcp);
			break;
		case FT_HE:
			phy_ft_fmt = *D11PHYSTSBUF_ACCESS_REF_D(rxh,
				corerev, PhyRxStatus_0) &
				PRXS_FTFMT_MASK(corerev);
#ifdef WL11AX
			/* HETB-SIG-A doesn't include rate info pass it using SIG-B */
			if (phy_ft_fmt == HE_FTFMT_HETB) {
				uint16 phy_ulrtinfo = HETB_ULRTINFO(rxh, corerev);
				plcp[6] = phy_ulrtinfo & 0xff;
				plcp[7] = (phy_ulrtinfo & 0xff00) >> 8;
			}
#endif // endif

			rspec = wf_he_plcp_to_rspec(plcp, phy_ft_fmt);
			break;
		default:
			/* return a valid rspec if not a debug/assert build */
			rspec = OFDM_RSPEC(6) | WL_RSPEC_BW_20MHZ;
			break;
	}

	return rspec;
} /* wlc_recv_compute_rspec */

uint32
wlc_he_sig_a1_from_plcp(uint8 *plcp)
{
	/* plcp offset handle: plcp += D11_PHY_RXPLCP_OFF(wlc->pub->corerev) */
	uint32 plcp0 = 0;
	ASSERT(plcp);
	plcp0 = ((plcp[3] << 24) | (plcp[2] << 16) | (plcp[1] << 8) | plcp[0]);
	return (plcp0 & HE_SIGA_MASK);
}

uint32
wlc_he_sig_a2_from_plcp(uint8 *plcp)
{
	uint32 plcp1 = 0;
	ASSERT(plcp);
	plcp1 = ((plcp[6] << 24) | (plcp[5] << 16) | (plcp[4] << 8) | plcp[3]) >> 2;
	return (plcp1 & HE_SIGA2_MASK);
}

/* recover 32bit TSF value from the 16bit TSF value */
/* assumption is time in rxh is within 65ms of the current tsf */
/* local TSF inserted in the rxh is at RxStart which is before 802.11 header */
static uint32
wlc_recover_tsf32(uint16 rxh_tsf, uint32 ts_tsf)
{
	uint16 rfdly;

	/* adjust rx dly added in RxTSFTime */
	/* XXX comment in d11.h:
	 * BWL_PRE_PACKED_STRUCT struct d11rxhdr {
	 *	...
	 *	uint16 RxTSFTime;	RxTSFTime time of first MAC symbol + M_PHY_PLCPRX_DLY
	 *	...
	 * }
	 */

	/* TODO: add PHY type specific value here... */
	rfdly = M_BPHY_PLCPRX_DLY;

	rxh_tsf -= rfdly;

	return (((ts_tsf - rxh_tsf) & 0xFFFF0000) | rxh_tsf);
}

static uint8
wlc_vht_get_gid(uint8 *plcp)
{
	uint32 plcp0 = plcp[0] | (plcp[1] << 8);
	return (plcp0 & VHT_SIGA1_GID_MASK) >> VHT_SIGA1_GID_SHIFT;
}

static uint16
wlc_vht_get_aid(uint8 *plcp)
{
	uint32 plcp0 = plcp[0] | (plcp[1] << 8) | (plcp[2] << 16);
	return (plcp0 & VHT_SIGA1_PARTIAL_AID_MASK) >> VHT_SIGA1_PARTIAL_AID_SHIFT;
}

static bool
wlc_vht_get_txop_ps_not_allowed(uint8 *plcp)
{
	return !!(plcp[2] & (VHT_SIGA1_TXOP_PS_NOT_ALLOWED >> 16));
}

static bool
wlc_vht_get_sgi_nsym_da(uint8 *plcp)
{
	return !!(plcp[3] & VHT_SIGA2_GI_W_MOD10);
}

static bool
wlc_vht_get_ldpc_extra_symbol(uint8 *plcp)
{
	return !!(plcp[3] & VHT_SIGA2_LDPC_EXTRA_OFDM_SYM);
}

static bool
wlc_vht_get_beamformed(uint8 *plcp)
{
	return !!(plcp[4] & (VHT_SIGA2_BEAMFORM_ENABLE >> 8));
}
/* Convert htflags and mcs values to
* rate in units of 500kbps
*/
static uint16
wlc_ht_phy_get_rate(uint8 htflags, uint8 mcs)
{

	ratespec_t rspec = HT_RSPEC(mcs);

	if (htflags & WL_RXS_HTF_40)
		rspec |= WL_RSPEC_BW_40MHZ;

	if (htflags & WL_RXS_HTF_SGI)
		rspec |= WL_RSPEC_SGI;

	return RSPEC2KBPS(rspec)/500;
}

uint16
wl_rxsts_to_rtap(struct wl_rxsts* rxsts, void *payload, uint16 len, void *pout, int16 *offset)
{
	uint16 rtap_len = 0;
	struct dot11_header* mac_header;
	uint8* p = payload;
	void *radio_tap;
	ASSERT(p && rxsts);

	if (rxsts->phytype == WL_RXS_PHY_N) {
		if (rxsts->encoding == WL_RXS_ENCODING_HT) {
			rtap_len = sizeof(wl_radiotap_ht_t);
		}
		else if (rxsts->encoding == WL_RXS_ENCODING_VHT) {
			rtap_len = sizeof(wl_radiotap_vht_t);
		}
		else if (rxsts->encoding == WL_RXS_ENCODING_HE) {
			rtap_len = sizeof(wl_radiotap_he_t);
		}
		else {
			rtap_len = sizeof(wl_radiotap_legacy_t);
		}
	} else {
		rtap_len = sizeof(wl_radiotap_legacy_t);
	}
#if defined(BCMDONGLEHOST)
	radio_tap = (void *)((uint8 *)pout - rtap_len);
#else
	radio_tap = pout;
	if (!(rxsts->nfrmtype & WL_RXS_NFRM_AMSDU_FIRST) &&
			!(rxsts->nfrmtype & WL_RXS_NFRM_AMSDU_SUB)) {
		memcpy((uint8*)pout + rtap_len, (uint8*)p, len);
	}
#endif /* BCMDONGLEHOST */
	mac_header = (struct dot11_header *)(p);
	len += rtap_len;

	if ((rxsts->phytype != WL_RXS_PHY_N) ||
		((rxsts->encoding != WL_RXS_ENCODING_HT) &&
		(rxsts->encoding != WL_RXS_ENCODING_VHT) &&
		(rxsts->encoding != WL_RXS_ENCODING_HE))) {
		wl_radiotap_rx_legacy(mac_header, rxsts,
			(wl_radiotap_legacy_t *)radio_tap);
	}
	else if (rxsts->encoding == WL_RXS_ENCODING_HE) {
		wl_radiotap_rx_he(mac_header, rxsts,
			(wl_radiotap_he_t *)radio_tap);
	}
	else if (rxsts->encoding == WL_RXS_ENCODING_VHT) {
		wl_radiotap_rx_vht(mac_header, rxsts,
			(wl_radiotap_vht_t *)radio_tap);
	}
	else if (rxsts->encoding == WL_RXS_ENCODING_HT) {
		wl_radiotap_rx_ht(mac_header, rxsts,
			(wl_radiotap_ht_t *)radio_tap);
	}
	*offset = rtap_len;
	return len;
}

/* Convert RX hardware status to standard format and send to wl_monitor
 * assume p points to plcp header
 */
static uint16
wl_d11rx_to_rxsts(monitor_info_t* info, monitor_pkt_info_t* pkt_info,
	wlc_d11rxhdr_t *wrxh, void *pkt, uint16 len, void *pout, int16 *offset,
	wl_phyextract_t  *phy_extract)
{
	struct wl_rxsts sts;
	uint32 rx_tsf_l;
	ratespec_t rspec;
	uint16 chan_num;
	uint8 *plcp;
	uint8 *p = (uint8*)pkt;
	uint8 hwrxoff = 0;
	uint32 ts_tsf = 0, corerev = 0;
	int8 rssi = 0, noise = 0;
	struct dot11_header *h;
	uint16 subtype;

	ASSERT(p);
	ASSERT(info);
	BCM_REFERENCE(chan_num);

	corerev = info->corerev;
	if (D11REV_GE(corerev, 128)) {
		hwrxoff = phy_extract->hwrxoff;
		rssi = phy_extract->rssi;
		noise = phy_extract->snr;
	} else {
		rssi = (pkt_info->marker >> 8) & 0xff;
		hwrxoff = (pkt_info->marker >> 16) & 0xff;
		noise = (int8)pkt_info->marker;
	}
	plcp = (uint8*)p + hwrxoff;

	plcp += RXHDR_GET_PAD_LEN(&wrxh->rxhdr, corerev, 0);

	ts_tsf = pkt_info->ts.ts_low;
	rx_tsf_l = wlc_recover_tsf32(D11RXHDR_ACCESS_VAL(&wrxh->rxhdr, corerev, RxTSFTime), ts_tsf);

	bzero((void *)&sts, sizeof(wl_rxsts_t));

	sts.mactime = rx_tsf_l;
	if (corerev < 128) {
		sts.antenna = (*D11PHYSTSBUF_ACCESS_REF_D(&wrxh->rxhdr, corerev, PhyRxStatus_0)
				& PRXS0_RXANT_UPSUBBAND) ? 1 : 0;
	}
	sts.signal = rssi;
	sts.noise = noise;
	sts.chanspec = D11RXHDR_ACCESS_VAL(&wrxh->rxhdr, corerev, RxChan);

	if (wf_chspec_malformed(sts.chanspec))
		return 0;

	chan_num = CHSPEC_CHANNEL(sts.chanspec);
	if (D11REV_GE(corerev, 128)) {
		rspec = phy_extract->rspec;
	} else {
		rspec = wlc_recv_mon_compute_rspec(wrxh, plcp, info->corerev);
	}

	plcp += RXHDR_GET_PAD_LEN(&wrxh->rxhdr, corerev, 0);
	h = (struct dot11_header *)(plcp + D11_PHY_RXPLCP_LEN(info->corerev));
	subtype = (ltoh16(h->fc) & FC_SUBTYPE_MASK) >> FC_SUBTYPE_SHIFT;

	if ((subtype == FC_SUBTYPE_QOS_DATA) || (subtype == FC_SUBTYPE_QOS_NULL)) {
		/* A-MPDU parsing */
		if (RSPEC_ISHT(rspec)) {
			if (WLC_IS_MIMO_PLCP_AMPDU(plcp)) {
				sts.nfrmtype |= WL_RXS_NFRM_AMPDU_FIRST;
				/* Save the rspec for later */
				info->ampdu_rspec = rspec;
			} else if (!(plcp[0] | plcp[1] | plcp[2])) {
				sts.nfrmtype |= WL_RXS_NFRM_AMPDU_SUB;
				/* Use the saved rspec */
				rspec = info->ampdu_rspec;
			}
		}
		else if (RSPEC_ISVHT(rspec)) {
			if ((plcp[0] | plcp[1] | plcp[2]) &&
				!(D11RXHDR_ACCESS_VAL(&wrxh->rxhdr, corerev, RxStatus2)
					& RXS_PHYRXST_VALID)) {
				/* First MPDU:
				 * PLCP header is valid, Phy RxStatus is not valid
				 */
				sts.nfrmtype |= WL_RXS_NFRM_AMPDU_FIRST;
				/* Save the rspec for later */
				info->ampdu_rspec = rspec;
				info->ampdu_counter++;
			} else if (!(plcp[0] | plcp[1] | plcp[2]) &&
				!(D11RXHDR_ACCESS_VAL(&wrxh->rxhdr, corerev, RxStatus2)
					& RXS_PHYRXST_VALID)) {
				/* Sub MPDU:
				 * PLCP header is not valid, Phy RxStatus is not valid
				 */
				sts.nfrmtype |= WL_RXS_NFRM_AMPDU_SUB;
				/* Use the saved rspec */
				rspec = info->ampdu_rspec;
			} else if ((plcp[0] | plcp[1] | plcp[2]) &&
				(D11RXHDR_ACCESS_VAL(&wrxh->rxhdr, corerev, RxStatus2)
				 & RXS_PHYRXST_VALID)) {
				/* MPDU is not a part of A-MPDU:
				 * PLCP header is valid and Phy RxStatus is valid
				 */
				info->ampdu_counter++;
			} else {
				/* Last MPDU */
				/* XXX: done to take care of the last MPDU in A-mpdu
				* VHT packets are considered A-mpdu
				* Use the saved rspec
				*/
				rspec = info->ampdu_rspec;
			}

			sts.ampdu_counter = info->ampdu_counter;
		}
	}
	/* A-MSDU parsing */
	if (D11RXHDR_ACCESS_VAL(&wrxh->rxhdr, corerev, RxStatus2)  & RXS_AMSDU_MASK) {
		/* it's chained buffer, break it if necessary */
		sts.nfrmtype |= WL_RXS_NFRM_AMSDU_FIRST | WL_RXS_NFRM_AMSDU_SUB;
	}

	if (PRXS5_ACPHY_DYNBWINNONHT(&wrxh->rxhdr))
		sts.vhtflags |= WL_RXS_VHTF_DYN_BW_NONHT;
	else
		sts.vhtflags &= ~WL_RXS_VHTF_DYN_BW_NONHT;

	switch (PRXS5_ACPHY_CHBWINNONHT(&wrxh->rxhdr)) {
	default: case PRXS5_ACPHY_CHBWINNONHT_20MHZ:
		sts.bw_nonht = WLC_20_MHZ;
		break;
	case PRXS5_ACPHY_CHBWINNONHT_40MHZ:
		sts.bw_nonht = WLC_40_MHZ;
		break;
	case PRXS5_ACPHY_CHBWINNONHT_80MHZ:
		sts.bw_nonht = WLC_80_MHZ;
		break;
	case PRXS5_ACPHY_CHBWINNONHT_160MHZ:
		sts.bw_nonht = WLC_160_MHZ;
		break;
	}

	/* prepare rate/modulation info */
	if (RSPEC_ISVHT(rspec)) {
		uint32 bw = RSPEC_BW(rspec);
		/* prepare VHT rate/modulation info */
		sts.nss = (rspec & WL_RSPEC_VHT_NSS_MASK) >> WL_RSPEC_VHT_NSS_SHIFT;
		sts.mcs = (rspec & WL_RSPEC_VHT_MCS_MASK);

		if (CHSPEC_IS80(sts.chanspec)) {
			if (bw == WL_RSPEC_BW_20MHZ) {
				switch (CHSPEC_CTL_SB(sts.chanspec)) {
					default:
					case WL_CHANSPEC_CTL_SB_LL:
						sts.bw = WL_RXS_VHT_BW_20LL;
						break;
					case WL_CHANSPEC_CTL_SB_LU:
						sts.bw = WL_RXS_VHT_BW_20LU;
						break;
					case WL_CHANSPEC_CTL_SB_UL:
						sts.bw = WL_RXS_VHT_BW_20UL;
						break;
					case WL_CHANSPEC_CTL_SB_UU:
						sts.bw = WL_RXS_VHT_BW_20UU;
						break;
				}
			} else if (bw == WL_RSPEC_BW_40MHZ) {
				switch (CHSPEC_CTL_SB(sts.chanspec)) {
					default:
					case WL_CHANSPEC_CTL_SB_L:
						sts.bw = WL_RXS_VHT_BW_40L;
						break;
					case WL_CHANSPEC_CTL_SB_U:
						sts.bw = WL_RXS_VHT_BW_40U;
						break;
				}
			} else {
				sts.bw = WL_RXS_VHT_BW_80;
			}
		} else if (CHSPEC_IS40(sts.chanspec)) {
			if (bw == WL_RSPEC_BW_20MHZ) {
				switch (CHSPEC_CTL_SB(sts.chanspec)) {
					default:
					case WL_CHANSPEC_CTL_SB_L:
						sts.bw = WL_RXS_VHT_BW_20L;
						break;
					case WL_CHANSPEC_CTL_SB_U:
						sts.bw = WL_RXS_VHT_BW_20U;
						break;
				}
			} else if (bw == WL_RSPEC_BW_40MHZ) {
				sts.bw = WL_RXS_VHT_BW_40;
			}
		} else {
			sts.bw = WL_RXS_VHT_BW_20;
		}

		if (RSPEC_ISSTBC(rspec))
			sts.vhtflags |= WL_RXS_VHTF_STBC;
		if (wlc_vht_get_txop_ps_not_allowed(plcp))
			sts.vhtflags |= WL_RXS_VHTF_TXOP_PS;
		if (RSPEC_ISSGI(rspec)) {
			sts.vhtflags |= WL_RXS_VHTF_SGI;
			if (wlc_vht_get_sgi_nsym_da(plcp))
				sts.vhtflags |= WL_RXS_VHTF_SGI_NSYM_DA;
		}
		if (RSPEC_ISLDPC(rspec)) {
			sts.coding = WL_RXS_VHTF_CODING_LDCP;
			if (wlc_vht_get_ldpc_extra_symbol(plcp))
				sts.vhtflags |= WL_RXS_VHTF_LDPC_EXTRA;
		}
		if (wlc_vht_get_beamformed(plcp))
			sts.vhtflags |= WL_RXS_VHTF_BF;

		sts.gid = wlc_vht_get_gid(plcp);
		sts.aid = wlc_vht_get_aid(plcp);
		sts.datarate = RSPEC2KBPS(rspec)/500;
	} else if (RSPEC_ISHT(rspec)) {
		/* prepare HT rate/modulation info */
		sts.mcs = (rspec & WL_RSPEC_HT_MCS_MASK);

		if (CHSPEC_IS40(sts.chanspec) || CHSPEC_IS80(sts.chanspec)) {
			uint32 bw = RSPEC_BW(rspec);

			if (bw == WL_RSPEC_BW_20MHZ) {
				if (CHSPEC_CTL_SB(sts.chanspec) == WL_CHANSPEC_CTL_SB_L) {
					sts.htflags = WL_RXS_HTF_20L;
				} else {
					sts.htflags = WL_RXS_HTF_20U;
				}
			} else if (bw == WL_RSPEC_BW_40MHZ) {
				sts.htflags = WL_RXS_HTF_40;
			}
		}

		if (RSPEC_ISSGI(rspec))
			sts.htflags |= WL_RXS_HTF_SGI;
		if (RSPEC_ISLDPC(rspec))
			sts.htflags |= WL_RXS_HTF_LDPC;
		if (RSPEC_ISSTBC(rspec))
			sts.htflags |= (1 << WL_RXS_HTF_STBC_SHIFT);

		sts.datarate = wlc_ht_phy_get_rate(sts.htflags, sts.mcs);
	}
	else if (RSPEC_ISHE(rspec)) {
		sts.nss = (rspec & WL_RSPEC_HE_NSS_MASK) >> WL_RSPEC_HE_NSS_SHIFT;
		sts.mcs = (rspec & WL_RSPEC_HE_MCS_MASK);

		sts.sig_a1 = wlc_he_sig_a1_from_plcp(plcp + D11_PHY_RXPLCP_OFF(corerev));
		sts.sig_a2 = wlc_he_sig_a2_from_plcp(plcp + D11_PHY_RXPLCP_OFF(corerev));
	} else {
		/* round non-HT data rate to nearest 500bkps unit */
		sts.datarate = RSPEC2KBPS(rspec)/500;
	}

	sts.pktlength = D11RXHDR_ACCESS_VAL(&wrxh->rxhdr, corerev, RxFrameSize)
		- D11_PHY_RXPLCP_LEN(info->corerev);
	if (corerev < 128) {
		sts.sq = ((*D11PHYSTSBUF_ACCESS_REF_D(&wrxh->rxhdr, corerev, PhyRxStatus_1)
					& PRXS1_SQ_MASK) >> PRXS1_SQ_SHIFT);
	}

	sts.phytype = WL_RXS_PHY_N;

	if (RSPEC_ISCCK(rspec)) {
		sts.encoding = WL_RXS_ENCODING_DSSS_CCK;
		if (D11REV_GE(corerev, 128)) {
			sts.preamble = phy_extract->preamble;
		} else {
			sts.preamble = (*D11PHYSTSBUF_ACCESS_REF_D(&wrxh->rxhdr,
				corerev, PhyRxStatus_0)
				& PRXS0_SHORTH) ?
				WL_RXS_PREAMBLE_SHORT : WL_RXS_PREAMBLE_LONG;
		}
	} else if (RSPEC_ISOFDM(rspec)) {
		sts.encoding = WL_RXS_ENCODING_OFDM;
		sts.preamble = WL_RXS_PREAMBLE_SHORT;
	} else if (RSPEC_ISHE(rspec)) {
		sts.encoding = WL_RXS_ENCODING_HE;
	} else {	/* MCS rate */
		sts.encoding = WL_RXS_ENCODING_HT;
		if (RSPEC_ISVHT(rspec)) {
			sts.encoding = WL_RXS_ENCODING_VHT;
		}
		sts.preamble = (uint32)((D11HT_MMPLCPLen(&wrxh->rxhdr) != 0) ?
			WL_RXS_PREAMBLE_HT_MM : WL_RXS_PREAMBLE_HT_GF);
	}
	/* translate error code */
	if (D11RXHDR_ACCESS_VAL(&wrxh->rxhdr, corerev, RxStatus1) & RXS_DECERR)
		sts.pkterror |= WL_RXS_DECRYPT_ERR;
	if (D11RXHDR_ACCESS_VAL(&wrxh->rxhdr, corerev, RxStatus1) & RXS_FCSERR)
		sts.pkterror |= WL_RXS_CRC_ERROR;

	p += RXHDR_GET_PAD_LEN(&wrxh->rxhdr, corerev, 0);
	len -= RXHDR_GET_PAD_LEN(&wrxh->rxhdr, corerev, 0);

	p += (hwrxoff + D11_PHY_RXPLCP_LEN(info->corerev));
	len -= (hwrxoff + D11_PHY_RXPLCP_LEN(info->corerev));

	wl_rxsts_to_rtap(&sts, p, len, pout, offset);
	*offset -= info->headroom;

	return len;
}

static uint16
wl_monitor_amsdu(monitor_info_t* info, monitor_pkt_info_t* pkt_info, wlc_d11rxhdr_t *wrxh,
		void *pkt, uint16 len, void *pout, int16* offset, uint16 *pkt_type, uint8 dma_flags,
		wl_phyextract_t  *phy_extract)
{
	uint8 *p = pkt;
	uint8 hwrxoff;
	uint16  aggtype = 0;
	if (D11REV_GE(info->corerev, 128)) {
		hwrxoff = phy_extract->hwrxoff;
	} else {
		hwrxoff = (pkt_info->marker >> 16) & 0xff;
	}

	aggtype = RXHDR_GET_AGG_TYPE(&wrxh->rxhdr, info->corerev, 0);

	switch (aggtype) {
	case RXS_AMSDU_FIRST:
	case RXS_AMSDU_N_ONE:
		/* Flush any previously collected */
		if (info->amsdu_len) {
			info->amsdu_len = 0;
			info->amsdu_pkt = NULL;
			*pkt_type = MON_PKT_AMSDU_ERROR;
			return 0;
		}

		/* Save the new starting AMSDU subframe */
		info->amsdu_len = len;
		info->amsdu_pkt = pkt;
		info->headroom =  D11_PHY_RXPLCP_LEN(info->corerev) + hwrxoff;
		info->headroom  += RXHDR_GET_PAD_LEN(&wrxh->rxhdr, info->corerev, 0);

		*pkt_type = MON_PKT_AMSDU_FIRST;
		if (aggtype == RXS_AMSDU_N_ONE) {
			/* all-in-one AMSDU subframe */
			wl_d11rx_to_rxsts(info, pkt_info, wrxh, p,
					len, (void *)((uint8 *)pout + info->headroom), offset,
					phy_extract);

			*pkt_type = MON_PKT_AMSDU_N_ONE;
			info->amsdu_len = 0;
		}
		break;

	case RXS_AMSDU_INTERMEDIATE:
	case RXS_AMSDU_LAST:
	default:
		/* Check for previously collected */
		if (info->amsdu_len) {
			/* Append next AMSDU subframe */
			p += hwrxoff;
			len -= hwrxoff;
			p += RXHDR_GET_PAD_LEN(&wrxh->rxhdr, info->corerev, 0);
			len -= RXHDR_GET_PAD_LEN(&wrxh->rxhdr, info->corerev, 0);
			memcpy(info->amsdu_pkt + info->amsdu_len, p, len);
			info->amsdu_len += len;

			*pkt_type = MON_PKT_AMSDU_INTERMEDIATE;
			/* current code can handle only msdu's per amsdu
			 *  one we want to support multiple msdu's
			 *  we need to remove next line and allocate bigger buffers
			 */
			aggtype = RXS_AMSDU_LAST;
			/* complete AMSDU frame */
			if (aggtype == RXS_AMSDU_LAST) {
				*pkt_type = MON_PKT_AMSDU_LAST;
				wl_d11rx_to_rxsts(info, pkt_info, wrxh, info->amsdu_pkt,
					info->amsdu_len, info->amsdu_pkt + info->headroom, offset,
					phy_extract);
				info->amsdu_len = 0;
				info->headroom = 0;
			}
		}
		else
		{
			info->amsdu_len = 0;
			info->amsdu_pkt = NULL;
			info->headroom = 0;
			*pkt_type = MON_PKT_AMSDU_ERROR;
			return 0;
		}
		break;
	}

	return len;
}

uint16 bcmwifi_monitor_create(monitor_info_t** info)
{
	*info = MALLOC(NULL, sizeof(struct monitor_info));

	if (!(*info))
		return FALSE;

	(*info)->amsdu_len = 0;

	return TRUE;
}

void
bcmwifi_monitor_delete(monitor_info_t* info)
{
	if (info)
		MFREE(NULL, info, sizeof(struct monitor_info));
}

uint16
bcmwifi_monitor(monitor_info_t* info, monitor_pkt_info_t* pkt_info,
	void *pkt, uint16 len, void *pout, int16* offset,
	uint16 *pkt_type, uint8 dma_flags, void  *phyextract)
{
	wlc_d11rxhdr_t *wrxh = (wlc_d11rxhdr_t*)pkt;
	uint16 hwrxoff;
	wl_phyextract_t *phy_extract = (wl_phyextract_t *)phyextract;
	if (dma_flags != 0)
		dma_flags = D11RXHDR_ACCESS_VAL(&wrxh->rxhdr,
				info->corerev, dma_flags);

	if (RXHDR_GET_AMSDU(&wrxh->rxhdr, info->corerev, 0)) {
		return wl_monitor_amsdu(info, pkt_info, wrxh, pkt,
				len, pout, offset, pkt_type, dma_flags, phyextract);
	} else {
		if (info->amsdu_len) {
			info->amsdu_len = 0;
			info->amsdu_pkt = NULL;
			*pkt_type = MON_PKT_AMSDU_ERROR;
			return 0;
		}

		if (D11REV_GE(info->corerev, 128)) {
			hwrxoff = phy_extract->hwrxoff;
		} else {
			hwrxoff = (pkt_info->marker >> 16) & 0xff;
		}
		info->amsdu_len = 0; /* reset amsdu */
		*pkt_type = MON_PKT_NON_AMSDU;
		pout = (uint8 *)pout + hwrxoff + D11_PHY_RXPLCP_LEN(info->corerev);
		info->headroom = D11_PHY_RXPLCP_LEN(info->corerev) + hwrxoff;
		pout = (uint8 *)pout + RXHDR_GET_PAD_LEN(&wrxh->rxhdr, info->corerev, 0);
		info->headroom += RXHDR_GET_PAD_LEN(&wrxh->rxhdr, info->corerev, 0);
		wl_d11rx_to_rxsts(info, pkt_info, wrxh, pkt, len, pout, offset, phyextract);
		info->headroom = 0;
		return len;
	}
}
