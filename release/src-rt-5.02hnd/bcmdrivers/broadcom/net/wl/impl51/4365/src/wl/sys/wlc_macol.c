/*
 * Common (OS-independent) portion of
 * Broadcom 802.11abg Networking Device Driver
 *
 * Copyright (C) 2012, Broadcom Corporation
 * All Rights Reserved.
 *
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom Corporation.
 *
 * $Id: wlc_macol.c 520176 2014-12-10 18:22:37Z $
 */

#include <wlc_cfg.h>
#include <typedefs.h>
#include <osl.h>
#include <bcmendian.h>
#include <wlioctl.h>
#include <d11.h>
#include <proto/802.1d.h>
#include <proto/802.3.h>
#include <proto/wpa.h>
#include <bcmcrypto/rc4.h>
#include <bcmcrypto/tkmic.h>
#include <bcmwpa.h>
#include <bcm_ol_msg.h>
#include <wlc_rate.h>
#include <wlc_pub.h>
#include <wlc.h>
#include <wlc_hw_priv.h>
#include <wlc_bmac.h>
#include <wlc_scandb.h>
#include <wlc_scan_priv.h>
#include <wlc_scan.h>
#include <wlc_scanol.h>
#include <wlc_bmac.h>
#include <wlc_keymgmt.h>
#include <wlc_macol.h>
#include <wl_export.h>

#define WLC_RX_CHANNEL(rxh)		(CHSPEC_CHANNEL((rxh)->RxChan))

/* Rate info used for TX */
#define WLC_RATE_6M	12	/* in 500kbps units */
#define WLC_RATE_1M	2	/* in 500kbps units */
#define	RATE_MASK	0x7f	/* Rate value mask w/o basic rate flag */
#define MAXRATE 	108
#define RSPEC_RATE_MASK         0x000000FF      /* Either 500Kbps units or MIMO MCS idx */
/* Rate info per rate: It tells whether a rate is ofdm or not and its phy_rate value */
extern const uint8 rate_info[];

static void wlc_macol_cck_plcp_set(int rate_500, uint length, uint8 *plcp);
static void wlc_macol_compute_cck_plcp(ratespec_t rspec, uint length, uint8 *plcp);
static void wlc_macol_compute_ofdm_plcp(ratespec_t rspec, uint32 length, uint8 *plcp);
static void wlc_macol_txh_iv_upd(macol_info_t *macol, d11txh_t *txh, uint8 *iv,
	tx_info_t *macol_txinfo);
static void wlc_macol_toe_add_hdr(macol_info_t *macol, void *p, uint16 *pushlen);
static uint16 wlc_macol_acphy_txctl0_calc(wlc_hw_info_t *wlc_hw, ratespec_t rspec, uint8 preamble);
static uint16 wlc_macol_acphy_txctl1_calc(wlc_hw_info_t *wlc_hw, ratespec_t rspec);
static uint16 wlc_macol_acphy_txctl2_calc(wlc_hw_info_t *wlc_hw, ratespec_t rspec);
static uint wlc_macol_ratespec_nss(ratespec_t rspec);

void
BCMATTACHFN(wlc_macol_attach)(wlc_hw_info_t *wlc_hw, int *err)
{
	osl_t *osh = wlc_hw->osh;		/* pointer to os handle */
	uint32 unit = wlc_hw->unit;
	macol_info_t *macol;
	uint i;

	*err = 0;
	macol = (macol_info_t *)MALLOC(osh, sizeof(macol_info_t));
	if (macol == NULL) {
		WL_ERROR(("wl%d: %s: MALLOC macol_info err\n", unit,  __FUNCTION__));
		*err = 98;
		return;
	}
	bzero((uint8 *)macol, sizeof(macol_info_t));
	macol->hw = wlc_hw;
	macol->osh = osh;
	macol->unit = unit;
	wlc_hw->ol = (void *)macol;
	for (i = 0; i < MAX_FRAME_PENDING; i++)
		macol->frameid[i] = 0xffff;
	macol->frame_pend = 0;
}

void
BCMATTACHFN(wlc_macol_detach)(wlc_hw_info_t *wlc_hw)
{
	osl_t *osh = wlc_hw->osh;		/* pointer to os handle */
	macol_info_t *macol = wlc_hw->ol;

	if (macol == NULL)
		return;

	if (macol)
		MFREE(osh, macol, sizeof(macol_info_t));
	wlc_hw->ol = NULL;
}


int
wlc_macol_chain_set(macol_info_t *macol, uint8 txchain, uint8 rxchain)
{
	WL_ERROR(("%s: Set TX/RX Chain %x  %x\n", __FUNCTION__, txchain, rxchain));
	macol->txchain = txchain;
	macol->rxchain = rxchain;
	return BCME_OK;
}

static void
wlc_macol_cck_plcp_set(int rate_500, uint length, uint8 *plcp)
{
	uint16 usec = 0;
	uint8 le = 0;

	switch (rate_500) {
		case 2:
			usec = length << 3;
			break;
		case 4:
			usec = length << 2;
			break;
		case 11:
			usec = (length << 4)/11;
			if ((length << 4) - (usec * 11) > 0)
				usec++;
			break;
		case 22:
			usec = (length << 3)/11;
			if ((length << 3) - (usec * 11) > 0) {
				usec++;
				if ((usec * 11) - (length << 3) >= 8)
					le = D11B_PLCP_SIGNAL_LE;
			}
			break;

		default:
			WL_ERROR(("%s: unsupported rate %d\n", __FUNCTION__, rate_500));
			ASSERT(!"invalid rate");
			break;
	}
	/* PLCP signal byte */
	plcp[0] = rate_500 * 5; /* r (500kbps) * 5 == r (100kbps) */
	/* PLCP service byte */
	plcp[1] = (uint8)(le | D11B_PLCP_SIGNAL_LOCKED);
	/* PLCP length uint16, little endian */
	plcp[2] = usec & 0xff;
	plcp[3] = (usec >> 8) & 0xff;
	/* PLCP CRC16 */
	plcp[4] = 0;
	plcp[5] = 0;
}

static void
wlc_macol_compute_cck_plcp(ratespec_t rspec, uint length, uint8 *plcp)
{
	int rate;

	ASSERT(IS_CCK(rspec));

	/* extract the 500Kbps rate for rate_info lookup */
	rate = (rspec & RSPEC_RATE_MASK);

	wlc_macol_cck_plcp_set(rate, length, plcp);
}

static void
wlc_macol_compute_ofdm_plcp(ratespec_t rspec, uint32 length, uint8 *plcp)
{
	uint8 rate_signal;
	uint32 tmp = 0;
	int rate;

	/* extract the 500Kbps rate for rate_info lookup */
	rate = (rspec & RSPEC_RATE_MASK);

	/* encode rate per 802.11a-1999 sec 17.3.4.1, with lsb transmitted first */
	rate_signal = rate_info[rate] & RATE_MASK;
	ASSERT(rate_signal != 0);

	bzero(plcp, D11_PHY_HDR_LEN);
	D11A_PHY_HDR_SRATE((ofdm_phy_hdr_t *)plcp, rate_signal);

	tmp = (length & 0xfff) << 5;
	plcp[2] |= (tmp >> 16) & 0xff;
	plcp[1] |= (tmp >> 8) & 0xff;
	plcp[0] |= tmp & 0xff;

	return;
}

static void
wlc_macol_txh_iv_upd(macol_info_t *macol, d11txh_t *txh, uint8 *iv, tx_info_t *macol_txinfo)
{
	d11actxh_t* d11ac_hdr = (d11actxh_t*)txh;
	d11actxh_cache_t	*cache_info;

	cache_info = WLC_TXD_CACHE_INFO_GET(d11ac_hdr, macol->pub->corerev);

	if (macol->secinfo.algo == CRYPTO_ALGO_TKIP) {
		union {
			uint16 u16;
			uint8  u8[2];
		} u;
		uint8 *ivptr = NULL;

		int i = 0, j = 0;
		ivptr = cache_info->TkipPH1Key;

		do {
			u.u16 = htol16(macol_txinfo->key.tkip_tx.phase1[i]);
			ivptr[j++] = u.u8[0];
			ivptr[j++] = u.u8[1];
		} while (++i < TKHASH_P1_KEY_SIZE/2);
		/* write replay counter */
		bcopy(iv, &cache_info->TSCPN, 3);
	}
}

static void
wlc_macol_toe_add_hdr(macol_info_t *macol, void *p, uint16 *pushlen)
{
	d11ac_tso_t tso;
	d11ac_tso_t * tsohdr;
	int len;

	/* No CSO, prepare a passthrough TOE header */
	bzero(&tso, TSO_HEADER_PASSTHROUGH_LENGTH);
	tso.flag[0] |= TOE_F0_PASSTHROUGH;
	len = TSO_HEADER_PASSTHROUGH_LENGTH;


	tsohdr = (d11ac_tso_t*)PKTPUSH(macol->osh, p, len);
	bcopy(&tso, tsohdr, len);

	if (pushlen != NULL)
		*pushlen = (uint16)len;
}

uint16
wlc_macol_d11hdrs(macol_info_t *macol, void *p, ratespec_t rspec, uint16 fifo)
{
	struct dot11_header *h;
	d11actxh_t *txh;
	osl_t *osh;
	uint16 txh_off;
	uint16 phyctl;
	int len, phylen;
	uint16 fc, type, frameid, mch;
	uint16 seq = 0, mcl = 0;
	uint8 IV_offset = 0;
	uint8 *plcp;
	d11actxh_rate_t *rate_blk;
	d11actxh_rate_t *rate_hdr;
	uint frag = 0;
	tx_info_t *macol_txinfo = &macol->txinfo;
	wlc_hw_info_t *hw = macol->hw;
	bool fc_mgmt = FALSE;

	osh = hw->osh;

	h = (struct dot11_header *) PKTDATA(osh, p);

	fc = ltoh16(h->fc);
	type = FC_TYPE(fc);
	if (type == FC_TYPE_MNG)
		fc_mgmt = TRUE;

	len = pkttotlen(osh, p);
	phylen = len + DOT11_FCS_LEN;

	if (!fc_mgmt && macol_txinfo->hwmic) {
		phylen += TKIP_MIC_SIZE;
		mcl |= D11AC_TXC_AMIC;
	}

	/* add Broadcom tx descriptor header */
	txh = (d11actxh_t*)PKTPUSH(osh, p, D11AC_TXH_LEN);
	bzero((char*)txh, D11AC_TXH_LEN);

	rate_blk = WLC_TXD_RATE_INFO_GET(vhtHdr, wlc->pub->corerev);
	rate_hdr = &rate_blk[0];

	plcp = rate_hdr->plcp;

	txh->PktInfo.TSOInfo = 0;

	if (type != FC_TYPE_CTL)
		mcl |= D11AC_TXC_ASEQ;

	if ((fc & FC_KIND_MASK) == FC_BEACON)
		mcl |= D11AC_TXC_IPMQ;

	mcl |= D11AC_TXC_STMSDU;

	if (!ETHER_ISMULTI(&h->a1))
		mcl |= D11AC_TXC_IACK;

	mch = D11AC_TXC_FIX_RATE;

	txh->PktInfo.Chanspec = htol16(hw->chanspec);
	IV_offset = DOT11_A3_HDR_LEN;

	if (!fc_mgmt && macol_txinfo->qos)
		IV_offset += DOT11_QOS_LEN;

	txh->PktInfo.IVOffset = IV_offset;

	mcl |= D11AC_TXC_ASEQ;

	/* FrameLen */
	txh->PktInfo.FrameLen = htol16((uint16)phylen);

	if (type != FC_TYPE_CTL) {
		/* Please check once we decide on sequence number */
		seq = macol_txinfo->seqnum++;
		/* Increment the sequence number only after the last fragment */
		seq = (seq << SEQNUM_SHIFT) | (frag & FRAGNUM_MASK);
		h->seq = htol16(seq);
		txh->PktInfo.Seq = h->seq;
	}

	seq = (macol->counter << SEQNUM_SHIFT) | (frag & FRAGNUM_MASK);
	frameid = ((seq << TXFID_SEQ_SHIFT) & TXFID_SEQ_MASK) | (WLC_TXFID_SET_QUEUE(fifo));
	macol->counter++;

	txh->PktInfo.TxStatus = 0;
	txh->PktInfo.PktCacheLen = 0;
	txh->PktInfo.Tstamp = 0;
	rate_hdr->TxRate = rspec;
	rate_hdr->RtsCtsControl = htol16(D11AC_RTSCTS_LAST_RATE);

	/* Need to send phy controlword0 */
	phyctl = wlc_macol_acphy_txctl0_calc(macol->hw, rspec, WLC_LONG_PREAMBLE);
	rate_hdr->PhyTxControlWord_0 = htol16(phyctl);

	phyctl = wlc_macol_acphy_txctl1_calc(macol->hw, rspec);
	rate_hdr->PhyTxControlWord_1 = htol16(phyctl);
	phyctl = wlc_macol_acphy_txctl2_calc(macol->hw, rspec);
	rate_hdr->PhyTxControlWord_2 = htol16(phyctl);

	txh->PktInfo.MacTxControlLow = htol16(mcl);
	txh->PktInfo.MacTxControlHigh = htol16(mch);
	/* Security */
	if (!fc_mgmt && macol_txinfo->key.algo) {
		if (D11REV_GE(macol->pub->corerev, 64)) {
			txh->u.rev64.CacheInfo.BssIdEncAlg |=
			  macol_txinfo->key.algo_hw << D11AC_ENCRYPT_ALG_SHIFT;
			txh->u.rev64.CacheInfo.KeyIdx = macol_txinfo->key.idx;
		} else {
			txh->u.rev40.CacheInfo.BssIdEncAlg |=
			  macol_txinfo->key.algo_hw << D11AC_ENCRYPT_ALG_SHIFT;
			txh->u.rev40.CacheInfo.KeyIdx = macol_txinfo->key.idx;
		}

		wlc_macol_txh_iv_upd(macol, (d11txh_t *)txh,
			((uint8*)txh + sizeof(d11actxh_t) + IV_offset), macol_txinfo);
	}

	/* PLCP: determine PLCP header and MAC duration, fill d11txh_t */
	if (RSPEC_ISVHT(rspec) || RSPEC_ISHT(rspec))
		ASSERT(0);

	if (IS_OFDM(rspec))
		wlc_macol_compute_ofdm_plcp(rspec, phylen, plcp);
	else
		wlc_macol_compute_cck_plcp(rspec, phylen, plcp);

	/* TxFrameID */
	txh->PktInfo.TxFrameID = htol16(frameid);

	wlc_macol_toe_add_hdr(macol, p, &txh_off);
	wlc_macol_frameid_add(macol, frameid);

	return (frameid);
}

static uint16
wlc_macol_acphy_txctl0_calc(wlc_hw_info_t *wlc_hw, ratespec_t rspec, uint8 preamble_type)
{
	macol_info_t *macol = wlc_hw->ol;
	uint16 phyctl;
	uint16 bw;

	/* PhyTxControlWord_0 */
	phyctl = PHY_TXC_FRAMETYPE(rspec);
	if ((preamble_type == WLC_SHORT_PREAMBLE) ||
	    (preamble_type == WLC_GF_PREAMBLE)) {
		ASSERT((preamble_type == WLC_GF_PREAMBLE) || !RSPEC_ISHT(rspec));
		phyctl |= D11AC_PHY_TXC_SHORT_PREAMBLE;
		WLCNTINCR(wlc_hw->wlc->pub->_cnt->txprshort);
	}

	if (D11REV_GE(wlc->pub->corerev, 64)) {
		/* STBC */
		if (RSPEC_ISSTBC(rspec) & (!RSPEC_ISTXBF(rspec))) {
			/* set b5 */
			phyctl |= D11AC2_PHY_TXC_STBC;
		}
	}

	/* phytxant is properly bit shifted */
	if (!RSPEC_ISTXBF(rspec)) {
		uint16 phytxant = macol->txchain;
		phytxant = (phytxant << D11AC_PHY_TXC_CORE_SHIFT) & D11AC_PHY_TXC_ANT_MASK;
		phyctl |= phytxant;
	} else {
		/*
		 * if beamforming is enabled, ignore the spatial_expension policy
		 * and alway use all currently enabled txcores.
		 * TODO: set coremask correctly for 2SS NDP
		 */
		phyctl |= ((macol->txchain << D11AC_PHY_TXC_CORE_SHIFT)
			   & D11AC_PHY_TXC_ANT_MASK);
		phyctl |= (D11AC_PHY_TXC_BFM);
	}

	/* bit 14/15 - 00/01/10/11 => 20/40/80/160 */
	switch (RSPEC_BW(rspec)) {
		case RSPEC_BW_20MHZ:
			bw = D11AC_PHY_TXC_BW_20MHZ;
			break;
		case RSPEC_BW_40MHZ:
			bw = D11AC_PHY_TXC_BW_40MHZ;
			break;
		case RSPEC_BW_80MHZ:
			bw = D11AC_PHY_TXC_BW_80MHZ;
			break;
		case RSPEC_BW_160MHZ:
			bw = D11AC_PHY_TXC_BW_160MHZ;
			break;
		default:
			ASSERT(0);
			bw = D11AC_PHY_TXC_BW_20MHZ;
			break;
	}
	phyctl |= bw;

	phyctl |= (D11AC_PHY_TXC_NON_SOUNDING);

	return phyctl;
}

static uint16
wlc_macol_acphy_txctl1_calc(wlc_hw_info_t *wlc_hw, ratespec_t rspec)
{
	uint16 phyctl = 0;
	uint8 pwr;
	uint16 spatial_map;
	chanspec_t cspec;
	uint16 sb;

	cspec = wlc_hw->chanspec;
	spatial_map = 0;
	sb = ((cspec & WL_CHANSPEC_CTL_SB_MASK) >> WL_CHANSPEC_CTL_SB_SHIFT);

	/* Primary Subband Location: b 16-18
		LLL ==> 000
		LLU ==> 001
		...
		UUU ==> 111
	*/
	phyctl = sb >> ((RSPEC_BW(rspec) >> RSPEC_BW_SHIFT) - 1);

	/* retrieve power offset on per packet basis */
	pwr = 0;
	phyctl |= (pwr << PHY_TXC1_HTTXPWR_OFFSET_SHIFT);

	if (D11REV_LT(wlc->pub->corerev, 64)) {
		if (WLPROPRIETARY_11N_RATES_ENAB(wlc_hw->wlc->pub)) {
			if (RSPEC_ISHT(rspec) && (rspec & RSPEC_RATE_MASK)
				>= WLC_11N_FIRST_PROP_MCS) {
				phyctl |= D11AC_PHY_TXC_11N_PROP_MCS;
			}
		}
	}

	return phyctl;
}

static uint16
wlc_macol_acphy_txctl2_calc(wlc_hw_info_t *wlc_hw, ratespec_t rspec)
{
	uint16 phyctl = 0;
	uint nss = wlc_macol_ratespec_nss(rspec);
	uint mcs;

	/* mcs+nss: bit 32 through 37 */
	if (RSPEC_ISHT(rspec)) {
		if (WLPROPRIETARY_11N_RATES_ENAB(wlc->pub)) {
			mcs = rspec & D11AC_PHY_TXC_11N_MCS_MASK;
		} else { /* for 11n: B[32:37] for mcs[0:32] */
			mcs = rspec & RSPEC_RATE_MASK;
		}
		phyctl = (uint16)mcs;
	} else if (RSPEC_ISVHT(rspec)) {
		/* for 11ac: B[32:35] for mcs[0-9], B[36:37] for n_ss[1-4]
			(0 = 1ss, 1 = 2ss, 2 = 3ss, 3 = 4ss)
		*/
		mcs = rspec & RSPEC_VHT_MCS_MASK;
		ASSERT(mcs <= 9);
		phyctl = (uint16)mcs;
		ASSERT(nss <= 8);
		phyctl |= ((nss-1) << D11AC_PHY_TXC_11AC_NSS_SHIFT);
	} else {
		phyctl = wlc_rateset_get_legacy_rateidx(rspec);
	}

	if (D11REV_GE(wlc->pub->corerev, 64)) {
		if (WLPROPRIETARY_11N_RATES_ENAB(wlc_hw->wlc->pub)) {
			if (RSPEC_ISHT(rspec) && (rspec & RSPEC_RATE_MASK)
				>= WLC_11N_FIRST_PROP_MCS) {
				phyctl |= D11AC2_PHY_TXC_11N_PROP_MCS;
			}
		}
	} else {
		/* corerev < 64 */
		/* STBC */
		if (RSPEC_ISSTBC(rspec) & (!RSPEC_ISTXBF(rspec))) {
			/* set b38 */
			phyctl |= D11AC_PHY_TXC_STBC;
		}
		/* b39 - b 47 all 0 */
	}

	return phyctl;
}

/* Number of spatial streams, Nss, specified by the ratespec */
static uint
wlc_macol_ratespec_nss(ratespec_t rspec)
{
	int Nss;

	if (RSPEC_ISVHT(rspec)) {

		/* VHT ratespec specifies Nss directly */
		Nss = (rspec & WL_RSPEC_VHT_NSS_MASK) >> WL_RSPEC_VHT_NSS_SHIFT;

	} else if (RSPEC_ISHT(rspec)) {
		uint mcs = rspec & RSPEC_RATE_MASK;

		ASSERT(mcs <= 32 || IS_PROPRIETARY_11N_MCS(mcs));
		Nss = GET_11N_MCS_NSS(mcs); /* HT MCS encodes Nss in MCS index */
	} else {
		/* Legacy rates are all Nss = 1, just add the expansion */
		Nss = 1;
	}

	return Nss;
}

static wlc_bss_info_t *
wlc_macol_BSSadd(wlc_hw_info_t *wlc_hw)
{
	wlc_bss_list_t *scan_results = wlc_hw->scan_pub->scan_results;
	wlc_bss_info_t *BSS;

	if (scan_results->count == MAXBSS)
		return NULL;

	/* allocate a new entry */
	BSS = (wlc_bss_info_t *)MALLOC(wlc_hw->osh, sizeof(wlc_bss_info_t));
	if (BSS) {
		bzero((char*)BSS, sizeof(wlc_bss_info_t));
		BSS->RSSI = WLC_RSSI_MINVAL;
		scan_results->ptrs[scan_results->count++] = BSS;
	}
	return BSS;
}

static wlc_bss_info_t *
wlc_macol_BSSlookup(wlc_hw_info_t *wlc_hw, uchar *bssid, chanspec_t chanspec,
	uchar ssid[], uint ssid_len)
{
	wlc_bss_list_t *scan_results = wlc_hw->scan_pub->scan_results;
	wlc_bss_info_t *BSS;
	uint indx;

	BSS = NULL;
	if (scan_results == NULL)
		return NULL;

	/* search for the BSS descriptor which matches
	 * the bssid AND band(2G/5G) AND SSID
	 */
	for (indx = 0; indx < scan_results->count; indx++) {
		BSS = scan_results->ptrs[indx];
		if (!bcmp(bssid, (char*)&BSS->BSSID, ETHER_ADDR_LEN) &&
		    CHSPEC_BAND(chanspec) == CHSPEC_BAND(BSS->chanspec) &&
		    ssid_len == BSS->SSID_len &&
		    !bcmp(ssid, BSS->SSID, ssid_len))
			return BSS;
	}
	return NULL;
}

static int
wlc_macol_BSSignorelookup(wlc_hw_info_t *wlc_hw, uchar *bssid, chanspec_t chanspec, uchar ssid[],
	uint ssid_len, bool add)
{
	int indx;
	uint16 ssid_sum = 0;
	iscan_ignore_t match;
	uint16 band = CHSPEC_BAND(chanspec);
	wlc_scan_info_t *scan = wlc_hw->scan_pub;

	/* memory savings: compute the sum of the ssid bytes */
	for (indx = 0; indx < ssid_len; ++indx)
		ssid_sum += ssid[indx];

	/* match on bssid/ssid_sum/ssid_len/band */
	bcopy(bssid, &match.bssid, ETHER_ADDR_LEN);
	match.ssid_sum = ssid_sum;
	match.ssid_len = (uint8) ssid_len;

	/* ignore it if it's already in the list */
	for (indx = 0; indx < scan->iscan_ignore_count; indx++) {
		if (!bcmp(&match, &scan->iscan_ignore_list[indx], IGNORE_LIST_MATCH_SZ) &&
		    band == CHSPEC_BAND(scan->iscan_ignore_list[indx].chanspec)) {
			return indx;
		}
	}

	if (add && indx < WLC_ISCAN_IGNORE_MAX) {
		match.chanspec = chanspec;
		bcopy(&match, &scan->iscan_ignore_list[scan->iscan_ignore_count++],
		      sizeof(iscan_ignore_t));
	}

	return -1;
}

static bool
wlc_macol_BSSIgnoreAdd(wlc_hw_info_t *wlc_hw, uchar *bssid, chanspec_t chanspec, uchar ssid[],
	uint ssid_len)
{
	uint indx;
	uint16 ssid_sum = 0;
	iscan_ignore_t new_entry;
	wlc_scan_info_t *scan = wlc_hw->scan_pub;

	if (scan->iscan_ignore_count >= WLC_ISCAN_IGNORE_MAX)
		return FALSE;

	/* memory savings: compute the sum of the ssid bytes */
	for (indx = 0; indx < ssid_len; ++indx) {
		ssid_sum += ssid[indx];
	}
	/* match on bssid/ssid_sum/ssid_len/band */
	bcopy(bssid, &new_entry.bssid, ETHER_ADDR_LEN);
	new_entry.ssid_sum = ssid_sum;
	new_entry.ssid_len = (uint8) ssid_len;

	if (scan->iscan_ignore_count < WLC_ISCAN_IGNORE_MAX) {
		new_entry.chanspec = chanspec;
		bcopy(&new_entry, &scan->iscan_ignore_list[scan->iscan_ignore_count++],
		      sizeof(iscan_ignore_t));
	}
	return TRUE;
}

static bool
wlc_macol_BSSignore(wlc_hw_info_t *wlc_hw, uchar *bssid, chanspec_t chanspec,
	uchar ssid[], uint ssid_len)
{
	int indx;

	indx = wlc_macol_BSSignorelookup(wlc_hw, bssid, chanspec, ssid, ssid_len, FALSE);
	return (indx >= 0);
}

/*
 * Parse a legacy rateset from a TLV section, optionally copy the raw version
 * into a *bss, sanitize and sort it, then optionally copy it into an scb.
 * Return 0 on success, nonzero on error.
 */
static int
wlc_macol_parse_rates(wlc_hw_info_t *wlc_hw, uchar *tlvs, uint tlvs_len, wlc_bss_info_t *bi)
{
	wlc_rateset_t rs;
	int ext_count;
	bcm_tlv_t *ie;

	bzero(&rs, sizeof(wlc_rateset_t));

	/* pick out rateset from tlv section */
	if ((ie = bcm_parse_tlvs(tlvs, tlvs_len, DOT11_MNG_RATES_ID)) == NULL)
		return (-1);

	rs.count = MIN(ie->len, WLC_NUMRATES);
	bcopy(ie->data, (char*)rs.rates, rs.count);

	/* pick out extended rateset from tlv section */
	if ((ie = bcm_parse_tlvs(tlvs, tlvs_len, DOT11_MNG_EXT_RATES_ID)) != NULL) {
		ext_count = MIN(ie->len, WLC_NUMRATES - (int)rs.count);
		bcopy(ie->data, (char*)(rs.rates + rs.count), ext_count);
		rs.count += ext_count;
	}

	/* copy raw set to bss_info */
	if (bi)
		bcopy((char*)&rs, (char*) &bi->rateset, sizeof(wlc_rateset_t));

	return (0);
}

/* This function should be called only when receiving mgmt packets.
 * The channel returned will always be a 20MHz channel.
 */
static uint16
wlc_recv_mgmt_rx_channel_get(wlc_hw_info_t *wlc_hw, wlc_d11rxhdr_t *wrxh)
{
	uint16 channel;

	channel = WLC_RX_CHANNEL(&wrxh->rxhdr);

	/* This tells us that we received a 20MHz packet on a 40MHz channel */
	if (CHSPEC_IS40(wrxh->rxhdr.RxChan)) {
		int is_upper;

		/* Channel reported by WLC_RX_CHANNEL() macro will be the 40MHz center channel.
		 * Since we are assuming the pkt is 20MHz for this fn,
		 * we need to find on which sideband the packet was received
		 * and adjust the channel value to reflect the true RX channel.
		 */
		if (WLCISACPHY(wlc_hw->band)) {
			uint16 subband;
			if (ACREV_GE(wlc_hw->band->phyrev, 32)) {
				subband = (wrxh->rxhdr.PhyRxStatus_1 & PRXS1_ACPHY2_SUBBAND_MASK)
				           >> PRXS1_ACPHY2_SUBBAND_SHIFT;
			} else {
				subband = (wrxh->rxhdr.PhyRxStatus_0 & PRXS0_ACPHY_SUBBAND_MASK)
				           >> PRXS0_ACPHY_SUBBAND_SHIFT;
			}

			is_upper = (subband == PRXS_SUBBAND_20LU);
		} else {
			is_upper = (wrxh->rxhdr.PhyRxStatus_0 & PRXS0_RXANT_UPSUBBAND) != 0;
		}

		if (is_upper) {
			channel = UPPER_20_SB(channel);
		} else {
			channel = LOWER_20_SB(channel);
		}
	} else if (CHSPEC_IS80(wrxh->rxhdr.RxChan)) {
		uint16 subband;

		/* adjust the channel to be the 20MHz center at the low edge of the 80Hz channel */
		channel = channel - CH_40MHZ_APART + CH_10MHZ_APART;

		if (ACREV_GE(wlc_hw->band->phyrev, 32)) {
			subband = (wrxh->rxhdr.PhyRxStatus_1 & PRXS1_ACPHY2_SUBBAND_MASK)
			           >> PRXS1_ACPHY2_SUBBAND_SHIFT;
		} else {
			subband = (wrxh->rxhdr.PhyRxStatus_0 & PRXS0_ACPHY_SUBBAND_MASK)
			           >> PRXS0_ACPHY_SUBBAND_SHIFT;
		}

		switch (subband) {
			case PRXS_SUBBAND_20LL:
			/* channel is correct */
				break;
			case PRXS_SUBBAND_20LU:
				channel += CH_20MHZ_APART;
				break;
			case PRXS_SUBBAND_20UL:
				channel += 2 * CH_20MHZ_APART;
				break;
			case PRXS_SUBBAND_20UU:
				channel += 3 * CH_20MHZ_APART;
				break;
			default:
				break;
		}
	}

	return (channel);
}

/* decode wpa IE to retrieve mcast/unicast ciphers and auth modes */
static int
wlc_macol_parse_wpa_ie(wlc_hw_info_t *wlc_hw, wpa_ie_fixed_t *wpaie, wlc_bss_info_t *bi)
{
	int len = wpaie->length;	/* value length */
	wpa_suite_mcast_t *mcast;
	wpa_suite_ucast_t *ucast;
	wpa_suite_auth_key_mgmt_t *mgmt;
	uint16 count;
	uint i, j;

	/*
	* The TLV tag is generic for all vendor-specific IEs. We need
	* to check OUI as well to make sure this TLV is indeed WPA,
	* and quietly discard the IE if it is not WPA IE.
	*/
	if ((len < WPA_IE_OUITYPE_LEN) ||
	    bcmp((uint8 *)wpaie->oui, WPA_OUI"\01", WPA_IE_OUITYPE_LEN))
		return 0;

	if ((len < WPA_IE_TAG_FIXED_LEN) ||
	    (ltoh16_ua(&wpaie->version) != WPA_VERSION)) {
		WL_ERROR(("wl%d: unsupported WPA version %d\n", wlc_hw->unit,
			ltoh16_ua(&wpaie->version)));
		return BCME_UNSUPPORTED;
	}
	len -= WPA_IE_TAG_FIXED_LEN;

	/* Default WPA parameters */
	bi->wpa.flags = RSN_FLAGS_SUPPORTED;
	bi->wpa.multicast = WPA_CIPHER_TKIP;
	bi->wpa.ucount = 1;
	bi->wpa.unicast[0] = WPA_CIPHER_TKIP;
	bi->wpa.acount = 1;
	bi->wpa.auth[0] = RSN_AKM_UNSPECIFIED;


	/* Check for multicast cipher suite */
	if (len < WPA_SUITE_LEN) {
		WL_INFORM(("wl%d: no multicast cipher suite\n", wlc_hw->unit));
		/* it is ok to not have multicast cipher */
		return 0;
	}
	/* pick up multicast cipher if we know what it is */
	mcast = (wpa_suite_mcast_t *)&wpaie[1];
	len -= WPA_SUITE_LEN;
	if (!bcmp(mcast->oui, WPA_OUI, WPA_OUI_LEN)) {
		if (IS_WPA_CIPHER(mcast->type))
			bi->wpa.multicast = mcast->type;
		else
			WL_INFORM(("wl%d: unsupported WPA multicast cipher %d\n",
				wlc_hw->unit, mcast->type));
	} else
		WL_INFORM(("wl%d: unsupported proprietary multicast cipher OUI "
			"%02X:%02X:%02X\n", wlc_hw->unit,
			mcast->oui[0], mcast->oui[1], mcast->oui[2]));

	/* Check for unicast suite(s) */
	if (len < WPA_IE_SUITE_COUNT_LEN) {
		WL_INFORM(("wl%d: no unicast suite\n", wlc_hw->unit));
		/* it is ok to not have unicast cipher(s) */
		return 0;
	}
	/* walk thru unicast cipher list and pick up what we recognize */
	ucast = (wpa_suite_ucast_t *)&mcast[1];
	count = ltoh16_ua(&ucast->count);
	len -= WPA_IE_SUITE_COUNT_LEN;
	for (i = 0, j = 0;
	     i < count && j < ARRAYSIZE(bi->wpa.unicast) && len >= WPA_SUITE_LEN;
	     i ++, len -= WPA_SUITE_LEN) {
		if (!bcmp(ucast->list[i].oui, WPA_OUI, WPA_OUI_LEN)) {
			if (IS_WPA_CIPHER(ucast->list[i].type))
				bi->wpa.unicast[j++] = ucast->list[i].type;
			else
				WL_INFORM(("wl%d: unsupported WPA unicast cipher %d\n",
					wlc_hw->unit, ucast->list[i].type));
		} else {
			WL_INFORM(("wl%d: unsupported proprietary unicast cipher OUI "
				"%02X:%02X:%02X\n", wlc_hw->unit,
				ucast->list[i].oui[0], ucast->list[i].oui[1],
				ucast->list[i].oui[2]));
		}
	}
	bi->wpa.ucount = (uint8)j;

	/* jump to auth key mgmt suites */
	len -= (count - i) * WPA_SUITE_LEN;

	/* Check for auth key management suite(s) */
	if (len < WPA_IE_SUITE_COUNT_LEN) {
		WL_INFORM(("wl%d: auth key mgmt suite\n", wlc_hw->unit));
		/* it is ok to not have auth key mgmt suites */
		return 0;
	}
	/* walk thru auth management suite list and pick up what we recognize */
	mgmt = (wpa_suite_auth_key_mgmt_t *)&ucast->list[count];
	count = ltoh16_ua(&mgmt->count);
	len -= WPA_IE_SUITE_COUNT_LEN;
	for (i = 0, j = 0;
	     i < count && j < ARRAYSIZE(bi->wpa.auth) && len >= WPA_SUITE_LEN;
	     i ++, len -= WPA_SUITE_LEN) {
		if (!bcmp(mgmt->list[i].oui, WPA_OUI, WPA_OUI_LEN)) {
			if (IS_WPA_AKM(mgmt->list[i].type))
				bi->wpa.auth[j++] = mgmt->list[i].type;
			else
				WL_INFORM(("wl%d: unsupported WPA auth %d\n",
					wlc_hw->unit, mgmt->list[i].type));
		} else
			WL_INFORM(("wl%d: unsupported proprietary auth OUI "
				"%02X:%02X:%02X\n", wlc_hw->unit,
				mgmt->list[i].oui[0], mgmt->list[i].oui[1],
				mgmt->list[i].oui[2]));
	}
	bi->wpa.acount = (uint8)j;

	bi->flags |= WLC_BSS_WPA;

	return 0;
}

/* decode wpa2 IE to retrieve mcast/unicast ciphers and auth modes */
static int
wlc_macol_parse_wpa2_ie(wlc_hw_info_t *wlc_hw, bcm_tlv_t *wpa2ie, wlc_bss_info_t *bi)
{
	int len = wpa2ie->len;		/* value length */
	wpa_suite_mcast_t *mcast;
	wpa_suite_ucast_t *ucast;
	wpa_suite_auth_key_mgmt_t *mgmt;
	uint8 *cap;
	uint16 count;
	uint i, j;

	/* Check min length and version */
	if ((len < WPA2_VERSION_LEN) ||
	    (ltoh16_ua(wpa2ie->data) != WPA2_VERSION)) {
		WL_ERROR(("wl%d: unsupported WPA2 version %d\n", wlc_hw->unit,
			ltoh16_ua(wpa2ie->data)));
		return BCME_UNSUPPORTED;
	}
	len -= WPA2_VERSION_LEN;

	/* Default WPA2 parameters */
	bi->wpa2.flags = RSN_FLAGS_SUPPORTED;
	bi->wpa2.multicast = WPA_CIPHER_AES_CCM;
	bi->wpa2.ucount = 1;
	bi->wpa2.unicast[0] = WPA_CIPHER_AES_CCM;
	bi->wpa2.acount = 1;
	bi->wpa2.auth[0] = RSN_AKM_UNSPECIFIED;

	/* Check for multicast cipher suite */
	if (len < WPA_SUITE_LEN) {
		WL_INFORM(("wl%d: no multicast cipher suite\n", wlc_hw->unit));
		/* it is ok to not have multicast cipher */
		return 0;
	}
	/* pick up multicast cipher if we know what it is */
	mcast = (wpa_suite_mcast_t *)&wpa2ie->data[WPA2_VERSION_LEN];
	len -= WPA_SUITE_LEN;
	if (!bcmp(mcast->oui, WPA2_OUI, DOT11_OUI_LEN)) {
		if (IS_WPA_CIPHER(mcast->type))
			bi->wpa2.multicast = mcast->type;
		else
			WL_INFORM(("wl%d: unsupported WPA2 multicast cipher %d\n",
				wlc_hw->unit, mcast->type));
	} else
		WL_INFORM(("wl%d: unsupported proprietary multicast cipher OUI "
			"%02X:%02X:%02X\n", wlc_hw->unit,
			mcast->oui[0], mcast->oui[1], mcast->oui[2]));

	/* Check for unicast suite(s) */
	if (len < WPA_IE_SUITE_COUNT_LEN) {
		WL_INFORM(("wl%d: no unicast suite\n", wlc_hw->unit));
		/* it is ok to not have unicast cipher(s) */
		return 0;
	}

	/* walk thru unicast cipher list and pick up what we recognize */
	ucast = (wpa_suite_ucast_t *)&mcast[1];
	count = ltoh16_ua(&ucast->count);
	len -= WPA_IE_SUITE_COUNT_LEN;
	for (i = 0, j = 0;
	     i < count && j < ARRAYSIZE(bi->wpa2.unicast) && len >= WPA_SUITE_LEN;
	     i ++, len -= WPA_SUITE_LEN) {
		if (!bcmp(ucast->list[i].oui, WPA2_OUI, DOT11_OUI_LEN)) {
			if (IS_WPA_CIPHER(ucast->list[i].type))
				bi->wpa2.unicast[j++] = ucast->list[i].type;
			else
				WL_INFORM(("wl%d: unsupported WPA2 unicast cipher %d\n",
					wlc_hw->unit, ucast->list[i].type));
		} else {
			WL_INFORM(("wl%d: unsupported proprietary unicast cipher OUI "
				"%02X:%02X:%02X\n", wlc_hw->unit,
				ucast->list[i].oui[0], ucast->list[i].oui[1],
				ucast->list[i].oui[2]));
		}
	}
	bi->wpa2.ucount = (uint8)j;

	/* jump to auth key mgmt suites */
	len -= (count - i) * WPA_SUITE_LEN;

	/* Check for auth key management suite(s) */
	if (len < WPA_IE_SUITE_COUNT_LEN) {
		WL_INFORM(("wl%d: auth key mgmt suite\n", wlc_hw->unit));
		/* it is ok to not have auth key mgmt suites */
		return 0;
	}
	/* walk thru auth management suite list and pick up what we recognize */
	mgmt = (wpa_suite_auth_key_mgmt_t *)&ucast->list[count];
	count = ltoh16_ua(&mgmt->count);
	len -= WPA_IE_SUITE_COUNT_LEN;
	for (i = 0, j = 0;
	     i < count && j < ARRAYSIZE(bi->wpa2.auth) && len >= WPA_SUITE_LEN;
	     i ++, len -= WPA_SUITE_LEN) {
		if (!bcmp(mgmt->list[i].oui, WPA2_OUI, DOT11_OUI_LEN)) {
			if (IS_WPA2_AKM(mgmt->list[i].type))
				bi->wpa2.auth[j++] = mgmt->list[i].type;
			else
				WL_INFORM(("wl%d: unsupported WPA2 auth %d\n",
					wlc_hw->unit, mgmt->list[i].type));
		} else
			WL_INFORM(("wl%d: unsupported proprietary auth OUI "
				"%02X:%02X:%02X\n", wlc_hw->unit,
				mgmt->list[i].oui[0], mgmt->list[i].oui[1],
				mgmt->list[i].oui[2]));
	}
	bi->wpa2.acount = (uint8)j;

	bi->flags |= WLC_BSS_WPA2;

	/* jump to RSN Cap */
	len -= (count - i) * WPA_SUITE_LEN;

	if (len < RSN_CAP_LEN) {
		WL_INFORM(("wl%d: no rsn cap\n", wlc_hw->unit));
		/* it is ok to not have RSN Cap */
		return 0;
	}

	/* parse RSN capabilities */
	cap = (uint8 *)&mgmt->list[count];
	if (cap[0] & WPA_CAP_WPA2_PREAUTH)
		bi->wpa2.flags |= RSN_FLAGS_PREAUTH;
	if (cap[1] & WPA_CAP_PEER_KEY_ENABLE)
		bi->wpa2.flags |= RSN_FLAGS_PEER_KEY_ENAB;

	bi->wpa2.cap[0] = cap[0];

	len -= RSN_CAP_LEN;
	if (len) {
		WL_INFORM(("wl%d: set RSN_FLAGS_PMKID_COUNT_PRESENT.\n", wlc_hw->unit));
		bi->wpa2.flags |= RSN_FLAGS_PMKID_COUNT_PRESENT;
	}
	return 0;
}

static int
wlc_macol_recv_parse_bcn_prb(wlc_hw_info_t *wlc_hw, wlc_d11rxhdr_t *wrxh, struct ether_addr *bssid,
	bool beacon, void *body, uint body_len, wlc_bss_info_t *bi)
{
	struct dot11_bcn_prb *fixed = (struct dot11_bcn_prb *) body;
	uint16 cap;
	uint8 chan = 0;
	uint8 rx_chan;
	uint8 *tlvs, *parse;
	uint tlvs_len, parse_len;
	bcm_tlv_t *ie;
	wpa_ie_fixed_t *wpaie;

	if (body_len < sizeof(struct dot11_bcn_prb)) {
		WL_INFORM(("wl%d: %s: invalid frame length\n", wlc_hw->unit, __FUNCTION__));
		return (-1);
	}

	/* check validity of beacon before update */
	tlvs_len = body_len - DOT11_BCN_PRB_LEN;
	tlvs = (uint8 *)body + DOT11_BCN_PRB_LEN;

	/* grab the channel from the rxheader, need conversion for b/g phy */
	rx_chan = (uint8)wlc_recv_mgmt_rx_channel_get(wlc_hw, wrxh);

	/* check the freq. */
	if ((ie = bcm_parse_tlvs(tlvs, tlvs_len, DOT11_MNG_DS_PARMS_ID)) != NULL) {
		chan = ie->data[0];
	} else {
		/* 802.11a beacons don't have a DS tlv in them.
		 * Figure it out from the current mac channel.
		 */
		chan = rx_chan;
	}

	/* Fill in bss with new info */
	bzero((char*)bi, sizeof(wlc_bss_info_t));

	bcopy((char *)bssid, (char *)&bi->BSSID, ETHER_ADDR_LEN);

	cap = ltoh16_ua(&fixed->capability);
	if ((cap & DOT11_CAP_ESS) && !(cap & DOT11_CAP_IBSS))
		bi->infra = 1;
	bi->capability = cap;

	bi->flags |= beacon ? WLC_BSS_BEACON : 0;

	/* extract RSSI from rxh */
	bi->RSSI = wrxh->rssi;

	/* extract SNR from rxh */
	bi->SNR = WLC_SNR_INVALID;

	/* flag if this is an RSSI reading on same channel as the bcn/prb was transmitted */
	if (rx_chan == chan)
		bi->flags |= WLC_BSS_RSSI_ON_CHANNEL;

	bi->beacon_period = ltoh16_ua(&fixed->beacon_interval);

	/* process tagged fields */
	/* SSID */
	if ((ie = bcm_parse_tlvs(tlvs, tlvs_len, DOT11_MNG_SSID_ID)) != NULL &&
	    ie->len <= DOT11_MAX_SSID_LEN) {
		bi->SSID_len = ie->len;
		bcopy(ie->data, bi->SSID, bi->SSID_len);
	} else {
		WL_INFORM(("Missing SSID info in beacon\n"));
		bi->SSID_len = 0;
	}

	/* parse rateset and pull out raw rateset */
	if (wlc_macol_parse_rates(wlc_hw, tlvs, tlvs_len, bi) != 0) {
		WL_INFORM(("Missing or Invalid rate info in beacon\n"));
		return (-1);
	}

	/* Check for a legacy 54G bcn/proberesp by looking for more than 8 rates
	 * in the Supported Rates elt
	 */
	ie = bcm_parse_tlvs(tlvs, tlvs_len, DOT11_MNG_RATES_ID);
	if (ie && ie->len > 8) {
		bi->flags |= WLC_BSS_54G;
	}

	/* DS parameters */
	bi->chanspec = CH20MHZ_CHSPEC(chan);

	/* WPA2 parameters */
	if ((ie = bcm_parse_tlvs(tlvs, tlvs_len, DOT11_MNG_RSN_ID)) != NULL) {
		WL_INFORM(("%s: wpa2 IE found\n", __FUNCTION__));
		wlc_macol_parse_wpa2_ie(wlc_hw, ie, bi);
	}

	/* WPA parameters */
	parse = tlvs;
	parse_len = tlvs_len;
	if ((wpaie = bcm_find_wpaie(parse, parse_len)) != NULL) {
		WL_INFORM(("%s: wpa IE found\n", __FUNCTION__));
		wlc_macol_parse_wpa_ie(wlc_hw, wpaie, bi);
	}

	return 0;
}

static void
wlc_macol_recv_scan_parse(wlc_hw_info_t *wlc_hw, wlc_d11rxhdr_t *wrxh, uint8 *plcp,
	struct dot11_management_header *hdr, uint8 *body, int body_len)
{
	struct dot11_bcn_prb *bcn = (struct dot11_bcn_prb *)body;
	struct dot11_bcn_prb *bcn_prb = NULL;
	uint16 cap = ltoh16(bcn->capability);
	wlc_bss_info_t bi;
	wlc_bss_info_t *BSS;
	bcm_tlv_t *ssid_ie;
	bool beacon, filter;
	chanspec_t chanspec = 0;
	wlc_scan_info_t *scan = wlc_hw->scan_pub;


	beacon = ((ltoh16(hdr->fc) & FC_KIND_MASK) == FC_BEACON);
	filter = beacon && !(scan->state & SCAN_STATE_PASSIVE) &&
		!wlc_scan_quiet_chanspec(scan->scan_priv, WLC_RX_CHANNEL(&wrxh->rxhdr));

	/* find and validate the SSID IE */
	ssid_ie = bcm_parse_tlvs(body + DOT11_BCN_PRB_LEN, body_len - DOT11_BCN_PRB_LEN,
		DOT11_MNG_SSID_ID);

	/*
	 * check for probe responses/beacons:
	 * - must have the correct Infra mode
	 * - must have the SSID(s) we were looking for,
	 *   unless we were doing a broadcast SSID request (ssid len == 0)
	 * - must have matching BSSID (if unicast was specified)
	 */
	if ((scan->wlc_scan_cmn->bss_type == DOT11_BSSTYPE_ANY ||
	     (scan->wlc_scan_cmn->bss_type == DOT11_BSSTYPE_INFRASTRUCTURE &&
		(cap & DOT11_CAP_ESS)) ||
		(scan->wlc_scan_cmn->bss_type == DOT11_BSSTYPE_INDEPENDENT &&
		(cap & DOT11_CAP_IBSS))) && wlc_scan_ssid_match(scan, ssid_ie, filter) &&
	    (ETHER_ISMULTI(&scan->bssid) || bcmp(&scan->bssid, &hdr->bssid, ETHER_ADDR_LEN) == 0)) {
		char eabuf[ETHER_ADDR_STR_LEN];
		BCM_REFERENCE(eabuf);

		if (wlc_macol_recv_parse_bcn_prb(wlc_hw, wrxh, &hdr->bssid, beacon,
			body, body_len, &bi)) {
			/* parse fail, discard this new bi */
			WL_INFORM(("wl%d: %s: tossing bcn/prb resp "
			           "collected on channel %d wlc_parse_bcn_prb failed\n",
			           wlc_hw->unit, __FUNCTION__, WLC_RX_CHANNEL(&wrxh->rxhdr)));
			return;
		}

		WL_INFORM(("wl%d: BSS %s [%s] on channel %d (0x%x)\n",
		         wlc_hw->unit, bcm_ether_ntoa(&hdr->bssid, eabuf),
		         bi.SSID, CHSPEC_CHANNEL(bi.chanspec), bi.chanspec));
		/* check bssid, ssid AND band matching */
		BSS = wlc_macol_BSSlookup(wlc_hw, (uchar *)&hdr->bssid, bi.chanspec,
			bi.SSID, bi.SSID_len);

		/* do not allow beacon data to update the data recd from a probe response */
		if (BSS && beacon && !(BSS->flags & WLC_BSS_BEACON)) {
			WL_INFORM(("wl%d: %s: tossing bcn for BSS\n", wlc_hw->unit, __FUNCTION__));
			return;
		}

		if (BSS) {
			if ((BSS->flags & WLC_BSS_RSSI_ON_CHANNEL) ==
			    (bi.flags & WLC_BSS_RSSI_ON_CHANNEL)) {
				/* preserve max RSSI if the measurements are
				 * both on-channel or both off-channel
				 */
				bi.RSSI = MAX(BSS->RSSI, bi.RSSI);
			} else if ((BSS->flags & WLC_BSS_RSSI_ON_CHANNEL) &&
			           (bi.flags & WLC_BSS_RSSI_ON_CHANNEL) == 0) {
				/* preserve the on-channel rssi measurement
				 * if the new measurement is off channel
				 */
				bi.RSSI = BSS->RSSI;
				bi.flags |= WLC_BSS_RSSI_ON_CHANNEL;
			}
		} else if (scan->scanresults_minrssi &&
		         bi.RSSI < (int16) (scan->scanresults_minrssi)) {
			/* filter this one out if the user specified a minimum signal strength */
			WL_INFORM(("wl%d: %s: tossing bcn/prb resp "
				   "for BSS on channel spec %d since target rssi was %d\n",
				   wlc_hw->unit, __FUNCTION__, bi.chanspec, bi.RSSI));
			return;
		}

		/* Iscan: add new BSS only if not previously reported (update existing is ok). */
		if (!BSS && SCAN_ISCAN_IN_PROGRESS(wlc_hw)) {
			/* use actual bcn_prb rx channel for comparison chanspec */
			chanspec = (CHSPEC_CHANNEL(wrxh->rxhdr.RxChan) << WL_CHANSPEC_CHAN_SHIFT);

			/* OR in the other bits */
			chanspec |= (chanspec_t)(bi.chanspec & ~(WL_CHANSPEC_CHAN_MASK));
			/* ignore if it's in the ignore list; add otherwise */
			if (wlc_macol_BSSignore(wlc_hw, (uchar *)&hdr->bssid, chanspec, bi.SSID,
			                  bi.SSID_len)) {
				return;
			}
		}

		/* make sure we have enough space before proceeding */
		if (scan->state & SCAN_STATE_SAVE_PRB) {
			bcn_prb = (struct dot11_bcn_prb *) MALLOC(wlc_hw->osh, body_len);
			if (!bcn_prb) {
				WL_ERROR(("wl%d: %s: out of memory %d bytes\n",
					wlc_hw->unit, __FUNCTION__, body_len));
				return;
			}
		}

		/* add it if it was not already in the list */
		if (!BSS) {
			BSS = wlc_macol_BSSadd(wlc_hw);
			if (BSS && SCAN_ISCAN_IN_PROGRESS(wlc_hw)) {
				wlc_macol_BSSIgnoreAdd(wlc_hw, (uchar *)&hdr->bssid, chanspec,
					bi.SSID, bi.SSID_len);
			}
		}

		if (BSS) {
			/* free the prb pointer, prevent memory leakage */
			if (BSS->bcn_prb) {
				MFREE(wlc_hw->osh, BSS->bcn_prb, BSS->bcn_prb_len);
				BSS->bcn_prb = NULL;
				BSS->bcn_prb_len = 0;
			}
			/* update the entry regardless existing or new */
			bcopy((char*)&bi, (char*)BSS, sizeof(wlc_bss_info_t));
#ifdef WLSCANCACHE
			BSS->timestamp = OSL_SYSUPTIME();
#endif
			if (scan->state & SCAN_STATE_SAVE_PRB) {
				/* scan completion responsible for freeing frame */
				ASSERT(!BSS->bcn_prb);
				ASSERT(bcn_prb != NULL);

				/* save raw probe response frame */
				BSS->bcn_prb = bcn_prb;
				bcopy((char*)body, (char*)BSS->bcn_prb, body_len);
				BSS->bcn_prb_len = (uint16) body_len;
			}
#ifdef WLPFN
			/* if this is a pfn scan */
			if (WLPFN_ENAB(wlc->pub) && (wl_pfn_scan_in_progress(wlc->pfn)) {
				wlc_bss_list_t *scan_results = wlc_hw->scan_pub->scan_results;
				/* process network */
				wl_pfn_process_scan_result(wlc->pfn, BSS);
				if (!(scan->state & SCAN_STATE_SAVE_PRB))
					wlc_bss_list_free(wlc, scan_results);
			}
#endif	/* WLPFN */
		} else {
			if (bcn_prb)
				MFREE(wlc_hw->osh, bcn_prb, body_len);
			/* no room in list or out of memory */
			WL_INFORM(("%s: can't add BSS: "
			           "terminating scan in process\n", __FUNCTION__));
			wlc_scan_terminate(scan, WLC_E_STATUS_PARTIAL);
		}
	}
}

bool
wlc_macol_rxpkt_consumed(wlc_hw_info_t *wlc_hw, void *p, wlc_d11rxhdr_t *wrxh)
{
	struct dot11_management_header *hdr;
	uint8 *plcp, *body;
	uint16 fc, ft;
	uint len, body_len;
	uint32 org_wl_msg_level = wl_msg_level;

	if (!SCAN_IN_PROGRESS(wlc_hw->scan_pub))
		return FALSE;

	plcp = PKTDATA(wlc_hw->osh, p);
	len = PKTLEN(wlc_hw->osh, p);
	hdr = (struct dot11_management_header *)(plcp + D11_PHY_HDR_LEN);
	body = (uint8 *)(hdr + 1);
	body_len = len - DOT11_MGMT_HDR_LEN - DOT11_FCS_LEN;

	if (len < D11_PHY_HDR_LEN + sizeof(hdr->fc)) {
		ASSERT(0);
		return FALSE;
	}
	ASSERT(WLC_RX_CHANNEL(&wrxh->rxhdr) != 0);
	fc = ltoh16(hdr->fc);
	ft = FC_TYPE(fc);
	if (ft != FC_TYPE_MNG) {
		ASSERT(0);
		return FALSE;
	}

	switch (fc & FC_KIND_MASK) {
		case FC_BEACON:
		case FC_PROBE_RESP:
			if (0) wl_msg_level |= WL_INFORM_VAL; /* DEBUG CODE */
			wlc_macol_recv_scan_parse(wlc_hw, wrxh, plcp, hdr, body, body_len);
			wl_msg_level = org_wl_msg_level;
			break;
		default:
			WL_INFORM(("%s: Discard Non Beacon/Probe Frames\n", __FUNCTION__));
			return FALSE;
	}
	return TRUE;
}

void
wlc_macol_watchdog(void *hdl)
{
	macol_info_t *macol = (macol_info_t *)hdl;
	if (macol->frame_pend != 0)
		printf("frame pending %d\n", macol->frame_pend);
}

void
wlc_macol_frameid_add(macol_info_t *macol, uint16 frameid)
{
	uint i;

	if (macol->frame_pend >= MAX_FRAME_PENDING)
		ASSERT(0 && "macol->frame_pend >= MAX_FRAME_PENDING");

	for (i = 0; i < 10; i++) {
		if (macol->frameid[i] == 0xffff) {
			macol->frameid[i] = frameid;
			macol->frame_pend++;
			return;
		}
	}
}

bool
wlc_macol_frameid_match(wlc_hw_info_t *wlc_hw, uint16 frameid)
{
	macol_info_t *macol = (macol_info_t *)wlc_hw->ol;
	uint i;

	for (i = 0; i < MAX_FRAME_PENDING; i++) {
		if (macol->frameid[i] == frameid) {
			macol->frameid[i] = 0xffff;
			macol->frame_pend--;
			return TRUE;
		}
	}
	return FALSE;
}

void
wlc_macol_dequeue_dma_pkt(wlc_hw_info_t *wlc_hw, tx_status_t *txs)
{
	uint queue;
	void *p;
	uint32 fifo = TX_AC_BE_FIFO;
#ifdef BCM_OL_DEV
	fifo = TX_ATIM_FIFO;	/* for low mac, only fifo 5 allowed */
#endif


	if (0) macol_print_txstatus(txs); /* DEBUG CODE */
	queue = WLC_TXFID_GET_QUEUE(txs->frameid);
	if (0 && queue != fifo) {
		ASSERT(0 && "FIFO MISMATCH");
		queue = fifo;
	}
	p = wlc_bmac_dma_getnexttxp(wlc_hw->wlc, queue, HNDDMA_RANGE_TRANSMITTED);
	ASSERT(p);
	if (p) {
		PKTFREE(wlc_hw->osh, p, FALSE);
		TXPKTPENDDEC(wlc_hw->wlc, queue, 1);
	}
}

void
macol_print_txstatus(tx_status_t* txs)
{
	uint16 s = txs->status.raw_bits;
	uint16 status_bits = txs->status.raw_bits;

	static const char *supr_reason[] = {
		"None", "PMQ Entry", "Flush request",
		"Previous frag failure", "Channel mismatch",
		"Lifetime Expiry", "Underflow", "AB NACK or TX SUPR"
	};

	printf("\ntxpkt (MPDU) Complete\n");

	printf("FrameID: 0x%04x   ", txs->frameid);
	printf("Seq: 0x%04x   ", txs->sequence);
	printf("TxStatus: 0x%04x", s);
	printf("\n");

	printf("ACK %d IM %d PM %d Suppr %d (%s)",
	       txs->status.was_acked, txs->status.is_intermediate,
	       txs->status.pm_indicated, txs->status.suppr_ind,
	       (txs->status.suppr_ind < ARRAYSIZE(supr_reason) ?
		supr_reason[txs->status.suppr_ind] : "Unkn supr"));

	printf("PHYTxErr:   0x%04x ", txs->phyerr);
	printf("\n");

	printf("Raw\n[0]	%d Valid\n", ((status_bits & TX_STATUS_VALID) != 0));
	printf("[2]    %d IM\n", ((status_bits & TX_STATUS40_INTERMEDIATE) != 0));
	printf("[3]    %d PM\n", ((status_bits & TX_STATUS40_PMINDCTD) != 0));
	printf("[7-4]  %d Suppr\n",
	       ((status_bits & TX_STATUS40_SUPR) >> TX_STATUS40_SUPR_SHIFT));
	printf("[14:8] %d Ncons\n", ((status_bits & TX_STATUS40_NCONS) >> TX_STATUS40_NCONS_SHIFT));
	printf("[15]   %d Acked\n", (status_bits & TX_STATUS40_ACK_RCV) != 0);
}

static void
wlc_macol_print_byte(const char* desc, uint8 val)
{
	printf("%s: %02x\n", desc, val);
}

static void
wlc_macol_print_word(const char* desc, uint16 val)
{
	printf("%s: %04x\n", desc, val);
}

static void
wlc_macol_print_per_pkt_desc_ac(d11actxh_t* acHdrPtr)
{
	d11actxh_pkt_t *pi = &acHdrPtr->PktInfo;
	uint16 mcl, mch;

	printf("TxD Pkt Info:\n");
	/* per packet info */
	mcl = ltoh16(pi->MacTxControlLow);
	ASSERT(mcl != 0 || pi->MacTxControlLow == 0);
	mch = ltoh16(pi->MacTxControlHigh);

	printf(" MacTxControlLow 0x%04X MacTxControlHigh 0x%04X Chspec 0x%04X\n",
	       mcl, mch, ltoh16(pi->Chanspec));
	printf(" TxDShrt %u UpdC %u CacheID %u AMPDU %u ImmdAck %u LFRM %u IgnPMQ %u\n",
	       (mcl & D11AC_TXC_HDR_FMT_SHORT) != 0,
	       (mcl & D11AC_TXC_UPD_CACHE) != 0,
	       (mcl & D11AC_TXC_CACHE_IDX_MASK) >> D11AC_TXC_CACHE_IDX_SHIFT,
	       (mcl & D11AC_TXC_AMPDU) != 0,
	       (mcl & D11AC_TXC_IACK) != 0,
	       (mcl & D11AC_TXC_LFRM) != 0,
	       (mcl & D11AC_TXC_HDR_FMT_SHORT) != 0);
	printf(" MBurst %u ASeq %u Aging %u AMIC %u STMSDU %u RIFS %u ~FCS %u FixRate %u\n",
	       (mcl & D11AC_TXC_MBURST) != 0,
	       (mcl & D11AC_TXC_ASEQ) != 0,
	       (mcl & D11AC_TXC_AGING) != 0,
	       (mcl & D11AC_TXC_AMIC) != 0,
	       (mcl & D11AC_TXC_STMSDU) != 0,
	       (mcl & D11AC_TXC_URIFS) != 0,
	       (mch & D11AC_TXC_DISFCS) != 0,
	       (mch & D11AC_TXC_FIX_RATE) != 0);

	printf(" IVOffset %u PktCacheLen %u FrameLen %u\n",
	       pi->IVOffset, pi->PktCacheLen, ltoh16(pi->FrameLen));
	printf(" Seq 0x%04X TxFrameID 0x%04X Tstamp 0x%04X TxStatus 0x%04X\n",
	       ltoh16(pi->Seq), ltoh16(pi->TxFrameID),
	       ltoh16(pi->Tstamp), ltoh16(pi->TxStatus));
}

static void
wlc_macol_print_per_rate_info(d11actxh_t* acHdrPtr, uint8 rateIdx)
{
	d11actxh_rate_t *rb;
	d11actxh_rate_t *ri;

	rb = WLC_TXD_RATE_INFO_GET(acHdrPtr, wlc->pub->corerev);
	ri = &rb[rateIdx];

	printf("TxD Rate[%d]:\n", rateIdx);

	printf(" PhyTxCtl: 0x%04X 0x%04X 0x%04X\n",
	       ltoh16(ri->PhyTxControlWord_0),
	       ltoh16(ri->PhyTxControlWord_1),
	       ltoh16(ri->PhyTxControlWord_2));
	printf(" plcp: %02X %02X %02X %02X %02X %02X\n",
	       ri->plcp[0], ri->plcp[1], ri->plcp[2],
	       ri->plcp[3], ri->plcp[4], ri->plcp[5]);
	printf(" FbwInfo %u TxRate 0x%04X RtsCtsControl 0x%04X\n",
	       ri->FbwInfo,
	       ltoh16(ri->TxRate), ltoh16(ri->RtsCtsControl));
	if (ri->Bfm0 != 0) {
		wlc_macol_print_word(" Bfm0", ltoh16(ri->Bfm0));
	}
}

static void
wlc_macol_print_per_pkt_cache_ac(wlc_info_t *wlc, d11actxh_t* acHdrPtr)
{
	d11actxh_cache_t	*cache_info;

	cache_info = WLC_TXD_CACHE_INFO_GET(acHdrPtr, wlc->pub->corerev);

	printf("TxD Pkt Cache Info:\n");
	wlc_macol_print_byte(" BssIdEncAlg", cache_info->BssIdEncAlg);
	wlc_macol_print_byte(" KeyIdx", cache_info->KeyIdx);
	wlc_macol_print_byte(" PrimeMpduMax", cache_info->PrimeMpduMax);
	wlc_macol_print_byte(" FallbackMpduMax", cache_info->FallbackMpduMax);
	wlc_macol_print_word(" AmpduDur", ltoh16(cache_info->AmpduDur));
	wlc_macol_print_byte(" BAWin", cache_info->BAWin);
	wlc_macol_print_byte(" MaxAggLen", cache_info->MaxAggLen);
	prhex(" TkipPH1Key", (uchar *)cache_info->TkipPH1Key, 10);
	prhex(" TSCPN", (uchar *)cache_info->TSCPN, 6);
}

static void
wlc_macol_print_txdesc_ac(wlc_info_t *wlc, void* hdrsBegin)
{
	/* tso header */
	d11actxh_t* acHdrPtr;
	uint len, rate_count;
	uint8 rateNum;

	/* d11ac headers */
	acHdrPtr = (d11actxh_t*)(hdrsBegin);

	if (acHdrPtr->PktInfo.MacTxControlLow & htol16(D11AC_TXC_HDR_FMT_SHORT)) {
		len = D11AC_TXH_SHORT_LEN;
	} else {
		len = D11AC_TXH_LEN;
	}

	printf("tx hdr len=%d dump follows:\n", len);

	prhex("Raw TxACDesc", (uchar *)hdrsBegin, len);
	wlc_macol_print_per_pkt_desc_ac(acHdrPtr);

	/* Short TxD only has the per-pkt info, long TxD has
	 * per-rate and cache sections. Dump the addtional
	 * sections if it is a long header.
	 */
	if (len == D11AC_TXH_LEN) {
		if (acHdrPtr->PktInfo.MacTxControlHigh & htol16(D11AC_TXC_FIX_RATE)) {
			rate_count = 1;
		} else {
			rate_count = D11AC_TXH_NUM_RATES;
		}
		for (rateNum = 0; rateNum < rate_count; rateNum++) {
			wlc_macol_print_per_rate_info(acHdrPtr, rateNum);
		}
		wlc_macol_print_per_pkt_cache_ac(wlc, acHdrPtr);
	}
}

void
wlc_macol_print_txdesc(wlc_info_t *wlc, wlc_txd_t *txd)
{
	wlc_macol_print_txdesc_ac(wlc, txd);
}

void
wlc_macol_intr_enable(wlc_hw_info_t *wlc_hw, uint32 bit)
{
	if (wlc_hw->defmacintmask & bit)
		return;

	wlc_hw->defmacintmask |= bit;
	if (wlc_hw->clk)
		wl_intrson(wlc_hw->wlc->wl);
}


void
wlc_macol_intr_disable(wlc_hw_info_t *wlc_hw, uint32 bit)
{
	if ((wlc_hw->defmacintmask & bit) == 0)
		return;

	wlc_hw->defmacintmask &= ~bit;
	if (wlc_hw->clk)
		wl_intrson(wlc_hw->wlc->wl);
}

void
wlc_macol_set_shmem_coremask(wlc_hw_info_t *wlc_hw)
{
	uint16 base = M_COREMASK_BLK;
	uint idx, offset;
	uint8 map;

	for (offset = 0, idx = 0; offset < MAX_COREMASK_BLK; offset++, idx++) {
		map = (wlc_hw->txcore[idx][1] & TXCOREMASK);
		wlc_bmac_write_shm(wlc_hw, (base+offset)*2, map);
	}
}

int
wlc_macol_pso_shmem_upd(wlc_hw_info_t *wlc_hw)
{
	uint16 pso_blk, pso_flags;

	pso_blk = wlc_bmac_read_shm(wlc_hw, M_ARM_PSO_BLK_PTR) * 2;
	if (pso_blk == NULL)
		return -1;

	pso_flags = wlc_bmac_read_shm(wlc_hw, pso_blk);
	if ((pso_flags & 0x8000) == 0) {
		pso_flags |= 0x8000;
		wlc_bmac_write_shm(wlc_hw, pso_blk, pso_flags);
	}
	WL_INFORM(("%s: pso_blk_ptr %x  pso_flag %x\n", __FUNCTION__, pso_blk, pso_flags));
	return 0;
}
