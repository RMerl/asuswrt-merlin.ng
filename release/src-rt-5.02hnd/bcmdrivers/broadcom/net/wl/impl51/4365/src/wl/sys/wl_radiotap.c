/*
 * RadioTap utility routines for WL
 * This file housing the functions use by
 * wl driver.
 *
 * Copyright (C) 2017, Broadcom. All Rights Reserved.
 * 
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
 * SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION
 * OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
 * CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 * $Id: wl_radiotap.c 598312 2015-11-09 10:01:36Z $
 */


#include <wlc_cfg.h>

#ifndef WL_MONITOR
#error "WL_MONITOR is not defined"
#endif	/* WL_MONITOR */

#include <typedefs.h>
#include <wl_dbg.h>
#include <osl.h>
#include <bcmutils.h>
#include <siutils.h>
#include <bcmendian.h>
#include <proto/802.11.h>
#include <wlioctl.h>
#include <d11.h>
#include <wlc_rate.h>
#include <wlc_pub.h>
#include <bcmwifi_channels.h>
#ifdef WL_RADIOTAP
#include <wlc_ethereal.h>
#endif /* WL_RADIOTAP */
#include <wl_radiotap.h>

const struct rtap_field rtap_parse_info[] = {
	{8, 8}, /* 0:  IEEE80211_RADIOTAP_TSFT */
	{1, 1}, /* 1:  IEEE80211_RADIOTAP_FLAGS */
	{1, 1}, /* 2:  IEEE80211_RADIOTAP_RATE */
	{4, 2}, /* 3:  IEEE80211_RADIOTAP_CHANNEL */
	{2, 2}, /* 4:  IEEE80211_RADIOTAP_FHSS */
	{1, 1}, /* 5:  IEEE80211_RADIOTAP_DBM_ANTSIGNAL */
	{1, 1}, /* 6:  IEEE80211_RADIOTAP_DBM_ANTNOISE */
	{2, 2}, /* 7:  IEEE80211_RADIOTAP_LOCK_QUALITY */
	{2, 2}, /* 8:  IEEE80211_RADIOTAP_TX_ATTENUATION */
	{2, 2}, /* 9:  IEEE80211_RADIOTAP_DB_TX_ATTENUATION */
	{1, 1}, /* 10: IEEE80211_RADIOTAP_DBM_TX_POWER */
	{1, 1}, /* 11: IEEE80211_RADIOTAP_ANTENNA */
	{1, 1}, /* 12: IEEE80211_RADIOTAP_DB_ANTSIGNAL */
	{1, 1}, /* 13: IEEE80211_RADIOTAP_DB_ANTNOISE */
	{0, 0}, /* 14: netbsd */
	{2, 2}, /* 15: IEEE80211_RADIOTAP_TXFLAGS */
	{0, 0}, /* 16: missing */
	{1, 1}, /* 17: IEEE80211_RADIOTAP_RETRIES */
	{8, 4}, /* 18: IEEE80211_RADIOTAP_XCHANNEL */
	{3, 1}, /* 19: IEEE80211_RADIOTAP_MCS */
	{8, 4}, /* 20: IEEE80211_RADIOTAP_AMPDU_STATUS */
	{12, 2}, /* 21: IEEE80211_RADIOTAP_VHT */
	{0, 0}, /* 22: */
	{0, 0}, /* 23: */
	{0, 0}, /* 24: */
	{0, 0}, /* 25: */
	{0, 0}, /* 26: */
	{0, 0}, /* 27: */
	{0, 0}, /* 28: */
	{0, 0}, /* 29: IEEE80211_RADIOTAP_RADIOTAP_NAMESPACE */
	{6, 2}, /* 30: IEEE80211_RADIOTAP_VENDOR_NAMESPACE */
	{0, 0}  /* 31: IEEE80211_RADIOTAP_EXT */
};

static const uint8 brcm_oui[] =  {0x00, 0x10, 0x18};

static void
wl_rtapParseReset(radiotap_parse_t *rtap)
{
	rtap->idx = 0;		/* reset parse index */
	rtap->offset = 0;	/* reset current field pointer */
}

static void*
wl_rtapParseFindField(radiotap_parse_t *rtap, uint search_idx)
{
	uint idx;	/* first bit index to parse */
	uint32 it_present;	/* presence bitmap */
	uint offset, field_offset;
	uint align, len;
	void *ptr = NULL;

	if (search_idx > IEEE80211_RADIOTAP_EXT)
		return ptr;

	if (search_idx < rtap->idx)
		wl_rtapParseReset(rtap);

	it_present = rtap->hdr->it_present;
	idx = rtap->idx;
	offset = rtap->offset;

	/* loop through each field index until we get to the target idx */
	while (idx <= search_idx) {
		/* if field 'idx' is present, update the offset and check for a match */
		if ((1 << idx) & it_present) {
			/* if we hit a field for which we have no parse info
			 * we need to just bail out
			 */
			if (rtap_parse_info[idx].align == 0)
				break;

			/* step past any alignment padding */
			align = rtap_parse_info[idx].align;
			len = rtap_parse_info[idx].len;

			/* ROUNDUP */
			field_offset = ((offset + (align - 1)) / align) * align;

			/* if this field is not in the boulds of the header
			 * just bail out
			 */
			if (field_offset + len > rtap->fields_len)
				break;

			/* did we find the field? */
			if (idx == search_idx)
				ptr = (uint8*)rtap->fields + field_offset;

			/* step past this field */
			offset = field_offset + len;
		}

		idx++;
	}

	rtap->idx = idx;
	rtap->offset = offset;

	return ptr;
}

ratespec_t
wl_calcRspecFromRTap(uint8 *rtap_header)
{
	ratespec_t rspec = 0;
	radiotap_parse_t rtap;
	uint8 rate = 0;
	uint8 flags = 0;
	int flags_present = FALSE;
	uint8 mcs = 0;
	uint8 mcs_flags = 0;
	uint8 mcs_known = 0;
	int mcs_present = FALSE;
	void *p;

	wl_rtapParseInit(&rtap, rtap_header);

	p = wl_rtapParseFindField(&rtap, IEEE80211_RADIOTAP_FLAGS);
	if (p != NULL) {
		flags_present = TRUE;
		flags = ((uint8*)p)[0];
	}

	p = wl_rtapParseFindField(&rtap, IEEE80211_RADIOTAP_RATE);
	if (p != NULL)
		rate = ((uint8*)p)[0];

	p = wl_rtapParseFindField(&rtap, IEEE80211_RADIOTAP_MCS);
	if (p != NULL) {
		mcs_present = TRUE;
		mcs_known = ((uint8*)p)[0];
		mcs_flags = ((uint8*)p)[1];
		mcs = ((uint8*)p)[2];
	}

	if (rate != 0) {
		/* validate the DSSS rates 1,2,5.5,11 */
		if (rate == 2 || rate == 4 || rate == 11 || rate == 22) {
			rspec = LEGACY_RSPEC(rate) | RSPEC_OVERRIDE_RATE;
			if (flags_present && (flags & IEEE80211_RADIOTAP_F_SHORTPRE)) {
				rspec |= RSPEC_OVERRIDE_MODE | RSPEC_SHORT_PREAMBLE;
			}
		}
	} else if (mcs_present) {
		/* validate the MCS value */
		if (mcs <= 23 || mcs == 32 || IS_PROPRIETARY_11N_MCS(mcs)) {
			uint32 override = 0;
			if (mcs_known &
			    (IEEE80211_RADIOTAP_MCS_HAVE_GI |
			     IEEE80211_RADIOTAP_MCS_HAVE_FMT |
			     IEEE80211_RADIOTAP_MCS_HAVE_FEC)) {
				override = RSPEC_OVERRIDE_MODE;
			}

			rspec = HT_RSPEC(mcs) | RSPEC_OVERRIDE_RATE;

			if ((mcs_known & IEEE80211_RADIOTAP_MCS_HAVE_GI) &&
			    (mcs_flags & IEEE80211_RADIOTAP_MCS_SGI))
				rspec |= RSPEC_SHORT_GI;
			if ((mcs_known & IEEE80211_RADIOTAP_MCS_HAVE_FMT) &&
			    (mcs_flags & IEEE80211_RADIOTAP_MCS_FMT_GF))
				rspec |= RSPEC_SHORT_PREAMBLE;
			if ((mcs_known & IEEE80211_RADIOTAP_MCS_HAVE_FEC) &&
			    (mcs_flags & IEEE80211_RADIOTAP_MCS_FEC_LDPC))
				rspec |= RSPEC_LDPC_CODING;

			rspec |= override;
		}
	}

	return rspec;
}

bool
wl_rtapFlags(uint8 *rtap_header, uint8* flags)
{
	radiotap_parse_t rtap;
	void *p;

	wl_rtapParseInit(&rtap, rtap_header);

	p = wl_rtapParseFindField(&rtap, IEEE80211_RADIOTAP_FLAGS);
	if (p != NULL) {
		*flags = ((uint8*)p)[0];
	}

	return (p != NULL);
}

void
wl_rtapParseInit(radiotap_parse_t *rtap, uint8 *rtap_header)
{
	uint rlen;
	uint32 *present_word;
	struct ieee80211_radiotap_header *hdr = (struct ieee80211_radiotap_header*)rtap_header;

	memset(rtap, 0, sizeof(radiotap_parse_t));

	rlen = hdr->it_len; /* total space in rtap_header */

	/* If a precence word has the IEEE80211_RADIOTAP_EXT bit set it indicates
	 * that there is another precence word.
	 * Step over the presence words until we find the end of the list
	 */
	present_word = &hdr->it_present;
	/* remaining length in header past it_present */
	rlen -= sizeof(struct ieee80211_radiotap_header);

	while ((*present_word & (1<<IEEE80211_RADIOTAP_EXT)) && rlen >= 4) {
		present_word++;
		rlen -= 4;	/* account for 4 bytes of present_word */
	}

	rtap->hdr = hdr;
	rtap->fields = (uint8*)(present_word + 1);
	rtap->fields_len = rlen;
	wl_rtapParseReset(rtap);
}

uint
wl_radiotap_rx(struct dot11_header *mac_header,	wl_rxsts_t *rxsts, bsd_header_rx_t *bsd_header)
{
	int channel_frequency;
	uint32 channel_flags;
	uint8 flags;
	uint8 *cp;
	uint pad_len;
	uint32 field_map;
	uint16 fc;
	uint bsd_header_len;
	uint16 ampdu_flags = 0;

	fc = LTOH16(mac_header->fc);
	pad_len = 3;
	field_map = WL_RADIOTAP_PRESENT_RX;

	if (CHSPEC_IS2G(rxsts->chanspec)) {
		channel_flags = IEEE80211_CHAN_2GHZ | IEEE80211_CHAN_DYN;
		channel_frequency = wf_channel2mhz(wf_chspec_ctlchan(rxsts->chanspec),
			WF_CHAN_FACTOR_2_4_G);
	} else {
		channel_flags = IEEE80211_CHAN_5GHZ | IEEE80211_CHAN_OFDM;
		channel_frequency = wf_channel2mhz(wf_chspec_ctlchan(rxsts->chanspec),
			WF_CHAN_FACTOR_5_G);
	}
	WL_TMP(("%s %d: chanspec 0x%x ctlchan 0x%x freq: %d 0x%x \n", __FUNCTION__, __LINE__,
		rxsts->chanspec, wf_chspec_ctlchan(rxsts->chanspec), channel_frequency, fc));

	if (rxsts->nfrmtype == WL_RXS_NFRM_AMPDU_FIRST ||
		rxsts->nfrmtype == WL_RXS_NFRM_AMPDU_SUB) {

		ampdu_flags = IEEE80211_RADIOTAP_AMPDU_LAST_KNOWN;
	}

	flags = IEEE80211_RADIOTAP_F_FCS;

	if (rxsts->preamble == WL_RXS_PREAMBLE_SHORT)
		flags |= IEEE80211_RADIOTAP_F_SHORTPRE;

	if ((fc &  FC_WEP) == FC_WEP)
		flags |= IEEE80211_RADIOTAP_F_WEP;

	if ((fc & FC_MOREFRAG) == FC_MOREFRAG)
		flags |= IEEE80211_RADIOTAP_F_FRAG;

	if (rxsts->pkterror & WL_RXS_CRC_ERROR)
		flags |= IEEE80211_RADIOTAP_F_BADFCS;

	if (rxsts->encoding == WL_RXS_ENCODING_HT)
		field_map = WL_RADIOTAP_PRESENT_RX_HT;
#ifdef WL11AC
	else if (rxsts->encoding == WL_RXS_ENCODING_VHT)
		field_map = WL_RADIOTAP_PRESENT_RX_VHT;

#endif /* WL11AC */
	bsd_header_len = sizeof(struct wl_radiotap_sna); /* start with sna size */
	/* Test for signal/noise values and update length and field bitmap */
	if (rxsts->signal == 0) {
		field_map &= ~(1 << IEEE80211_RADIOTAP_DBM_ANTSIGNAL);
		pad_len = (pad_len - 1);
		bsd_header_len--;
	}

	if (rxsts->noise == 0) {
		field_map &= ~(1 << IEEE80211_RADIOTAP_DBM_ANTNOISE);
		pad_len = (pad_len - 1);
		bsd_header_len--;
	}

	if (rxsts->encoding == WL_RXS_ENCODING_HT ||
	    rxsts->encoding == WL_RXS_ENCODING_VHT) {
		struct wl_radiotap_hdr *rtht = &bsd_header->hdr;
		struct wl_radiotap_ht_tail *tail;

		/*
		 * Header length is complicated due to dynamic
		 * presence of signal and noise fields
		 * and padding for xchannel following
		 * signal/noise/ant.
		 * Start with length of wl_radiotap_ht plus
		 * signal/noise/ant
		 */
		bsd_header_len += sizeof(struct wl_radiotap_hdr) + pad_len;
		bsd_header_len += sizeof(struct wl_radiotap_xchan);
		if (rxsts->nfrmtype == WL_RXS_NFRM_AMPDU_FIRST ||
			rxsts->nfrmtype == WL_RXS_NFRM_AMPDU_SUB) {
			bsd_header_len += sizeof(struct wl_radiotap_ampdu);
		}
		/* add the length of the tail end of the structure */
		if (rxsts->encoding == WL_RXS_ENCODING_HT)
			bsd_header_len += sizeof(struct wl_htmcs);
#ifdef WL11AC
		else if (rxsts->encoding == WL_RXS_ENCODING_VHT)
			bsd_header_len += sizeof(struct wl_vhtmcs);
#endif /* WL11AC */
		bzero((uint8 *)rtht, sizeof(*rtht));

		rtht->ieee_radiotap.it_version = 0;
		rtht->ieee_radiotap.it_pad = 0;
		rtht->ieee_radiotap.it_len = HTOL16(bsd_header_len);
		rtht->ieee_radiotap.it_present = HTOL32(field_map);

		rtht->tsft = HTOL64((uint64)rxsts->mactime);
		rtht->flags = flags;
		rtht->channel_freq = HTOL16(channel_frequency);
		rtht->channel_flags = HTOL16(channel_flags);

		cp = bsd_header->pad;
		/* add in signal/noise/ant */
		if (rxsts->signal != 0) {
			*cp++ = (int8)rxsts->signal;
			pad_len--;
		}
		if (rxsts->noise != 0) {
			*cp++ = (int8)rxsts->noise;
			pad_len--;
		}
		*cp++ = (int8)rxsts->antenna;
		pad_len--;

		tail = (struct wl_radiotap_ht_tail *)(bsd_header->ht);
		/* Fill in XCHANNEL */
		if (CHSPEC_IS40(rxsts->chanspec)) {
			if (CHSPEC_SB_UPPER(rxsts->chanspec))
				channel_flags |= IEEE80211_CHAN_HT40D;
			else
				channel_flags |= IEEE80211_CHAN_HT40U;
		} else
			channel_flags |= IEEE80211_CHAN_HT20;

		tail->xc.xchannel_flags = HTOL32(channel_flags);
		tail->xc.xchannel_freq = (uint16)HTOL16(channel_frequency);
		tail->xc.xchannel_channel = wf_chspec_ctlchan(rxsts->chanspec);
		tail->xc.xchannel_maxpower = (17*2);
		/* fill in A-mpdu Status */
		tail->ampdu.ref_num = mac_header->seq;
		tail->ampdu.flags = ampdu_flags;
		tail->ampdu.delimiter_crc = 0;
		tail->ampdu.reserved = 0;

		if (rxsts->encoding == WL_RXS_ENCODING_HT) {
			tail->u.ht.mcs_index = rxsts->mcs;
			tail->u.ht.mcs_known = (IEEE80211_RADIOTAP_MCS_HAVE_BW |
			                        IEEE80211_RADIOTAP_MCS_HAVE_MCS |
			                        IEEE80211_RADIOTAP_MCS_HAVE_GI |
			                        IEEE80211_RADIOTAP_MCS_HAVE_FEC |
			                        IEEE80211_RADIOTAP_MCS_HAVE_FMT);
			tail->u.ht.mcs_flags = 0;

			switch (rxsts->htflags & WL_RXS_HTF_BW_MASK) {
				case WL_RXS_HTF_20L:
					tail->u.ht.mcs_flags |= IEEE80211_RADIOTAP_MCS_BW_20L;
					break;
				case WL_RXS_HTF_20U:
					tail->u.ht.mcs_flags |= IEEE80211_RADIOTAP_MCS_BW_20U;
					break;
				case WL_RXS_HTF_40:
					tail->u.ht.mcs_flags |= IEEE80211_RADIOTAP_MCS_BW_40;
					break;
				default:
					tail->u.ht.mcs_flags |= IEEE80211_RADIOTAP_MCS_BW_20;
			}

			if (rxsts->htflags & WL_RXS_HTF_SGI)
				tail->u.ht.mcs_flags |= IEEE80211_RADIOTAP_MCS_SGI;
			if (rxsts->preamble & WL_RXS_PREAMBLE_HT_GF)
				tail->u.ht.mcs_flags |= IEEE80211_RADIOTAP_MCS_FMT_GF;
			if (rxsts->htflags & WL_RXS_HTF_LDPC)
				tail->u.ht.mcs_flags |= IEEE80211_RADIOTAP_MCS_FEC_LDPC;
		}
#ifdef WL11AC
		else if (rxsts->encoding == WL_RXS_ENCODING_VHT) {
			tail->u.vht.vht_known = (IEEE80211_RADIOTAP_VHT_HAVE_STBC |
			                         IEEE80211_RADIOTAP_VHT_HAVE_TXOP_PS |
			                         IEEE80211_RADIOTAP_VHT_HAVE_GI |
			                         IEEE80211_RADIOTAP_VHT_HAVE_SGI_NSYM_DA |
			                         IEEE80211_RADIOTAP_VHT_HAVE_LDPC_EXTRA |
			                         IEEE80211_RADIOTAP_VHT_HAVE_BF |
			                         IEEE80211_RADIOTAP_VHT_HAVE_BW |
			                         IEEE80211_RADIOTAP_VHT_HAVE_GID |
			                         IEEE80211_RADIOTAP_VHT_HAVE_PAID);

			tail->u.vht.vht_flags = (uint8)HTOL16(rxsts->vhtflags);

			switch (rxsts->bw) {
				case WL_RXS_VHT_BW_20:
					tail->u.vht.vht_bw = IEEE80211_RADIOTAP_VHT_BW_20;
					break;
				case WL_RXS_VHT_BW_40:
					tail->u.vht.vht_bw = IEEE80211_RADIOTAP_VHT_BW_40;
					break;
				case WL_RXS_VHT_BW_20L:
					tail->u.vht.vht_bw = IEEE80211_RADIOTAP_VHT_BW_20L;
					break;
				case WL_RXS_VHT_BW_20U:
					tail->u.vht.vht_bw = IEEE80211_RADIOTAP_VHT_BW_20U;
					break;
				case WL_RXS_VHT_BW_80:
					tail->u.vht.vht_bw = IEEE80211_RADIOTAP_VHT_BW_80;
					break;
				case WL_RXS_VHT_BW_40L:
					tail->u.vht.vht_bw = IEEE80211_RADIOTAP_VHT_BW_40L;
					break;
				case WL_RXS_VHT_BW_40U:
					tail->u.vht.vht_bw = IEEE80211_RADIOTAP_VHT_BW_40U;
					break;
				case WL_RXS_VHT_BW_20LL:
					tail->u.vht.vht_bw = IEEE80211_RADIOTAP_VHT_BW_20LL;
					break;
				case WL_RXS_VHT_BW_20LU:
					tail->u.vht.vht_bw = IEEE80211_RADIOTAP_VHT_BW_20LU;
					break;
				case WL_RXS_VHT_BW_20UL:
					tail->u.vht.vht_bw = IEEE80211_RADIOTAP_VHT_BW_20UL;
					break;
				case WL_RXS_VHT_BW_20UU:
					tail->u.vht.vht_bw = IEEE80211_RADIOTAP_VHT_BW_20UU;
					break;
				default:
					tail->u.vht.vht_bw = IEEE80211_RADIOTAP_VHT_BW_20;
					break;
			}

			tail->u.vht.vht_mcs_nss[0] = (rxsts->mcs << 4) |
				(rxsts->nss & IEEE80211_RADIOTAP_VHT_NSS);
			tail->u.vht.vht_mcs_nss[1] = 0;
			tail->u.vht.vht_mcs_nss[2] = 0;
			tail->u.vht.vht_mcs_nss[3] = 0;

			tail->u.vht.vht_coding = rxsts->coding;
			tail->u.vht.vht_group_id = rxsts->gid;
			tail->u.vht.vht_partial_aid = HTOL16(rxsts->aid);
		}
#endif /* WL11AC */
	} else {
		struct wl_radiotap_hdr *rtl = &bsd_header->hdr;

		/*
		 * Header length is complicated due to dynamic presence of signal and noise fields
		 * Start with length of wl_radiotap_legacy plus signal/noise/ant
		 */
		bsd_header_len = sizeof(struct wl_radiotap_hdr) + pad_len;
		bzero((uint8 *)rtl, sizeof(*rtl));

		rtl->ieee_radiotap.it_version = 0;
		rtl->ieee_radiotap.it_pad = 0;
		rtl->ieee_radiotap.it_len = (uint16)HTOL16(bsd_header_len);
		rtl->ieee_radiotap.it_present = HTOL32(field_map);

		rtl->tsft = HTOL64((uint64)rxsts->mactime);
		rtl->flags = flags;
		rtl->u.rate = (uint8)rxsts->datarate;
		rtl->channel_freq = (uint16)HTOL16(channel_frequency);
		rtl->channel_flags = (uint16)HTOL16(channel_flags);

		/* add in signal/noise/ant */
		cp = bsd_header->pad;
		if (rxsts->signal != 0)
			*cp++ = (int8)rxsts->signal;
		if (rxsts->noise != 0)
			*cp++ = (int8)rxsts->noise;
		*cp++ = (int8)rxsts->antenna;
	}
	return bsd_header_len;
}

#ifdef WLTXMONITOR
uint
wl_radiotap_tx(struct dot11_header *mac_header,	wl_txsts_t *txsts, bsd_header_tx_t *bsd_header)
{
	int channel_frequency;
	uint32 channel_flags;
	uint8 flags;
	uint16 fc = LTOH16(mac_header->fc);
	uint bsd_header_len;

	if (CHSPEC_IS2G(txsts->chanspec)) {
		channel_flags = IEEE80211_CHAN_2GHZ;
		channel_frequency = wf_channel2mhz(wf_chspec_ctlchan(txsts->chanspec),
			WF_CHAN_FACTOR_2_4_G);
	} else {
		channel_flags = IEEE80211_CHAN_5GHZ;
		channel_frequency = wf_channel2mhz(wf_chspec_ctlchan(txsts->chanspec),
			WF_CHAN_FACTOR_5_G);
	}
	WL_TMP(("%s %d: chanspec 0x%x ctlchan 0x%x freq: %d 0x%x \n", __FUNCTION__, __LINE__,
		txsts->chanspec, wf_chspec_ctlchan(txsts->chanspec), channel_frequency, fc));

	flags = 0;

	if (txsts->preamble == WL_RXS_PREAMBLE_SHORT)
		flags |= IEEE80211_RADIOTAP_F_SHORTPRE;

	if ((fc &  FC_WEP) == FC_WEP)
		flags |= IEEE80211_RADIOTAP_F_WEP;

	if ((fc & FC_MOREFRAG) == FC_MOREFRAG)
		flags |= IEEE80211_RADIOTAP_F_FRAG;

	if (txsts->datarate != 0) {
		uint32 field_map = WL_RADIOTAP_PRESENT_TX;
		struct wl_radiotap_hdr_tx *rtl = &bsd_header->hdr;

		/*
		 * Header length is complicated due to dynamic presence of signal and noise fields
		 * Start with length of wl_radiotap_legacy plus signal/noise/ant
		 */
		bsd_header_len = sizeof(struct wl_radiotap_hdr_tx) + 3;
		bzero((uint8 *)rtl, sizeof(bsd_header));

		rtl->ieee_radiotap.it_version = 0;
		rtl->ieee_radiotap.it_pad = 0;
		rtl->ieee_radiotap.it_len = HTOL16(bsd_header_len);
		rtl->ieee_radiotap.it_present = HTOL32(field_map);

		rtl->tsft = HTOL64((uint64)txsts->mactime);
		rtl->flags = flags;
		rtl->u.rate = txsts->datarate;
		rtl->channel_freq = HTOL16(channel_frequency);
		rtl->channel_flags = HTOL16(channel_flags);
	} else {
		uint32 field_map = WL_RADIOTAP_PRESENT_HT_TX;
		struct wl_radiotap_hdr_tx *rtht = &bsd_header->hdr;
		struct wl_radiotap_ht_tail *tail;
		uint pad_len;

		/*
		 * Header length is complicated due to dynamic presence of signal and noise fields
		 * and padding for xchannel following signal/noise/ant.
		 * Start with length of wl_radiotap_ht plus signal/noise/ant
		 */
		bsd_header_len = sizeof(struct wl_radiotap_hdr_tx);

		/* calc pad for xchannel field */
		pad_len = 3;

		/* add the length of the tail end of the structure */
		bsd_header_len += pad_len + sizeof(struct wl_radiotap_ht_tail);
		bzero((uint8 *)rtht, sizeof(*rtht));

		rtht->ieee_radiotap.it_version = 0;
		rtht->ieee_radiotap.it_pad = 0;
		rtht->ieee_radiotap.it_len = HTOL16(bsd_header_len);
		rtht->ieee_radiotap.it_present = HTOL32(field_map);

		rtht->tsft = HTOL64((uint64)txsts->mactime);
		rtht->flags = flags;
		rtht->u.pad = 0;
		rtht->channel_freq = HTOL16(channel_frequency);
		rtht->channel_flags = HTOL16(channel_flags);

		rtht->txflags = HTOL16(txsts->txflags);
		rtht->retries = txsts->retries;

		tail = (struct wl_radiotap_ht_tail *)(bsd_header->ht + pad_len);
		tail = (struct wl_radiotap_ht_tail*)bsd_header->ht;

		/* Fill in XCHANNEL */
		if (CHSPEC_IS40(txsts->chanspec)) {
			if (CHSPEC_SB_UPPER(txsts->chanspec))
				channel_flags |= IEEE80211_CHAN_HT40D;
			else
				channel_flags |= IEEE80211_CHAN_HT40U;
		} else
			channel_flags |= IEEE80211_CHAN_HT20;

		tail->xc.xchannel_flags = HTOL32(channel_flags);
		tail->xc.xchannel_freq = HTOL16(channel_frequency);
		tail->xc.xchannel_channel = wf_chspec_ctlchan(txsts->chanspec);
		tail->xc.xchannel_maxpower = (17*2);

		tail->u.ht.mcs_index = txsts->mcs;
		tail->u.ht.mcs_known = (IEEE80211_RADIOTAP_MCS_HAVE_MCS |
		                        IEEE80211_RADIOTAP_MCS_HAVE_BW |
		                        IEEE80211_RADIOTAP_MCS_HAVE_GI |
		                        IEEE80211_RADIOTAP_MCS_HAVE_FMT |
		                        IEEE80211_RADIOTAP_MCS_HAVE_FEC);
		tail->u.ht.mcs_flags = 0;
		if (txsts->htflags & WL_RXS_HTF_40) {
			tail->u.ht.mcs_flags |= IEEE80211_RADIOTAP_MCS_BW_40;
		} else if (CHSPEC_IS40(txsts->chanspec)) {
			if (CHSPEC_SB_UPPER(txsts->chanspec))
				tail->u.ht.mcs_flags |= IEEE80211_RADIOTAP_MCS_BW_20U;
			else
				tail->u.ht.mcs_flags |= IEEE80211_RADIOTAP_MCS_BW_20L;
		} else
			tail->u.ht.mcs_flags |= IEEE80211_RADIOTAP_MCS_BW_20;

		if (txsts->htflags & WL_RXS_HTF_SGI)
			tail->u.ht.mcs_flags |= IEEE80211_RADIOTAP_MCS_SGI;

		if (txsts->preamble & WL_RXS_PREAMBLE_HT_GF)
			tail->u.ht.mcs_flags |= IEEE80211_RADIOTAP_MCS_FMT_GF;

		if (txsts->htflags & WL_RXS_HTF_LDPC)
			tail->u.ht.mcs_flags |= IEEE80211_RADIOTAP_MCS_FEC_LDPC;
	}
	return bsd_header_len;
}
#endif /* WLTXMONITOR */

#ifdef WL_RADIOTAP
uint
wl_radiotap_rx_p80211msg(char *devname, wl_rxsts_t *rxsts, p80211msg_t *phdr)
{
	phdr->msgcode = WL_MON_FRAME;
	phdr->msglen = sizeof(p80211msg_t);
	strncpy((char *)phdr->devname, devname, DNAMELEN);
	phdr->devname[DNAMELEN - 1] = '\0';

	phdr->hosttime.did = WL_MON_FRAME_HOSTTIME;
	phdr->hosttime.status = P80211ITEM_OK;
	phdr->hosttime.len = 4;
	phdr->hosttime.data = 0;

	phdr->channel.did = WL_MON_FRAME_CHANNEL;
	phdr->channel.status = P80211ITEM_NO_VALUE;
	phdr->channel.len = 4;
	phdr->channel.data = 0;

	phdr->signal.did = WL_MON_FRAME_SIGNAL;
	phdr->signal.status = P80211ITEM_OK;
	phdr->signal.len = 4;
	/* two sets of preamble values are defined in wlc_ethereal and wlc_pub.h
	 * and this assumet their values are matched. Otherwise,
	 * we have to go through conversion, which requires rspec since datarate
	 * is just kbps now
	 */
	phdr->signal.data = rxsts->preamble;

	phdr->noise.did = WL_MON_FRAME_NOISE;
	phdr->noise.status = P80211ITEM_NO_VALUE;
	phdr->noise.len = 4;
	phdr->noise.data = 0;

	phdr->rate.did = WL_MON_FRAME_RATE;
	phdr->rate.status = P80211ITEM_OK;
	phdr->rate.len = 4;
	phdr->rate.data = rxsts->datarate;

	phdr->istx.did = WL_MON_FRAME_ISTX;
	phdr->istx.status = P80211ITEM_NO_VALUE;
	phdr->istx.len = 4;
	phdr->istx.data = 0;

	phdr->mactime.did = WL_MON_FRAME_MACTIME;
	phdr->mactime.status = P80211ITEM_OK;
	phdr->mactime.len = 4;
	phdr->mactime.data = rxsts->mactime;

	phdr->rssi.did = WL_MON_FRAME_RSSI;
	phdr->rssi.status = P80211ITEM_OK;
	phdr->rssi.len = 4;
	phdr->rssi.data = rxsts->signal;		/* to dbm */

	phdr->sq.did = WL_MON_FRAME_SQ;
	phdr->sq.status = P80211ITEM_OK;
	phdr->sq.len = 4;
	phdr->sq.data = rxsts->sq;

	phdr->frmlen.did = WL_MON_FRAME_FRMLEN;
	phdr->frmlen.status = P80211ITEM_OK;
	phdr->frmlen.status = P80211ITEM_OK;
	phdr->frmlen.len = 4;
	phdr->frmlen.data = rxsts->pktlength;

	return 0;
}

static int
wl_radiotap_rx_channel_frequency(wl_rxsts_t *rxsts)
{
	if (CHSPEC_IS2G(rxsts->chanspec))
		return wf_channel2mhz(wf_chspec_ctlchan(rxsts->chanspec),
			WF_CHAN_FACTOR_2_4_G);
	else
		return wf_channel2mhz(wf_chspec_ctlchan(rxsts->chanspec),
			WF_CHAN_FACTOR_5_G);
}

static uint16
wl_radiotap_rx_channel_flags(wl_rxsts_t *rxsts)
{
	if (CHSPEC_IS2G(rxsts->chanspec))
		return (IEEE80211_CHAN_2GHZ | IEEE80211_CHAN_DYN);
	else
		return (IEEE80211_CHAN_5GHZ | IEEE80211_CHAN_OFDM);
}

static uint8
wl_radiotap_rx_flags(struct dot11_header *mac_header, wl_rxsts_t *rxsts)
{
	uint8 flags;
	uint16 fc;

	fc = ltoh16(mac_header->fc);

	flags = IEEE80211_RADIOTAP_F_FCS;

	if (rxsts->preamble == WL_RXS_PREAMBLE_SHORT)
		flags |= IEEE80211_RADIOTAP_F_SHORTPRE;

	if (fc & FC_WEP)
		flags |= IEEE80211_RADIOTAP_F_WEP;

	if (fc & FC_MOREFRAG)
		flags |= IEEE80211_RADIOTAP_F_FRAG;

	return flags;
}

uint
wl_radiotap_rx_legacy(struct dot11_header *mac_header, wl_rxsts_t *rxsts, wl_radiotap_legacy_t *rtl)
{
	int channel_frequency;
	uint16 channel_flags;
	uint8 flags;
	uint16 rtap_len;
#ifdef WL11AC
	uint16 fc;
#endif /* WL11AC */

	rtap_len = sizeof(wl_radiotap_legacy_t);
	channel_frequency = wl_radiotap_rx_channel_flags(rxsts);
	channel_flags = (uint16)wl_radiotap_rx_channel_frequency(rxsts);
	flags = wl_radiotap_rx_flags(mac_header, rxsts);

	rtl->ieee_radiotap.it_version = 0;
	rtl->ieee_radiotap.it_pad = 0;
	rtl->ieee_radiotap.it_len = HTOL16(rtap_len);
	rtl->ieee_radiotap.it_present = HTOL32(WL_RADIOTAP_PRESENT_LEGACY);

	rtl->it_present_ext = HTOL32(WL_RADIOTAP_LEGACY_VHT);
	rtl->tsft_l = htol32(rxsts->mactime);
	rtl->tsft_h = 0;
	rtl->flags = flags;
	rtl->rate = (uint8)rxsts->datarate;
	rtl->channel_freq = (uint16)HTOL16(channel_frequency);
	rtl->channel_flags = HTOL16(channel_flags);
	rtl->signal = (int8)rxsts->signal;
	rtl->noise = (int8)rxsts->noise;
	rtl->antenna = (int8)rxsts->antenna;

	/* Broadcom specific */
	memcpy(rtl->vend_oui, brcm_oui, sizeof(brcm_oui));
	rtl->vend_skip_len = WL_RADIOTAP_LEGACY_SKIP_LEN;
	rtl->vend_sns = 0;

	/* VHT b/w signalling */
	memset(&rtl->nonht_vht, 0, sizeof(rtl->nonht_vht));
	rtl->nonht_vht.len = WL_RADIOTAP_NONHT_VHT_LEN;
#ifdef WL11AC
	fc = ltoh16(mac_header->fc);
	if (((fc & FC_KIND_MASK) == FC_RTS) ||
		((fc & FC_KIND_MASK) == FC_CTS)) {
		rtl->nonht_vht.flags |= WL_RADIOTAP_F_NONHT_VHT_BW;
		rtl->nonht_vht.bw = rxsts->bw_nonht;
		rtl->vend_sns = WL_RADIOTAP_LEGACY_SNS;

	}
	if ((fc & FC_KIND_MASK) == FC_RTS) {
		if (rxsts->vhtflags & WL_RXS_VHTF_DYN_BW_NONHT)
			rtl->nonht_vht.flags
				|= WL_RADIOTAP_F_NONHT_VHT_DYN_BW;
	}
#endif /* WL11AC */

	return 0;
}

uint
wl_radiotap_rx_ht(struct dot11_header *mac_header, wl_rxsts_t *rxsts, wl_radiotap_ht_t *rtht)
{
	int channel_frequency;
	uint16 channel_flags;
	uint8 flags;
	uint16 rtap_len;

	rtap_len = sizeof(wl_radiotap_ht_t);
	channel_frequency = wl_radiotap_rx_channel_flags(rxsts);
	channel_flags = (uint16)wl_radiotap_rx_channel_frequency(rxsts);
	flags = wl_radiotap_rx_flags(mac_header, rxsts);

	rtht->ieee_radiotap.it_version = 0;
	rtht->ieee_radiotap.it_pad = 0;
	rtht->ieee_radiotap.it_len = HTOL16(rtap_len);
	rtht->ieee_radiotap.it_present
		= HTOL32(WL_RADIOTAP_PRESENT_HT);
	rtht->pad1 = 0;

	rtht->tsft_l = htol32(rxsts->mactime);
	rtht->tsft_h = 0;
	rtht->flags = flags;
	rtht->channel_freq = (uint16)HTOL16(channel_frequency);
	rtht->channel_flags = HTOL16(channel_flags);
	rtht->signal = (int8)rxsts->signal;
	rtht->noise = (int8)rxsts->noise;
	rtht->antenna = (uint8)rxsts->antenna;

	/* add standard MCS */
	rtht->mcs_known = (IEEE80211_RADIOTAP_MCS_HAVE_BW |
		IEEE80211_RADIOTAP_MCS_HAVE_MCS |
		IEEE80211_RADIOTAP_MCS_HAVE_GI |
		IEEE80211_RADIOTAP_MCS_HAVE_FEC |
		IEEE80211_RADIOTAP_MCS_HAVE_FMT);

	rtht->mcs_flags = 0;
	switch (rxsts->htflags & WL_RXS_HTF_BW_MASK) {
		case WL_RXS_HTF_20L:
			rtht->mcs_flags |= IEEE80211_RADIOTAP_MCS_BW_20L;
			break;
		case WL_RXS_HTF_20U:
			rtht->mcs_flags |= IEEE80211_RADIOTAP_MCS_BW_20U;
			break;
		case WL_RXS_HTF_40:
			rtht->mcs_flags |= IEEE80211_RADIOTAP_MCS_BW_40;
			break;
		default:
			rtht->mcs_flags |= IEEE80211_RADIOTAP_MCS_BW_20;
	}

	if (rxsts->htflags & WL_RXS_HTF_SGI) {
		rtht->mcs_flags |= IEEE80211_RADIOTAP_MCS_SGI;
	}
	if (rxsts->preamble & WL_RXS_PREAMBLE_HT_GF) {
		rtht->mcs_flags |= IEEE80211_RADIOTAP_MCS_FMT_GF;
	}
	if (rxsts->htflags & WL_RXS_HTF_LDPC) {
		rtht->mcs_flags |= IEEE80211_RADIOTAP_MCS_FEC_LDPC;
	}
	rtht->mcs_index = rxsts->mcs;

	return 0;
}

uint
wl_radiotap_rx_ht_brcm(struct dot11_header *mac_header, wl_rxsts_t *rxsts,
		wl_radiotap_ht_brcm_t *rtht)
{
	int channel_frequency;
	uint16 channel_flags;
	uint8 flags;
	uint16 rtap_len;

	rtap_len = sizeof(wl_radiotap_ht_brcm_t);
	channel_frequency = wl_radiotap_rx_channel_flags(rxsts);
	channel_flags = (uint16)wl_radiotap_rx_channel_frequency(rxsts);
	flags = wl_radiotap_rx_flags(mac_header, rxsts);

	rtht->ieee_radiotap.it_version = 0;
	rtht->ieee_radiotap.it_pad = 0;
	rtht->ieee_radiotap.it_len = HTOL16(rtap_len);
	rtht->ieee_radiotap.it_present = HTOL32(WL_RADIOTAP_PRESENT_HT_BRCM);
	rtht->it_present_ext = HTOL32(WL_RADIOTAP_BRCM_MCS);
	rtht->pad1 = 0;

	rtht->tsft_l = htol32(rxsts->mactime);
	rtht->tsft_h = 0;
	rtht->flags = flags;
	rtht->pad2 = 0;
	rtht->channel_freq = (uint16)HTOL16(channel_frequency);
	rtht->channel_flags = HTOL16(channel_flags);
	rtht->signal = (int8)rxsts->signal;
	rtht->noise = (int8)rxsts->noise;
	rtht->antenna = (uint8)rxsts->antenna;
	rtht->pad3 = 0;

	memcpy(rtht->vend_oui, brcm_oui, sizeof(brcm_oui));
	rtht->vend_sns = WL_RADIOTAP_BRCM_SNS;
	rtht->vend_skip_len = 2;
	rtht->mcs = rxsts->mcs;
	rtht->htflags = 0;
	if (rxsts->htflags & WL_RXS_HTF_40)
		rtht->htflags |= IEEE80211_RADIOTAP_HTMOD_40;
	if (rxsts->htflags & WL_RXS_HTF_SGI)
		rtht->htflags |= IEEE80211_RADIOTAP_HTMOD_SGI;
	if (rxsts->preamble & WL_RXS_PREAMBLE_HT_GF)
		rtht->htflags |= IEEE80211_RADIOTAP_HTMOD_GF;
	if (rxsts->htflags & WL_RXS_HTF_LDPC)
		rtht->htflags |= IEEE80211_RADIOTAP_HTMOD_LDPC;
	rtht->htflags |=
		(rxsts->htflags & WL_RXS_HTF_STBC_MASK) <<
		IEEE80211_RADIOTAP_HTMOD_STBC_SHIFT;

	return 0;
}

uint
wl_radiotap_rx_vht(struct dot11_header *mac_header, wl_rxsts_t *rxsts, wl_radiotap_vht_t *rtvht)
{
	int channel_frequency;
	uint16 channel_flags;
	uint8 flags;
	uint16 rtap_len;

	rtap_len = sizeof(wl_radiotap_vht_t);
	channel_frequency = wl_radiotap_rx_channel_flags(rxsts);
	channel_flags = (uint16)wl_radiotap_rx_channel_frequency(rxsts);
	flags = wl_radiotap_rx_flags(mac_header, rxsts);

	rtvht->ieee_radiotap.it_version = 0;
	rtvht->ieee_radiotap.it_pad = 0;
	rtvht->ieee_radiotap.it_len = HTOL16(rtap_len);
	rtvht->ieee_radiotap.it_present =
		HTOL32(WL_RADIOTAP_PRESENT_VHT);

	rtvht->tsft_l = htol32(rxsts->mactime);
	rtvht->tsft_h = 0;
	rtvht->flags = flags;
	rtvht->pad1 = 0;
	rtvht->channel_freq = (uint16)HTOL16(channel_frequency);
	rtvht->channel_flags = HTOL16(channel_flags);
	rtvht->signal = (int8)rxsts->signal;
	rtvht->noise = (int8)rxsts->noise;
	rtvht->antenna = (uint8)rxsts->antenna;

	rtvht->vht_known = (IEEE80211_RADIOTAP_VHT_HAVE_STBC |
		IEEE80211_RADIOTAP_VHT_HAVE_TXOP_PS |
		IEEE80211_RADIOTAP_VHT_HAVE_GI |
		IEEE80211_RADIOTAP_VHT_HAVE_SGI_NSYM_DA |
		IEEE80211_RADIOTAP_VHT_HAVE_LDPC_EXTRA |
		IEEE80211_RADIOTAP_VHT_HAVE_BF |
		IEEE80211_RADIOTAP_VHT_HAVE_BW |
		IEEE80211_RADIOTAP_VHT_HAVE_GID |
		IEEE80211_RADIOTAP_VHT_HAVE_PAID);

	STATIC_ASSERT(WL_RXS_VHTF_STBC ==
		IEEE80211_RADIOTAP_VHT_STBC);
	STATIC_ASSERT(WL_RXS_VHTF_TXOP_PS ==
		IEEE80211_RADIOTAP_VHT_TXOP_PS);
	STATIC_ASSERT(WL_RXS_VHTF_SGI ==
		IEEE80211_RADIOTAP_VHT_SGI);
	STATIC_ASSERT(WL_RXS_VHTF_SGI_NSYM_DA ==
		IEEE80211_RADIOTAP_VHT_SGI_NSYM_DA);
	STATIC_ASSERT(WL_RXS_VHTF_LDPC_EXTRA ==
		IEEE80211_RADIOTAP_VHT_LDPC_EXTRA);
	STATIC_ASSERT(WL_RXS_VHTF_BF ==
		IEEE80211_RADIOTAP_VHT_BF);

	rtvht->vht_flags = (uint8)HTOL16(rxsts->vhtflags);

	STATIC_ASSERT(WL_RXS_VHT_BW_20 ==
		IEEE80211_RADIOTAP_VHT_BW_20);
	STATIC_ASSERT(WL_RXS_VHT_BW_40 ==
		IEEE80211_RADIOTAP_VHT_BW_40);
	STATIC_ASSERT(WL_RXS_VHT_BW_20L ==
		IEEE80211_RADIOTAP_VHT_BW_20L);
	STATIC_ASSERT(WL_RXS_VHT_BW_20U ==
		IEEE80211_RADIOTAP_VHT_BW_20U);
	STATIC_ASSERT(WL_RXS_VHT_BW_80 ==
		IEEE80211_RADIOTAP_VHT_BW_80);
	STATIC_ASSERT(WL_RXS_VHT_BW_40L ==
		IEEE80211_RADIOTAP_VHT_BW_40L);
	STATIC_ASSERT(WL_RXS_VHT_BW_40U ==
		IEEE80211_RADIOTAP_VHT_BW_40U);
	STATIC_ASSERT(WL_RXS_VHT_BW_20LL ==
		IEEE80211_RADIOTAP_VHT_BW_20LL);
	STATIC_ASSERT(WL_RXS_VHT_BW_20LU ==
		IEEE80211_RADIOTAP_VHT_BW_20LU);
	STATIC_ASSERT(WL_RXS_VHT_BW_20UL ==
		IEEE80211_RADIOTAP_VHT_BW_20UL);
	STATIC_ASSERT(WL_RXS_VHT_BW_20UU ==
		IEEE80211_RADIOTAP_VHT_BW_20UU);

	rtvht->vht_bw = rxsts->bw;

	rtvht->vht_mcs_nss[0] = (rxsts->mcs << 4) |
		(rxsts->nss & IEEE80211_RADIOTAP_VHT_NSS);
	rtvht->vht_mcs_nss[1] = 0;
	rtvht->vht_mcs_nss[2] = 0;
	rtvht->vht_mcs_nss[3] = 0;

	STATIC_ASSERT(WL_RXS_VHTF_CODING_LDCP ==
		IEEE80211_RADIOTAP_VHT_CODING_LDPC);

	rtvht->vht_coding = rxsts->coding;
	rtvht->vht_group_id = rxsts->gid;
	rtvht->vht_partial_aid = HTOL16(rxsts->aid);

	rtvht->ampdu_flags = 0;
	rtvht->ampdu_delim_crc = 0;

	rtvht->ampdu_ref_num = rxsts->ampdu_counter;

	if (!(rxsts->nfrmtype & WL_RXS_NFRM_AMPDU_FIRST) &&
		!(rxsts->nfrmtype & WL_RXS_NFRM_AMPDU_SUB))
		rtvht->ampdu_flags |= IEEE80211_RADIOTAP_AMPDU_IS_LAST;

	return 0;
}
#endif /* WL_RADIOTAP */
