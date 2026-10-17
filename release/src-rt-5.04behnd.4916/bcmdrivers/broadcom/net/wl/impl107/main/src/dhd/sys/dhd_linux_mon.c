/*
 * Broadcom Dongle Host Driver (DHD), Linux monitor network interface
 *
 * Copyright (C) 2025, Broadcom. All Rights Reserved.
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
 *
 *
 * <<Broadcom-WL-IPTag/Open:>>
 *
 * $Id: dhd_linux_mon.c 821234 2023-02-06 14:16:52Z $
 */

#include <osl.h>
#include <linux/string.h>
#include <linux/module.h>
#include <linux/netdevice.h>
#include <linux/etherdevice.h>
#include <linux/if_arp.h>
#include <linux/ieee80211.h>
#include <linux/rtnetlink.h>
#include <802.1d.h>

#include <wlioctl.h>
#include <bcmutils.h>
#include <bcm_pcap.h>
#include <dhd_dbg.h>
#include <dngl_stats.h>
#include <dhd.h>
#include <dhd_linux_wq.h>
#include <dhd_macdbg.h>
#ifdef WL_MONITOR
#include <bcmwifi_monitor.h>
#include <bcmendian.h>
#include <d11_pub.h>
#endif /* WL_MONITOR */

int dhd_pcap_add(const char *name, struct net_device **new_ndev, dhd_pub_t *pub);
int dhd_pcap_del(dhd_pub_t *pub);
int dhd_pcap_init(dhd_pub_t *pub);
int dhd_pcap_uninit(dhd_pub_t *pub);

/**
 * Local declarations and definitions (not exposed)
 */
#define PCAP_PRINT(format, ...) printk("DHD-PCAP: %s " format, __func__, ##__VA_ARGS__)
#define PCAP_TRACE PCAP_PRINT

#define D11REV_GE(var, val)     ((var) >= (val))

static struct net_device* lookup_real_netdev(dhd_pub_t *pub);
static pcap_interface* ndev_to_pcapif(struct net_device *ndev);
static int dhd_pcap_if_open(struct net_device *ndev);
static int dhd_pcap_if_stop(struct net_device *ndev);
static int dhd_pcap_if_subif_start_xmit(struct sk_buff *skb, struct net_device *ndev);
static void dhd_pcap_if_set_multicast_list(struct net_device *ndev);
static int dhd_pcap_if_change_mac(struct net_device *ndev, void *addr);

static const struct net_device_ops dhd_pcap_if_ops = {
	.ndo_open		= dhd_pcap_if_open,
	.ndo_stop		= dhd_pcap_if_stop,
	.ndo_start_xmit		= dhd_pcap_if_subif_start_xmit,
	.ndo_set_rx_mode	= dhd_pcap_if_set_multicast_list,
	.ndo_set_mac_address	= dhd_pcap_if_change_mac,
};

/**
 * Local static function defintions
 */

#ifdef WL_MONITOR

/* This function is based off of wlc_recv_compute_rspec().
 * Given a PLCP, it calculates the rate of a received frame and returns it as a ratespec.
 */
static ratespec_t
dhd_recv_compute_rspec(dhd_pub_t *dhdp, d11rxhdr_t *rxhdr, uint8 *plcp)
{
	uint corerev = dhdp->monitor_info->corerev;
	ratespec_t rspec;
	uint8 ftfmt;
	uint8 ft, fmt;

	BCM_REFERENCE(corerev);

	/* Set a default valid rspec */
	rspec = OFDM_RSPEC(WLC_RATE_6M);

	ftfmt = ((rxhdr->ge129.RxStatus2) >> 8);

	ft = ftfmt & PRXS0_FT_MASK_GE129;
	fmt = (ftfmt & PRXS0_FMT_MASK_GE129) >> PRXS0_FMT_SHIFT_GE129;

	/* HT/VHT/HE-SIG-A start from plcp[4] in rev128 */
	plcp += D11_PHY_RXPLCP_OFF;

	/* HETB-SIG-A doesn't include rate info pass it using SIG-B */
	if ((ft == FT_HE || ft == FT_EHT) && fmt == HE_EHT_FMT_TB) {
		uint16 phy_ulrtinfo = HETB_ULRTINFO(rxhdr, corerev);
		plcp[6] = phy_ulrtinfo & 0xff;
		plcp[7] = (phy_ulrtinfo & 0xff00) >> 8;
	}

	if (wf_plcp_to_rspec(ft, fmt, plcp, &rspec) == FALSE) {
		DHD_ERROR(("%s() rspec calc failed\n", __FUNCTION__));
		/* Return a valid rspec if not a debug/assert build */
		rspec = OFDM_RSPEC(WLC_RATE_6M);
	} else if (ft == FT_EHT && RSPEC_GE_VHT_MCS(rspec) >= 14 &&
		RSPEC_GE_VHT_NSS(rspec) > 1) {
		/* DCM/DUP support 1 spatial stream only */
		DHD_TRACE(("%s: EHT format %d with mcs %d nss %d, where nss should be 1\n",
				__FUNCTION__, fmt,
				RSPEC_GE_VHT_MCS(rspec), RSPEC_GE_VHT_NSS(rspec)));
		/* The combination values for MCS and Nss is incorrect and the error
		 * handling sets Rx Nss to 1, assuming MCS is correct
		 */
		rspec &= ~WL_RSPEC_GE_VHT_NSS_MASK;
		rspec |= (1 << WL_RSPEC_GE_VHT_NSS_SHIFT);
	}

	return (rspec);
}

/*
 * This function is based on dhd_linux.c::dhd_rx_mon_pkt to setup all the data structres needed
 * to call bcmwifi_monitor()
 */
static int
dhd_pcap_rx_mon(dhd_pub_t *dhdp, void *pcap_pkt, uint8 type)
{
	wl_phyextract_t phyextract;
	monitor_pkt_info_t pkt_info = {0};
	uint16 len = 0;
	uint8 dma_flags;
	d11rxhdr_t *rxhdr;
	d11rxhdr_t *tmprxhdr;
	int16 offset = 0;
	uint16 pkt_type;
	ratespec_t rspec;
	uint8 *plcp;

	if (!dhdp->monitor_skb) {
		dhdp->monitor_skb = (struct sk_buff *)pcap_pkt;
	}

	/* Make sure we have enough room to add in another possibly max sized amsdu */
	if ((skb_tailroom(dhdp->monitor_skb) < PCAP_PKT_BUFSZ) && (type != PCAP_PKT_RX_COPY)) {
		if (dhdp->monitor_skb != pcap_pkt) {
			PKTFREE(dhdp->osh, dhdp->monitor_skb, FALSE);
		}
		PKTFREE(dhdp->osh, pcap_pkt, FALSE);
		dhdp->monitor_skb = NULL;
		dhdp->monitor_info->amsdu_len = 0;
		dhdp->monitor_info->amsdu_pkt = NULL;
		return -1;
	}

	/*
	 ************
	 * pkt_info fields
	 ************
	 */
	pkt_info.marker = 0; /* Doesn't look like this is used */

	if (PCAP_SKB_PKTFLAGS((struct sk_buff *)pcap_pkt) & PCAP_PKTFLAG_TRUNC) {
		pkt_info.flags |= PCAP_PKTFLAG_TRUNC;
	}
	if (PCAP_SKB_PKTFLAGS((struct sk_buff *)pcap_pkt) & PCAP_PKTFLAG_FMPDU_IN_AMPDU) {
		pkt_info.flags |= PCAP_PKTFLAG_FMPDU_IN_AMPDU;
	}
	rxhdr = (d11rxhdr_t *)PKTDATA(dhdp->osh, pcap_pkt);

	/*
	 ************
	 * phyextract fields
	 *
	 ************
	 */
	/* Some of the phyextract needs to be added by wl, see wl_rte.c::wl_monitor(),
	 * maybe can be pulled for rx status headers, TBD
	 */
	phyextract.rssi = (rxhdr->ge129.AvbRxTimeL >> 8);	/* Bits 15:8 */
	phyextract.snr = 0;	/* phy_noise_avg(wlc->pi) */

	/* WL_RXS_PREAMBLE_SHORT or WL_RXS_PREAMBLE_LONG, for now just use short */
	phyextract.preamble = WL_RXS_PREAMBLE_SHORT;

	/*
	 * Assume buffers have (hw + ucode hdrs) + physts, but for
	 * RX_COPY if BCM_PCAP_PHYSTS is not define, then there will
	 * NOT be a PhyRx status header (hwrxoff will be reset below in that case)
	 */
	phyextract.hwrxoff = sizeof(d11rxhdr_t) +
			D11PHYRXSTS_LEN(dhdp->monitor_info->corerev);

	/* In the case of RX_COPY we need to do some fix-ups on the rxhdr. The way the
	 * RX_COPY code handles AMSDUs is that it re-aggregates the AMSDU before sending
	 * to the radiotap.  So, from the radiotap perspective these should not be handled
	 * as AMSDU so :
	 *
	 *   Clear the AMSDU (RXSS_AMSDU) bit in the heder.
	 *
	 * Additionally, when re-aggregating the AMSDU frame the RX_COPY does not
	 * include any pad betweeen the headers and the PLCP, so :
	 *
	 *   Clear the PLCP Pad Present bit (RXSS_PBPRES) in the header
	 *
	 * One last thing, since the RX_COPY code uses the HW header (rxhdr) from the
	 * first AMSDU which will indicate that the ucode header is not valid (dma_flags = 1)
	 * However, there is a valid ucode status as it was copied from the last AMSDU bu
	 * the RX_COPY code, so:
	 *
	 *   Set the dma_flags field to 0 which indicates a valid ucode status is present
	 *
	 * Once this cleanup is done, the rate info can be calculated.
	 */
	if (type == PCAP_PKT_RX_COPY) {
		uint16 rxhdr_val;

		/* Clear AMSDU and PAD flags */
		rxhdr_val = D11RXHDR_ACCESS_VAL(rxhdr, mrxs);
		rxhdr_val &= ~(RXSS_AMSDU_MASK | RXSS_PBPRES);

		D11RXHDR_ACCESS_VAL(rxhdr, mrxs) = rxhdr_val;

		rxhdr_val = D11RXHDR_ACCESS_VAL(rxhdr, dma_flags);

		/* Indicate that the ucode status is valid */
		rxhdr_val &= ~RXS_DMA_FLAGS_RXS_TYPE_MASK;
		rxhdr_val |= ((RXS_MAC_UCODE & RXS_DMA_FLAGS_RXS_TYPE_MASK) <<
				RXS_DMA_FLAGS_RXS_TYPE_SHIFT);

		D11RXHDR_ACCESS_VAL(rxhdr, dma_flags) = rxhdr_val;

#ifndef BCM_PCAP_PHYSTS
		/*
		 * For Rx Copy there is no PhyRx status header in the pcap buffer so
		 * hwrxoff needs to be just sizeof(d11rxhdr_t)
		 *
		 */
		phyextract.hwrxoff = sizeof(d11rxhdr_t);
#endif /* BCM_PCAP_PHYSTS */
	}

	switch (type) {
		case PCAP_PKT_RX_PROMISC_MULTI_FIRST:
			/* This is the FIRST in a multi-buffer aggregation so we expect monitor_skb
			 * to be the same as pcap_pkt.  If it is not, then we must have been in the
			 * process of aggregating a prior multi-buffer packet.  Flush out the old
			 * one and carry on with the new one. BTW, this -should- never happen.
			 */
			if (dhdp->monitor_skb != pcap_pkt) {
				/* previous aggregation was interrupted. start over */
				DHD_PKT_PCAP(("%s() MULTI-BUF unexpected FIRST\n", __FUNCTION__));
				PKTFREE(dhdp->osh, dhdp->monitor_skb, FALSE);
				dhdp->monitor_skb = pcap_pkt;
				dhdp->monitor_info->amsdu_len = 0;
				dhdp->monitor_info->amsdu_pkt = NULL;
				dhdp->pcap.pcap_multi_err++;
			}

			len = pcap_handle_multi_rx_promisc(dhdp->monitor_info, RXS_AMSDU_FIRST,
					PKTDATA(dhdp->osh, pcap_pkt), &phyextract, rxhdr, &offset,
					PKTLEN(dhdp->osh, pcap_pkt));

			/* If len is 0, flush this buffer (this should never happen either) */
			if (len == 0) {
				DHD_PKT_PCAP(("%s() MULTI-BUF error handling FIRST\n",
					__FUNCTION__));
				if (dhdp->monitor_skb != NULL) {
					PKTFREE(dhdp->osh, dhdp->monitor_skb, FALSE);
				}
				dhdp->monitor_skb = NULL;
				dhdp->monitor_info->amsdu_len = 0;
				dhdp->monitor_info->amsdu_pkt = NULL;
				dhdp->pcap.pcap_multi_err++;
				return -1;
			} else {
				return len;
			}
			break;

		case PCAP_PKT_RX_PROMISC_MULTI_INTRM:
			/* Check that the aggregation buffer has room to hold this data */
			if (skb_tailroom(dhdp->monitor_skb) < PKTLEN(dhdp->osh, pcap_pkt)) {
				len = 0;
			} else {

				len = pcap_handle_multi_rx_promisc(dhdp->monitor_info,
					RXS_AMSDU_INTERMEDIATE, PKTDATA(dhdp->osh, pcap_pkt),
					&phyextract, rxhdr, &offset, PKTLEN(dhdp->osh, pcap_pkt));
			}

			/* If something went wrong, flush this and the aggregation buffer */
			if (len == 0) {
				DHD_PKT_PCAP(("%s() MULTI-BUF error handling INTRM\n",
					__FUNCTION__));
				if (pcap_pkt != dhdp->monitor_skb) {
					PKTFREE(dhdp->osh, pcap_pkt, FALSE);
				}
				PKTFREE(dhdp->osh, dhdp->monitor_skb, FALSE);
				dhdp->monitor_skb = NULL;
				dhdp->monitor_info->amsdu_len = 0;
				dhdp->monitor_info->amsdu_pkt = NULL;
				dhdp->pcap.pcap_multi_err++;
				return -1;
			} else {
				skb_put(dhdp->monitor_skb, len);
				PKTFREE(dhdp->osh, pcap_pkt, FALSE);
				if (dhdp->monitor_skb == pcap_pkt) {
					dhdp->monitor_skb = NULL;
				}
				return len;
			}
			break;

		case PCAP_PKT_RX_PROMISC_MULTI_LAST:
			/* Since this is the last in a multi-buffer, we expect the rxstatus
			 * to be at the beginning of the aggregated pkt
			 */
			if (dhdp->monitor_info->amsdu_pkt == NULL) {
				/* If this packet is out of order, due to SKB alloc failure,
				 * we need to discard this out of sequence MSDU.
				 */
				if (dhdp->monitor_skb != pcap_pkt) {
					PKTFREE(dhdp->osh, dhdp->monitor_skb, FALSE);
				}
				dhdp->monitor_skb = NULL;
				PKTFREE(dhdp->osh, pcap_pkt, FALSE);
				dhdp->monitor_info->amsdu_pkt = NULL;
				dhdp->monitor_info->amsdu_len = 0;
				return -1;
			}

			rxhdr = (d11rxhdr_t *)dhdp->monitor_info->amsdu_pkt;
			plcp = (uint8 *)(((uint8 *)rxhdr) + phyextract.hwrxoff);
			rspec = dhd_recv_compute_rspec(dhdp, rxhdr, plcp);

			phyextract.rspec = rspec;

			/* Check that the aggregation buffer has room to hold this data */
			if (skb_tailroom(dhdp->monitor_skb) < PKTLEN(dhdp->osh, pcap_pkt)) {
				len = 0;
			} else {
				len = pcap_handle_multi_rx_promisc(dhdp->monitor_info,
						RXS_AMSDU_LAST,
					PKTDATA(dhdp->osh, pcap_pkt), &phyextract, rxhdr, &offset,
					PKTLEN(dhdp->osh, pcap_pkt));
			}

			/* If something went wrong, flush this and the aggregation buffer */
			if (len == 0) {
				DHD_PKT_PCAP(("%s() MULTI-BUF error handling INTRM\n",
					__FUNCTION__));
				if (pcap_pkt != dhdp->monitor_skb) {
					PKTFREE(dhdp->osh, pcap_pkt, FALSE);
				}
				PKTFREE(dhdp->osh, dhdp->monitor_skb, FALSE);
				dhdp->monitor_skb = NULL;
				dhdp->monitor_info->amsdu_len = 0;
				dhdp->monitor_info->amsdu_pkt = NULL;
				dhdp->pcap.pcap_multi_err++;
				return -1;
			} else {
				skb_put(dhdp->monitor_skb, len);

				/* Adjust the data pointer for radiotap header */
				if (offset)  {
					if (offset > 0) {
						skb_push((struct sk_buff *)dhdp->monitor_skb,
								offset);
					}
					else {
						skb_pull((struct sk_buff *)dhdp->monitor_skb,
								-(offset));
					}
				}
				PKTFREE(dhdp->osh, pcap_pkt, FALSE);
				if (dhdp->monitor_skb == pcap_pkt) {
					dhdp->monitor_skb = NULL;
				}
				dhdp->pcap.pcap_multi_succ++;
				return 0;
			}
			break;

		default:
			/* Not a multi-buffer aggregation type, carry on with other processing */
			break;
	}

	/* Pick up a pointer to the PLCP in the Rx status header area */
	plcp = (uint8 *)(((uint8 *)rxhdr) + phyextract.hwrxoff + RXHDR_GET_PAD_LEN(rxhdr));

	rspec = OFDM_RSPEC(WLC_RATE_6M);

	/* If we have a valid ucode status, we know the frame type and can compute the
	 * rate from the plcp (this should always be the case except for first/middle
	 * amsdus).
	 * For last amsdus, use the plcp that was received with the first amsdu and is
	 * at the beginning of the amsdu reassembly buffer (monitor_info->amsdu_pkt)
	 */
	if (RXS_GE129_VALID_UCODE_STS(rxhdr, dhdp->monitor_info->corerev)) {
		if (RXHDR_GET_AMSDU(rxhdr) &&
			(RXHDR_GET_AGG_TYPE(rxhdr) == RXS_AMSDU_LAST)) {
			if (dhdp->monitor_info->amsdu_pkt) {
				/* Get a pointer to the rxsts header in the amsud reassem buffer,
				 * then offset to where the PLCP is
				 */
				tmprxhdr = (d11rxhdr_t *)dhdp->monitor_info->amsdu_pkt;
				plcp = (uint8 *)(((uint8 *)tmprxhdr) + phyextract.hwrxoff +
						RXHDR_GET_PAD_LEN(tmprxhdr));
				rspec = dhd_recv_compute_rspec(dhdp, rxhdr, plcp);
			} else {
				DHD_PKT_PCAP(("%s() : Unexpected NULL amsdu_pkt, %d %d!!!!\n",
						__FUNCTION__,
						dhdp->monitor_info->amsdu_len,
						dhdp->monitor_info->headroom));
			}

		} else {
			rspec = dhd_recv_compute_rspec(dhdp, rxhdr, plcp);
		}
	}

	phyextract.rspec = rspec;

	dma_flags = 0xff;

	len = bcmwifi_monitor(dhdp->monitor_info, &pkt_info, PKTDATA(dhdp->osh, pcap_pkt),
			PKTLEN(dhdp->osh, pcap_pkt),  PKTDATA(dhdp->osh, dhdp->monitor_skb),
			&offset, &pkt_type, dma_flags, &phyextract, rxhdr);

	if (pkt_type == MON_PKT_AMSDU_ERROR) {
		if (dhdp->monitor_skb != pcap_pkt) {
			PKTFREE(dhdp->osh, dhdp->monitor_skb, FALSE);
			dhdp->monitor_info->amsdu_pkt = NULL;
			dhdp->monitor_skb = pcap_pkt;
			len = bcmwifi_monitor(dhdp->monitor_info, &pkt_info,
					PKTDATA(dhdp->osh, pcap_pkt), PKTLEN(dhdp->osh, pcap_pkt),
					PKTDATA(dhdp->osh, dhdp->monitor_skb),
				&offset, &pkt_type, dma_flags, &phyextract, rxhdr);
			if (pkt_type == MON_PKT_AMSDU_ERROR) {
				PKTFREE(dhdp->osh, pcap_pkt, FALSE);
				dhdp->monitor_skb = NULL;
				return -1;
			}
		}
		else {
			PKTFREE(dhdp->osh, dhdp->monitor_skb, FALSE);
			dhdp->monitor_info->amsdu_pkt = NULL;
			dhdp->monitor_skb = NULL;
			return -1;
		}
	}

	if (pkt_type == MON_PKT_AMSDU_INTERMEDIATE || pkt_type == MON_PKT_AMSDU_LAST) {
		PKTFREE(dhdp->osh, pcap_pkt, FALSE);
		if (dhdp->monitor_skb == pcap_pkt) {
			dhdp->monitor_skb = NULL;
			return -1;
		}
		if (skb_tailroom(dhdp->monitor_skb) >= len)
		{
			skb_put(dhdp->monitor_skb, len);
		}
	}

	if (pkt_type == MON_PKT_AMSDU_FIRST || pkt_type == MON_PKT_AMSDU_INTERMEDIATE) {
		return -1;
	}
	if (offset)  {
		if (offset > 0) {
			skb_push((struct sk_buff *)dhdp->monitor_skb, offset);
		}
		else {
			skb_pull((struct sk_buff *)dhdp->monitor_skb, -(offset));
		}
	}

	return 0;
}

#ifdef BCM_PCAP
static int
dhd_radiotap_tx_channel_frequency(pcap_tx_hdr_t *txsts)
{
	switch (CHSPEC_BAND(txsts->chanspec)) {
		case WL_CHANSPEC_BAND_2G:
			return wf_channel2mhz(wf_chspec_ctlchan(txsts->chanspec),
					WF_CHAN_FACTOR_2_4_G);
			break;
		case WL_CHANSPEC_BAND_5G:
			return wf_channel2mhz(wf_chspec_ctlchan(txsts->chanspec),
					WF_CHAN_FACTOR_5_G);
			break;
		case WL_CHANSPEC_BAND_6G:
			return wf_channel2mhz(wf_chspec_ctlchan(txsts->chanspec),
					WF_CHAN_FACTOR_6_G);
			break;
		default:
			PCAP_PRINT("%s() bad BAND 0x%x\n", __FUNCTION__,
					CHSPEC_BAND(txsts->chanspec));
			return 0;
			break;
	}
}

static uint16
dhd_radiotap_tx_channel_flags(pcap_tx_hdr_t *txsts)
{
	switch (CHSPEC_BAND(txsts->chanspec)) {
		case WL_CHANSPEC_BAND_2G:
			return (IEEE80211_CHAN_2GHZ | IEEE80211_CHAN_DYN);
			break;
		case WL_CHANSPEC_BAND_5G:
			return (IEEE80211_CHAN_5GHZ | IEEE80211_CHAN_OFDM);
			break;
		case WL_CHANSPEC_BAND_6G:
			/* no radiotap flag defined for 6G */
			return IEEE80211_CHAN_OFDM;
			break;
		default:
			PCAP_PRINT("%s() bad BAND 0x%x\n", __FUNCTION__,
					CHSPEC_BAND(txsts->chanspec));
			return 0;
			break;
	}
}

/**
 * Lookup table for Frametypes [LEGACY (CCK / OFDM) /HT/VHT/HE/EHT]
 */
const uint8 dhd_rspec_enc2ft_tbl[] = {FT_LEGACY, FT_HT, FT_VHT, FT_HE, FT_EHT};

#define WLC_RSPEC_ENC2FT(rspec)	((RSPEC_ISLEGACY(rspec)) ? \
				 (RSPEC_ISOFDM(rspec) ? FT_OFDM : FT_CCK) : \
				 dhd_rspec_enc2ft_tbl[RSPEC_ENCODE(rspec)])

static uint8
dhd_radiotap_tx_flags(struct dot11_header *mac_header, pcap_tx_hdr_t *txsts)
{
	uint8 flags;
	uint16 fc;
	uint8 frametype;	/**< 802.11 A/B/G /N */

	frametype = WLC_RSPEC_ENC2FT(txsts->rspec);

	fc = ltoh16(mac_header->fc);

	flags = IEEE80211_RADIOTAP_F_FCS;

	if (txsts->rspec & WL_RSPEC_SHORT_PREAMBLE) {
		flags |= IEEE80211_RADIOTAP_F_SHORTPRE;
	}

	if (fc & FC_WEP) {
		flags |= IEEE80211_RADIOTAP_F_WEP;
	}

	if (fc & FC_MOREFRAG) {
		flags |= IEEE80211_RADIOTAP_F_FRAG;
	}

	return flags;
}

uint
dhd_radiotap_tx_legacy(struct dot11_header *mac_header, pcap_tx_hdr_t *txsts,
		wl_radiotap_tx_legacy_t *rtl)
{
	uint16 channel_frequency;
	uint16 channel_flags;
	uint8 flags;
	uint16 rtap_len;

	rtap_len = sizeof(wl_radiotap_tx_legacy_t);
	memset(rtl, 0, rtap_len);
	channel_frequency = (uint16)dhd_radiotap_tx_channel_frequency(txsts);
	channel_flags = dhd_radiotap_tx_channel_flags(txsts);
	flags = dhd_radiotap_tx_flags(mac_header, (pcap_tx_hdr_t *)txsts);
	flags &= ~IEEE80211_RADIOTAP_F_FCS;
	rtl->ieee_radiotap.it_version = 0;
	rtl->ieee_radiotap.it_pad = 0;
	rtl->ieee_radiotap.it_len = HTOL16(rtap_len);
	rtl->ieee_radiotap.it_present = HTOL32(WL_RADIOTAP_PRESENT_TX_LEGACY);
	rtl->rt_common.tsft_h = HTOL32(txsts->tsf_hi);
	rtl->rt_common.tsft_l = HTOL32(txsts->tsf_lo);
	rtl->rt_common.flags = flags;
	rtl->rt_common.u.rate = (uint8)txsts->datarate;
	rtl->rt_common.channel_freq = HTOL16(channel_frequency);
	rtl->rt_common.channel_flags = HTOL16(channel_flags);
	rtl->rt_common.antenna = 0;
	memcpy(rtl->vend_oui, BRCM_OUI, DOT11_OUI_LEN);
	rtl->vend_skip_len = WL_RADIOTAP_LEGACY_SKIP_LEN;
	rtl->vend_sns = 0;

	return 0;
}

#define MCS_INVALID	0xFF
#define RSPEC2BW(rspec) ((WL_RSPEC_BW_MASK & (rspec)) >> WL_RSPEC_BW_SHIFT)

/* Based on wlc_ratespec_mcs() */
uint8
dhd_ratespec_mcs(ratespec_t rspec)
{
	uint8 mcs;

	if (RSPEC_GE_VHT(rspec)) {
		mcs = (uint8)RSPEC_GE_VHT_MCS(rspec);
	} else if (RSPEC_ISHT(rspec)) {
		if (RSPEC_HT_MCS(rspec) == 32) {
			mcs = 32;
		} else {
			mcs = (uint8)RSPEC_HT_1SS_MCS(rspec); /* a 0-7 SS MCS index */
		}
	} else {
		mcs = MCS_INVALID;
	}

	return mcs;
}

uint8
dhd_ratespec_bw(ratespec_t rspec)
{
	const uint8 rspec2bw[] = {0, 0, 1, 4, 11, 0, 0, 0};

	/* Convert RSPEC BW values to radiotap BW values */
	return (rspec2bw[RSPEC2BW(rspec)]);
}

/* Based on wlc_ratespec_nss() */
uint
dhd_ratespec_nss(ratespec_t rspec)
{
	int Nss;

	if (RSPEC_GE_VHT(rspec)) {
		Nss = RSPEC_GE_VHT_NSS(rspec);
	} else if (RSPEC_ISHT(rspec)) {
		uint mcs = RSPEC_HT_MCS(rspec);

		ASSERT(mcs <= 32);
		if (mcs == 32) {
			Nss = 1;
		} else {
			Nss = 1 + (mcs / 8);
		}
	} else {
		/* Legacy rates are all Nss = 1, just add the expansion */
		Nss = 1;
	}

	ASSERT(Nss != 0);
	return Nss;
}

uint
dhd_radiotap_tx_ht(struct dot11_header *mac_header, pcap_tx_hdr_t *txsts, wl_radiotap_ht_t *rtht)
{
	uint16 channel_frequency;
	uint16 channel_flags;
	uint8 flags;
	uint16 rtap_len;
	uint32 htflags;

	rtap_len = sizeof(wl_radiotap_ht_t);
	channel_frequency = (uint16)dhd_radiotap_tx_channel_frequency(txsts);
	channel_flags = dhd_radiotap_tx_channel_flags(txsts);
	flags = dhd_radiotap_tx_flags(mac_header, (pcap_tx_hdr_t *)txsts);
	flags &= ~IEEE80211_RADIOTAP_F_FCS;
	rtht->ieee_radiotap.it_version = 0;
	rtht->ieee_radiotap.it_pad = 0;
	rtht->ieee_radiotap.it_len = HTOL16(rtap_len);
	rtht->ieee_radiotap.it_present = HTOL32(WL_RADIOTAP_PRESENT_HT);

	rtht->rt_common.tsft_h = HTOL32(txsts->tsf_hi);
	rtht->rt_common.tsft_l = HTOL32(txsts->tsf_lo);
	rtht->rt_common.flags = flags;
	rtht->rt_common.u.pad1 = 0;
	rtht->rt_common.channel_freq = HTOL16(channel_frequency);
	rtht->rt_common.channel_flags = HTOL16(channel_flags);
	rtht->rt_common.antenna = 0;

	/* add standard MCS */
	rtht->mcs_known = (IEEE80211_RADIOTAP_MCS_HAVE_BW |
		IEEE80211_RADIOTAP_MCS_HAVE_MCS |
		IEEE80211_RADIOTAP_MCS_HAVE_GI |
		IEEE80211_RADIOTAP_MCS_HAVE_FEC |
		IEEE80211_RADIOTAP_MCS_HAVE_FMT);

	rtht->mcs_flags = 0;

	htflags = 0;

	if (CHSPEC_BW_GE(txsts->chanspec, WL_CHANSPEC_BW_40)) {
		uint32 bw = RSPEC_BW(txsts->rspec);

		if (bw == WL_RSPEC_BW_20MHZ) {
			if (CHSPEC_CTL_SB(txsts->chanspec) == WL_CHANSPEC_CTL_SB_L) {
				htflags = WL_RXS_HTF_20L;
			} else {
				htflags = WL_RXS_HTF_20U;
			}
		} else if (bw == WL_RSPEC_BW_40MHZ) {
			htflags = WL_RXS_HTF_40;
		}
	}

	if (RSPEC_ISSGI(txsts->rspec))
		htflags |= WL_RXS_HTF_SGI;
	if (RSPEC_ISLDPC(txsts->rspec))
		htflags |= WL_RXS_HTF_LDPC;

	switch (htflags & WL_RXS_HTF_BW_MASK) {
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

	if (htflags & WL_RXS_HTF_SGI) {
		rtht->mcs_flags |= IEEE80211_RADIOTAP_MCS_SGI;
	}

	if (htflags & WL_RXS_HTF_LDPC) {
		rtht->mcs_flags |= IEEE80211_RADIOTAP_MCS_FEC_LDPC;
	}

	rtht->mcs_index = dhd_ratespec_mcs(txsts->rspec);

	return 0;
}

uint
dhd_radiotap_tx_vht(struct dot11_header *mac_header, pcap_tx_hdr_t *txsts, wl_radiotap_vht_t *rtvht)
{
	uint16 channel_frequency;
	uint16 channel_flags;
	uint8 flags;
	uint16 rtap_len;
	uint8 mcs;
	uint8 nss;
	uint8 id;

	rtap_len = sizeof(wl_radiotap_vht_t);
	channel_frequency = (uint16)dhd_radiotap_tx_channel_frequency(txsts);
	channel_flags = dhd_radiotap_tx_channel_flags(txsts);

	memset(rtvht, 0, sizeof(wl_radiotap_vht_t));
	flags = dhd_radiotap_tx_flags(mac_header, (pcap_tx_hdr_t *)txsts);
	flags &= ~IEEE80211_RADIOTAP_F_FCS;
	rtvht->ieee_radiotap.it_version = 0;
	rtvht->ieee_radiotap.it_pad = 0;
	rtvht->ieee_radiotap.it_len = HTOL16(rtap_len);
	rtvht->ieee_radiotap.it_present = HTOL32(WL_RADIOTAP_PRESENT_VHT);

	rtvht->rt_common.tsft_h = HTOL32(txsts->tsf_hi);
	rtvht->rt_common.tsft_l = HTOL32(txsts->tsf_lo);
	rtvht->rt_common.flags = flags;
	rtvht->rt_common.u.pad1 = 0;
	rtvht->rt_common.channel_freq = HTOL16(channel_frequency);
	rtvht->rt_common.channel_flags = HTOL16(channel_flags);
	rtvht->rt_common.antenna = 0;

	if (txsts->tx_type == PCAP_MU_VHT) {
		id = txsts->mu_user_id;
		mcs = RSPEC_GE_VHT_MCS(txsts->mu_nss_mcs);
	        nss = RSPEC_GE_VHT_NSS(txsts->mu_nss_mcs);

		/* Check that id is within the index range of vht_mcs_nss[] */
		if (id < sizeof(rtvht->vht_mcs_nss)) {
			rtvht->vht_mcs_nss[id] = ((nss & IEEE80211_RADIOTAP_VHT_NSS) |
					((mcs << IEEE80211_RADIOTAP_VHT_MCS_SHIFT) &
					IEEE80211_RADIOTAP_VHT_MCS));
		} else {
			/* id is out-of-range, clear the vht_mcs_nss array */
			bzero(rtvht->vht_mcs_nss, sizeof(rtvht->vht_mcs_nss));
		}

		/* gi */
		if (RSPEC_ISSGI(txsts->mu_nss_mcs)) {
			rtvht->vht_known |= HTOL16(IEEE80211_RADIOTAP_VHT_HAVE_GI);
			rtvht->vht_flags |= IEEE80211_RADIOTAP_VHT_SGI;
		}

		/* group id */
		rtvht->vht_known |= HTOL16(IEEE80211_RADIOTAP_VHT_HAVE_GID);
		rtvht->vht_group_id = txsts->gid;
	} else { /* SU user */
		rtvht->vht_mcs_nss[0] = (dhd_ratespec_mcs(txsts->rspec) <<4) |
				(dhd_ratespec_nss(txsts->rspec) & IEEE80211_RADIOTAP_VHT_NSS);
		rtvht->vht_mcs_nss[1] = 0;
		rtvht->vht_mcs_nss[2] = 0;
		rtvht->vht_mcs_nss[3] = 0;

		/* gi */
		if (RSPEC_ISSGI(txsts->rspec)) {
			rtvht->vht_known |= HTOL16(IEEE80211_RADIOTAP_VHT_HAVE_GI);
			rtvht->vht_flags |= IEEE80211_RADIOTAP_VHT_SGI;
		}

		/* band width */
		rtvht->vht_known |= HTOL16(IEEE80211_RADIOTAP_VHT_HAVE_BW);
		rtvht->vht_bw = dhd_ratespec_bw(txsts->rspec);
	}

	return 0;
}

uint
dhd_radiotap_tx_he(struct dot11_header *mac_header, pcap_tx_hdr_t *txsts, wl_radiotap_he_t *rthe)
{
	uint16 channel_frequency = 0;
	uint16 channel_flags = 0;
	uint8 flags = 0;
	uint8 mcs = 0;

	channel_frequency = (uint16)dhd_radiotap_tx_channel_frequency(txsts);
	channel_flags = dhd_radiotap_tx_channel_flags(txsts);

	memset(rthe, 0, sizeof(wl_radiotap_he_t));
	flags = dhd_radiotap_tx_flags(mac_header, (pcap_tx_hdr_t *)txsts);
	flags &= ~IEEE80211_RADIOTAP_F_FCS;
	rthe->ieee_radiotap.it_version = 0;
	rthe->ieee_radiotap.it_pad = 0;

	rthe->ieee_radiotap.it_len = HTOL16(sizeof(wl_radiotap_he_t));
	rthe->ieee_radiotap.it_present = HTOL32(WL_RADIOTAP_PRESENT_HE);

	rthe->rt_common.tsft_h = HTOL32(txsts->tsf_hi);
	rthe->rt_common.tsft_l = HTOL32(txsts->tsf_lo);
	rthe->rt_common.flags = flags;
	rthe->rt_common.u.pad1 = 0;
	rthe->rt_common.channel_freq = HTOL16(channel_frequency);
	rthe->rt_common.channel_flags = HTOL16(channel_flags);

	/*
	 * More HE fields in the Tx radiotap header (antenna, data1...6) need to be populated,
	 * but might need added fields in the pcap_tx_hdr_t in order to populate.
	 */
	rthe->rt_common.antenna = 0;
	if (txsts->tx_type == PCAP_MU_HE) {
		rthe->data1 = HTOL16((PCAP_HE_MU & IEEE80211_RADIOTAP_HE_PPDU_FMT_MASK) <<
				IEEE80211_RADIOTAP_HE_PPDU_FMT_SHIFT);
	}

	rthe->data1 |= HTOL16(IEEE80211_RADIOTAP_HE_HAVE_BSS_COLOR);
	rthe->data2 = 0;

	/* bss color */
	rthe->data3 = HTOL16(((txsts->bss_color & IEEE80211_RADIOTAP_HE_BSS_COLOR_MASK) <<
			IEEE80211_RADIOTAP_HE_BSS_COLOR_SHIFT));

	/* mcs */
	rthe->data1 |= HTOL16(IEEE80211_RADIOTAP_HE_HAVE_MCS);
	if (txsts->tx_type == PCAP_MU_HE) {
		mcs = RSPEC_GE_VHT_MCS(txsts->mu_nss_mcs);
	} else {
		mcs = RSPEC_GE_VHT_MCS(txsts->rspec);
	}
	rthe->data3 |= HTOL16(((mcs << IEEE80211_RADIOTAP_HE_MCS_SHIFT) &
			IEEE80211_RADIOTAP_HE_MCS_MASK));

	rthe->data4 = 0;
	rthe->data5 = 0;
	rthe->data6 = 0;

	return 0;
}

uint
dhd_radiotap_tx_eht(struct dot11_header *mac_header, pcap_tx_hdr_t *txsts, wl_radiotap_eht_t *rteht)
{
	ratespec_t rspec = txsts->rspec;
	uint16 channel_frequency = 0;
	uint16 channel_flags = 0;
	uint8 flags = 0;
	uint8 mcs = 0;
	uint8 nss = 0;
	uint8 gi = 0;

	channel_frequency = (uint16)dhd_radiotap_tx_channel_frequency(txsts);
	channel_flags = dhd_radiotap_tx_channel_flags(txsts);

	flags = dhd_radiotap_tx_flags(mac_header, (pcap_tx_hdr_t *)txsts);
	flags &= ~IEEE80211_RADIOTAP_F_FCS;

	memset(rteht, 0, sizeof(wl_radiotap_eht_t));
	/* IEEE80211 Radiotap Header */
	rteht->ieee_radiotap.it_version = 0;
	rteht->ieee_radiotap.it_pad = 0;
	rteht->ieee_radiotap.it_len = HTOL16(sizeof(wl_radiotap_eht_t));

	/* IEEE80211 Radiotap Present flags */
	rteht->ieee_radiotap.it_present = HTOL32(WL_RADIOTAP_PRESENT_EHT);
	rteht->rt_common.tsft_h = HTOL32(txsts->tsf_hi);
	rteht->rt_common.tsft_l = HTOL32(txsts->tsf_lo);
	rteht->rt_common.flags = flags;
	rteht->rt_common.channel_freq = HTOL16(channel_frequency);
	rteht->rt_common.channel_flags = HTOL16(channel_flags);

	/* TODO: update IEEE80211 Radiotap AMPDU Status */

	/* EHT TLV */
	rteht->eht_t = HTOL16(IEEE80211_RADIOTAP_TLV_EHT);
	rteht->eht_l = HTOL16(IEEE80211_RADIOTAP_EHT_TLV_LEN);
	rteht->known = HTOL32(IEEE80211_RADIOTAP_EHT_HAVE_GI);
	/*
	 * TODO: More EHT fields in the Tx radiotap header (data0...8,etc) need to be populated,
	 * but might need added fields in the pcap_tx_hdr_t in order to populate.
	 */
	gi =  RSPEC_EHT_LTF_GI(rspec);
	rteht->data[0] = HTOL32(((gi & (IEEE80211_RADIOTAP_EHT_GI_MASK >>
				IEEE80211_RADIOTAP_EHT_GI_SHIFT)) <<
				IEEE80211_RADIOTAP_EHT_GI_SHIFT));

	/* EHT User info field */
	mcs = RSPEC_GE_VHT_MCS(txsts->mu_nss_mcs);
	nss = RSPEC_GE_VHT_NSS(txsts->mu_nss_mcs);

	rteht->user_info = HTOL32(IEEE80211_RADIOTAP_EHT_HAVE_MCS |
			IEEE80211_RADIOTAP_EHT_HAVE_NSS);
	/* mcs */
	rteht->user_info |= HTOL32(((mcs & (IEEE80211_RADIOTAP_EHT_MCS_USR_INF_MASK >>
				IEEE80211_RADIOTAP_EHT_MCS_USR_INF_SHIFT))
				<< IEEE80211_RADIOTAP_EHT_MCS_USR_INF_SHIFT));
	/* nss */
	rteht->user_info |= HTOL32(((nss & (IEEE80211_RADIOTAP_EHT_NSS_USR_INF_MASK >>
				IEEE80211_RADIOTAP_EHT_NSS_USR_INF_SHIFT))
				<< IEEE80211_RADIOTAP_EHT_NSS_USR_INF_SHIFT));

	/* U-SIG TLV */
	rteht->usig_t = HTOL16(IEEE80211_RADIOTAP_TLV_U_SIG);
	rteht->usig_l = HTOL16(IEEE80211_RADIOTAP_USIG_TLV_LEN);

	return 0;
}

/**
 * Send Tx packets to the radiotap interface
 *
 * *mac_header  : Pointer to the 802.11 Mac header
 * *pcap_tx_hdr : Pointer to the structure that contains Tx status information from the dongle
 * *rtap_header : Pointer to where to build the radiotap header
 */
static uint
dhd_pcap_to_rtap_txcpy(struct dot11_header *mac_header, pcap_tx_hdr_t *pcap_tx_hdr,
		struct sk_buff *pcap_skb)
{
	uint bsd_header_len, skb_hdroom;
	uint8 flags;
	uint16 fc = LTOH16(mac_header->fc);
	uint	phytype;	/**< 802.11 A/B/G /N */
	wl_radiotap_elem_t *rtap;

	/* Do some basic sanity checking of the pcap_tx_hdr, bail if it looks funny */
	if (wf_chspec_malformed(pcap_tx_hdr->chanspec)) {
		DHD_ERROR(("%s() bad channel spec 0x%x pkttype=%d\n", __FUNCTION__,
				pcap_tx_hdr->chanspec, PCAP_SKB_PKTTYPE(pcap_skb)));
		prhex("d11mac_hdr", (void*)mac_header, sizeof(struct dot11_header));
		prhex("pcap_tx_hdr", (void*)pcap_tx_hdr, sizeof(pcap_tx_hdr_t));
		dump_stack();

		return (0);
	}

	flags = 0;

	if (pcap_tx_hdr->rspec & WL_RSPEC_SHORT_PREAMBLE)
		flags |= IEEE80211_RADIOTAP_F_SHORTPRE;

	if ((fc &  FC_WEP) == FC_WEP)
		flags |= IEEE80211_RADIOTAP_F_WEP;

	if ((fc & FC_MOREFRAG) == FC_MOREFRAG)
		flags |= IEEE80211_RADIOTAP_F_FRAG;

	phytype = (pcap_tx_hdr->rspec & WL_RSPEC_ENCODING_MASK);
	skb_hdroom = skb_headroom(pcap_skb);

	switch (phytype) {
		case WL_RSPEC_ENCODE_RATE: {
			wl_radiotap_tx_legacy_t *rtl;

			bsd_header_len = sizeof(wl_radiotap_tx_legacy_t);

			/* Make room for the radiotap header */
			if (skb_hdroom >= bsd_header_len) {
				rtap = (wl_radiotap_elem_t *)skb_push(pcap_skb, bsd_header_len);

				rtl = &rtap->rtap_leg;

				dhd_radiotap_tx_legacy(mac_header, pcap_tx_hdr, rtl);
			}
			break;
		}

		case WL_RSPEC_ENCODE_HT: {
			wl_radiotap_ht_t *rtht;

			bsd_header_len = sizeof(wl_radiotap_ht_t);

			/* Make room for the radiotap header */
			if (skb_hdroom >= bsd_header_len) {
				rtap = (wl_radiotap_elem_t *)skb_push(pcap_skb, bsd_header_len);

				rtht = &rtap->rtap_ht;

				dhd_radiotap_tx_ht(mac_header, pcap_tx_hdr, rtht);
			}
			break;
		}

		case WL_RSPEC_ENCODE_VHT: {
			wl_radiotap_vht_t *rtvht;

			bsd_header_len = sizeof(wl_radiotap_vht_t);

			/* Make room for the radiotap header */
			if (skb_hdroom >= bsd_header_len) {
				rtap = (wl_radiotap_elem_t *)skb_push(pcap_skb, bsd_header_len);

				rtvht = &rtap->rtap_vht;

				dhd_radiotap_tx_vht(mac_header, pcap_tx_hdr, rtvht);
			}
			break;
		}

		case WL_RSPEC_ENCODE_HE: {
			wl_radiotap_he_t *rthe;

			bsd_header_len = sizeof(wl_radiotap_he_t);

			/* Make room for the radiotap header */
			if (skb_hdroom >= bsd_header_len) {
				rtap = (wl_radiotap_elem_t *)skb_push(pcap_skb, bsd_header_len);

				rthe =  &rtap->rtap_he;

				dhd_radiotap_tx_he(mac_header, pcap_tx_hdr, rthe);
			}
			break;
		}
		case WL_RSPEC_ENCODE_EHT: {
			wl_radiotap_eht_t *rteht;

			bsd_header_len = sizeof(wl_radiotap_eht_t);

			/* Make room for the radiotap header */
			if (skb_hdroom >= bsd_header_len) {
				rtap = (wl_radiotap_elem_t *)skb_push(pcap_skb, bsd_header_len);

				rteht =  &rtap->rtap_eht;

				dhd_radiotap_tx_eht(mac_header, pcap_tx_hdr, rteht);
			}
			break;
		}
		default:
			DHD_ERROR(("%s() unknown phytype 0x%x\n", __FUNCTION__, phytype));
			bsd_header_len = 0;
			break;
	}

	if (skb_hdroom < bsd_header_len) {
		DHD_ERROR(("%s() phytype 0x%x type %d len %d headroom %d < bsd_header_len %u\n",
			__FUNCTION__, phytype, PCAP_SKB_PKTTYPE(pcap_skb), pcap_skb->len,
			skb_hdroom, bsd_header_len));
		bsd_header_len = 0;
	}
	return bsd_header_len;
}

/**
 * Reassemble Tx AMSDUs then once the last one is handled, post to the radiotap interface
 *
 * *mac_header  : Pointer to the 802.11 Mac header
 * *pcap_tx_hdr : Pointer to the structure that contains Tx status information from the dongle
 * *pcap_skb    : Pointer to the SKB to rebuild the AMSDUs into
 * pcap_type	: AMSDU (first/mid/last)
 */
#define TXCPY_AMSDU_REASSEM_ERROR 0
#define TXCPY_AMSDU_REASSEM_KEEP_SKB 1
#define TXCPY_AMSDU_REASSEM_FREE_SKB 2
#define TXCPY_AMSDU_REASSEM_DONE 3
static uint
dhd_pcap_to_rtap_txcpy_amsdu(dhd_pub_t *dhdp, struct dot11_header *mac_header,
		pcap_tx_hdr_t *pcap_tx_hdr, struct sk_buff *pcap_skb, uint8 pcap_type)
{
	monitor_info_t *monitor_info;
	uint ret_val;

	monitor_info = dhdp->monitor_info;

	switch (pcap_type) {
		case PCAP_PKT_TX_COPY_AMSDU_FIRST:
			ret_val = dhd_pcap_to_rtap_txcpy(mac_header, pcap_tx_hdr, pcap_skb);
			if (ret_val != 0) {
				/* Save the new starting AMSDU subframe */
				monitor_info->tx_amsdu_len = pcap_skb->len;
				monitor_info->tx_amsdu_pkt = pcap_skb->data;
				if (PCAP_SKB_PKTFLAGS(pcap_skb) & PCAP_PKTFLAG_LAST) {
					ret_val = TXCPY_AMSDU_REASSEM_DONE;
				} else {
					ret_val = TXCPY_AMSDU_REASSEM_KEEP_SKB;
				}
			} else {
				ret_val = TXCPY_AMSDU_REASSEM_ERROR;
			}
			break;

		case PCAP_PKT_TX_COPY_AMSDU_INTRM:
			ret_val = TXCPY_AMSDU_REASSEM_FREE_SKB;
			/* fall through */

		case PCAP_PKT_TX_COPY_AMSDU_LAST:
			if (dhdp->tx_monitor_skb != NULL) {
				if (monitor_info->tx_amsdu_len) {
					int tailroom = skb_tailroom(dhdp->tx_monitor_skb);
					ASSERT(tailroom >= pcap_skb->len);
					if (tailroom < pcap_skb->len) {
						/* this should never happen */
						DHD_ERROR(("wl%d: %s: tailroom %d < len %d\n",
							dhdp->unit, __FUNCTION__, tailroom,
							pcap_skb->len));
					} else {
						skb_put(dhdp->tx_monitor_skb, pcap_skb->len);
						memcpy((monitor_info->tx_amsdu_pkt +
							monitor_info->tx_amsdu_len),
							pcap_skb->data, pcap_skb->len);
						monitor_info->tx_amsdu_len += pcap_skb->len;
					}
					if ((pcap_type == PCAP_PKT_TX_COPY_AMSDU_LAST) ||
						(PCAP_SKB_PKTFLAGS(pcap_skb) & PCAP_PKTFLAG_LAST)) {
						ret_val = TXCPY_AMSDU_REASSEM_DONE;
					}
				} else {
					ret_val = TXCPY_AMSDU_REASSEM_ERROR;
				}
			} else {
				ret_val = TXCPY_AMSDU_REASSEM_ERROR;
			}
			break;

		default:
			ret_val = TXCPY_AMSDU_REASSEM_ERROR;
			break;
	}

	return ret_val;

}
#endif /* BCM_PCAP */

#ifdef BCM_PCAP
/*
 * dhd_pcap_tx_mon()
 *   Create and send Tx radiotap header based on PCAP Tx pkt from dongle.
 *
 *   Return: 1 on success,
 *           0 on failure
 *
 * This function expects the pcap_skb data to look like this for
 * non-local copy Tx pkts (not to scale) :
 *
 * +------------------------------+
 * |       pcap_tx_hdr_t          |
 * |                              |
 * |..............................|
 * |  d11hdr_len |    d11hdr[]    |
 * |..............................|
 * |          d11hdr[]            |
 * +------------------------------+
 * |     SNAP Hdr (8-byte)        |
 * +------------------------------+
 * +        Packet Payload        |
 * |~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~|
 * |~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~|
 * +------------------------------+
 *
 * The resulting packet sent to the
 * radiotap header will be:
 *
 * +------------------------------+
 * |       Radiotap Header        |
 * +------------------------------+
 * |      802.11 MAC Header       |
 * +------------------------------+
 * |     SNAP Hdr (8-byte)        |
 * +------------------------------+
 * +        Packet Payload        |
 * |~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~|
 * |~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~|
 * +------------------------------+
 *
 * Like this for local copy tx (dongle generated) packets:
 *
 * +------------------------------+
 * |       pcap_tx_hdr_t          |
 * |                              |
 * +------------------------------+
 * |       802.11 MAC HDR         |
 * +------------------------------+
 * +        Packet Payload        |
 * |~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~|
 * |~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~|
 * +------------------------------+
 *
 *
 * The resulting packet sent to the
 * radiotap header will be:
 *
 * +------------------------------+
 * |       Radiotap Header        |
 * +------------------------------+
 * |      802.11 MAC Header       |
 * +------------------------------+
 * +        Packet Payload        |
 * |~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~|
 * |~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~|
 * +------------------------------+
 */
static int
dhd_pcap_tx_mon(dhd_pub_t *dhdp, struct sk_buff *pcap_skb)
{
	pcap_tx_hdr_t *pcap_tx_hdr;
	pcap_tx_hdr_ext_t *pcap_tx_hdr_ext;
	pcap_tx_hdr_ext_t local_pcap_tx_hdr;
	void *d11_mac_hdr;
	uint8 pcap_type;
	uint amsdu_status;
	bool is_amsdu, secondary_msdu;
	int pktlen;

	/* Sanity check the pcap_skb before using it */
	ASSERT(pcap_skb != NULL);

	pktlen = PKTLEN(dhdp->osh, pcap_skb);

	is_amsdu = secondary_msdu = FALSE;

	pcap_type = PCAP_SKB_PKTTYPE(pcap_skb);

	switch (pcap_type) {
		case PCAP_PKT_TX_COPY_AMSDU_FIRST:
			/* Flush any previously collected */
			if (dhdp->tx_monitor_skb) {
				dhdp->monitor_info->tx_amsdu_len = 0;
				dhdp->monitor_info->tx_amsdu_pkt = NULL;

				/*
				 * If filtering on only the FIRST AMSDU, then send
				 * the previously cached FIRST AMSDU up
				 */
				if (PCAP_CFG_TX_FIRST_MSDU(dhdp->pcap.pcap_config)) {
					pcap_skb->dev->stats.rx_packets++;
					skb_reset_mac_header(dhdp->tx_monitor_skb);
					netif_rx(dhdp->tx_monitor_skb);
				} else {
					/*
					 * This must mean we didn't see a LAST before this
					 * First, so ditch the in-progress AMSDU reassembly
					 */
					PKTFREE(dhdp->osh, dhdp->tx_monitor_skb, TRUE);
				}
				dhdp->tx_monitor_skb = NULL;
			}

			is_amsdu = TRUE;
			/* fall through */
		case PCAP_PKT_TX_COPY:
			pcap_tx_hdr_ext = (pcap_tx_hdr_ext_t *)pcap_skb->data;

			/* Move the data pointer past the pcap_tx_hdr */
			skb_pull(pcap_skb, sizeof(pcap_tx_hdr_ext_t));

			/* Make a copy of the pcap_tx_hdr so when we are creating the
			 * radiotap header over it in place in the skb, we don't
			 * lose any info
			 */
			memcpy(&local_pcap_tx_hdr, pcap_tx_hdr_ext, sizeof(pcap_tx_hdr_ext_t));

			/* Make room for and copy in D11 Mac header */
			if (local_pcap_tx_hdr.d11hdr_len <= sizeof(local_pcap_tx_hdr.d11hdr)) {
				d11_mac_hdr = (void *)skb_push(pcap_skb,
						local_pcap_tx_hdr.d11hdr_len);
				memcpy_s((uint8 *)d11_mac_hdr, sizeof(local_pcap_tx_hdr.d11hdr),
						&local_pcap_tx_hdr.d11hdr,
						local_pcap_tx_hdr.d11hdr_len);
			} else {
				DHD_PKT_PCAP(("Invalid len in d11hdr_len field %d, need %d\n",
						local_pcap_tx_hdr.d11hdr_len,
						(int)sizeof(local_pcap_tx_hdr.d11hdr)));
				/* Invlaid len in d11hdr_len field */
				kfree_skb(pcap_skb);
				return (1);
			}
			break;

		case PCAP_PKT_TX_COPY_AMSDU_INTRM:
		case PCAP_PKT_TX_COPY_AMSDU_LAST:
			d11_mac_hdr = (void *)NULL;
			is_amsdu = TRUE;
			secondary_msdu = TRUE;
			break;

		case PCAP_PKT_TX_LOCAL_COPY:
			pcap_tx_hdr =  (pcap_tx_hdr_t *)pcap_skb->data;

			/* Move the data pointer past the pcap_tx_hdr */
			d11_mac_hdr = skb_pull(pcap_skb, sizeof(pcap_tx_hdr_t));
			memcpy(&local_pcap_tx_hdr, pcap_tx_hdr, sizeof(pcap_tx_hdr_t));
			break;

		default:
			DHD_ERROR(("%s() unknown pcap_type 0x%x\n", __FUNCTION__, pcap_type));
			kfree_skb(pcap_skb);
			return (1);
			break;
	}

	/* Create radiotap header */
	if (is_amsdu) {
		int pkt_align;
		if (!dhdp->tx_monitor_skb) {
			dhdp->tx_monitor_skb = pcap_skb;
		}
		/* only need to check for tail room on msdu 2+. First msdu reuses
		 * SKB (no tailroom required) -- in 'first-msdu-only' mode, no add'l tailroom
		 * is needed.
		 */
		if (secondary_msdu && skb_tailroom(dhdp->tx_monitor_skb) < pktlen) {
			DHD_PKT_PCAP(("%s() Not enough tailroom %d need %d\n", __FUNCTION__,
					skb_tailroom(dhdp->tx_monitor_skb), pktlen));
			if (dhdp->tx_monitor_skb != pcap_skb) {
				PKTFREE(dhdp->osh, dhdp->tx_monitor_skb, TRUE);
			}
			PKTFREE(dhdp->osh, pcap_skb, TRUE);
			dhdp->tx_monitor_skb = NULL;
			dhdp->monitor_info->tx_amsdu_len = 0;
			dhdp->monitor_info->tx_amsdu_pkt = NULL;
			return -1;
		}

		/* Add msdu in amsdu padding to 4-byte alignment */
		pkt_align = pktlen & 0x3;
		if (pkt_align) {
			skb_put(pcap_skb, (4 - pkt_align));
		}
		amsdu_status = dhd_pcap_to_rtap_txcpy_amsdu(dhdp, d11_mac_hdr,
				(pcap_tx_hdr_t *)(&local_pcap_tx_hdr), pcap_skb, pcap_type);

		switch (amsdu_status) {
			case TXCPY_AMSDU_REASSEM_KEEP_SKB:
				break;

			case TXCPY_AMSDU_REASSEM_FREE_SKB:
				kfree_skb(pcap_skb);
				break;

			case TXCPY_AMSDU_REASSEM_DONE:
				/* Send radiotap header packet out the radiotap interface */
				pcap_skb->dev->stats.rx_packets++;
				if (pcap_type != PCAP_PKT_TX_COPY_AMSDU_FIRST) {
					kfree_skb(pcap_skb);
				}
				skb_reset_mac_header(dhdp->tx_monitor_skb);
				netif_rx(dhdp->tx_monitor_skb);
				dhdp->tx_monitor_skb = NULL;
				dhdp->monitor_info->tx_amsdu_len = 0;
				dhdp->monitor_info->tx_amsdu_pkt = NULL;
				break;

			case TXCPY_AMSDU_REASSEM_ERROR:
			default:
				DHD_ERROR(("%s() Error amsdu_status %d\n",
						__FUNCTION__, amsdu_status));
				PKTFREE(dhdp->osh, pcap_skb, TRUE);
				dhdp->tx_monitor_skb = NULL;
				dhdp->monitor_info->tx_amsdu_len = 0;
				dhdp->monitor_info->tx_amsdu_pkt = NULL;
				break;
		}
		return (1);

	} else {
		if (dhd_pcap_to_rtap_txcpy(d11_mac_hdr,
				(pcap_tx_hdr_t *)(&local_pcap_tx_hdr), pcap_skb))
		{
			/* Send radiotap header packet out the radiotap interface */
			skb_reset_mac_header(pcap_skb);
			pcap_skb->dev->stats.rx_packets++;
			netif_rx(pcap_skb);
		} else {
			/* Something failed with rtap creation */
			kfree_skb(pcap_skb);
			return (1);
		}
	}

	return (1);
}
#else
static int
dhd_pcap_tx_mon(dhd_pub_t *dhdp, struct sk_buff *pcap_skb)
{
	BCM_REFERENCE(dhdp);
	BCM_REFERENCE(pcap_skb);

	return (1);
}
#endif /* BCM_PCAP */

static inline void
dhd_pcap_tap(dhd_pub_t *pub, void *event_info, enum dhd_wq_event event)
{
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 19, 0))
	struct sk_buff *pcap_skb, *n;
	unsigned long flags;
	int getnextpkt;
	unsigned int process_cnt = 0;

	list_for_each_entry_safe(pcap_skb, n, &pub->pcap.skb_list, list) {
		DHD_PCAP_LOCK(pub->pcap.skb_pcap_tap_lock, flags);
		list_del(&pcap_skb->list);
		DHD_PCAP_UNLOCK(pub->pcap.skb_pcap_tap_lock, flags);

		process_cnt++; /* Count total number of bufs processed */

		/*
		 * normally the SKBs that come here are unique copies and should
		 * have a ref count of 1. But if the code path changes and we
		 * receive a shared skb, we cannot expand the header for it
		 * (will crash) so skip these SKBs.
		 */
		if (skb_shared(pcap_skb)) {
			continue;
		}
		/* If ndev is deleted, free all skbs */
		if (pub->pcap.pcap_if.pcap_ndev == NULL) {
			kfree_skb(pcap_skb);
			if (pub->monitor_skb) {
				PKTFREE(pub->osh, pcap_skb, FALSE);
				pub->monitor_skb = NULL;
			}
			continue;
		}

		/* send the packet to the radiotap interface */
		pcap_skb->dev = pub->pcap.pcap_if.pcap_ndev;
		pcap_skb->next = NULL;

		switch (PCAP_SKB_PKTTYPE(pcap_skb)) {
			case PCAP_PKT_RX_PROMISC:
			case PCAP_PKT_RX_PROMISC_MULTI_FIRST:
			case PCAP_PKT_RX_PROMISC_MULTI_INTRM:
			case PCAP_PKT_RX_PROMISC_MULTI_LAST:
			case PCAP_PKT_RX_COPY: /* intentional fall-through */
				getnextpkt = dhd_pcap_rx_mon(pub, (void *)pcap_skb,
						PCAP_SKB_PKTTYPE(pcap_skb));
				break;

			case PCAP_PKT_TX_COPY:
			case PCAP_PKT_TX_COPY_AMSDU_FIRST:
			case PCAP_PKT_TX_COPY_AMSDU_INTRM:
			case PCAP_PKT_TX_COPY_AMSDU_LAST:
			case PCAP_PKT_TX_LOCAL_COPY:
				/* Send radiotap header packet out the radiotap interface */
				getnextpkt = dhd_pcap_tx_mon(pub, pcap_skb);
				break;

			default:
				DHD_ERROR(("%s: unhandled pkt type %d\n", __FUNCTION__,
					PCAP_SKB_PKTTYPE(pcap_skb)));
				getnextpkt = 1;
				break;
		}

		if (getnextpkt) {
			/* process the next packet */
			continue;
		}

		if (pub->monitor_skb) {
			skb_reset_mac_header(pub->monitor_skb);

			pub->monitor_skb->dev->stats.rx_packets++;
			netif_rx(pub->monitor_skb);

			pub->monitor_skb = NULL;
		} else {
			printk("\n\n %s() : !!! monitor_skb NULL !!!! \n\n", __FUNCTION__);
		}

	}

	/*
	 * Grab the lock again and decrement the list count with the number of bufs we
	 * processed in this execution as well as flagging that this execution has completed
	 */
	DHD_PCAP_LOCK(pub->pcap.skb_pcap_tap_lock, flags);
	pub->pcap.skb_list_cnt -= process_cnt;
	pub->pcap.pcap_tap_proc_pend = FALSE;
	DHD_PCAP_UNLOCK(pub->pcap.skb_pcap_tap_lock, flags);

#endif /* LINUX_VERSION_CODE >= KERNEL_VERSION(4, 19, 0) */
}

#define DHD_PCAP_FORCE_SCHEDULE_TOUT_MS 1000
/*
 * Called from dhd_watchdog_thread().
 *
 * This function will deferred schedule the pcap_tap thread if there packets posted to it
 * but the thread has not been scheduled in the last DHD_PCAP_FORCE_SCHEDULE_TOUT_MS msec.
 *
 * This is mainly to cover the situation where packets are only being slowly posted to the
 * pcap_tap and the pkt list has not hit the PCAP_TAP_SCHED_THRESH yet.
 */
int
dhd_pcap_tap_watchdog(dhd_pub_t *dhdp)
{
	uint32 now;
	uint32 delta;
	unsigned long flags;

	/* Don't need to try and schedule the deferred task if it is not enabled (this can
	 * happen if dhd or dongle has crashed)
	 */
	if (dhd_deferred_work_enabled(dhdp, DHD_WQ_WORK_PCAP_TAP) == FALSE) {
		dhdp->pcap.pcap_tap_proc_pend = FALSE;
		return BCME_ERROR;
	}

	/* If there are pkts in the pcap_tap list AND pcap_tap is not pending to run,
	 * check the time and see if it has been DHD_PCAP_FORCE_SCHEDULE_TOUT_MS 1000ms
	 * since the last time pcap_tap was scheduled
	 */
	DHD_PCAP_LOCK(dhdp->pcap.skb_pcap_tap_lock, flags);
	if ((dhdp->pcap.skb_list_cnt != 0) && (dhdp->pcap.pcap_tap_proc_pend == FALSE)) {
		now = OSL_SYSUPTIME();

		/* Calculate the delta, taking into account wrap */
		if (now <  dhdp->pcap.last_scheduled) {
			delta = (~0 - dhdp->pcap.last_scheduled) + now;
		} else {
			delta = now - dhdp->pcap.last_scheduled;
		}

		/* Schedule pcap_tap now if we have never scheduled the pcap_tap thread and now
		 * it is time OR if the time since the last schedule exceeds the timeout
		 */
		if (((dhdp->pcap.last_scheduled == 0) && (now > DHD_PCAP_FORCE_SCHEDULE_TOUT_MS)) ||
			delta > DHD_PCAP_FORCE_SCHEDULE_TOUT_MS) {
			if (dhd_deferred_schedule_work(dhdp, NULL, DHD_WQ_WORK_PCAP_TAP,
				dhd_pcap_tap, DHD_WORK_PRIORITY_LOW) == DHD_WQ_STS_OK) {
				dhdp->pcap.pcap_tap_proc_pend = TRUE;
				dhdp->pcap.last_scheduled = now;
				dhdp->pcap.pcap_tap_wd_sched++;
			} else {
				PCAP_PRINT("%s() : Failed to schedule deferred work\n",
						__FUNCTION__);
			}
		}
	}
	DHD_PCAP_UNLOCK(dhdp->pcap.skb_pcap_tap_lock, flags);

	return BCME_OK;
}

static int
dhd_pcap_if_subif_start_xmit(struct sk_buff *skb, struct net_device *ndev)
{
	int ret = 0;
	int rtap_len;
	int qos_len = 0;
	int dot11_hdr_len = 24;
	int snap_len = 6;
	unsigned char *pdata;
	unsigned short frame_ctl;
	unsigned char src_mac_addr[6];
	unsigned char dst_mac_addr[6];
	struct ieee80211_hdr *dot11_hdr;
	struct ieee80211_radiotap_header *rtap_hdr;
	pcap_interface* pcap_if;
	struct net_device *real_ndev;

	pcap_if = ndev_to_pcapif(ndev);
	if (pcap_if == NULL || pcap_if->real_ndev == NULL) {
		PCAP_PRINT(" cannot find matched net dev, skip the packet\n");
		goto fail;
	}

	if (unlikely(skb->len < sizeof(struct ieee80211_radiotap_header)))
		goto fail;

	rtap_hdr = (struct ieee80211_radiotap_header *)skb->data;
	if (unlikely(rtap_hdr->it_version))
		goto fail;

	rtap_len = get_unaligned_le16(&rtap_hdr->it_len);
	if (unlikely(skb->len < rtap_len))
		goto fail;

	real_ndev = pcap_if->real_ndev;

	/* Skip the ratio tap header */
	skb_pull(skb, rtap_len);

	dot11_hdr = (struct ieee80211_hdr *)skb->data;
	frame_ctl = le16_to_cpu(dot11_hdr->frame_control);
	/* Check if the QoS bit is set */
	if ((frame_ctl & IEEE80211_FCTL_FTYPE) == IEEE80211_FTYPE_DATA) {
		/* Check if this ia a Wireless Distribution System (WDS) frame
		 * which has 4 MAC addresses
		 */
		if (dot11_hdr->frame_control & 0x0080)
			qos_len = 2;
		if ((dot11_hdr->frame_control & 0x0300) == 0x0300)
			dot11_hdr_len += 6;

		memcpy(dst_mac_addr, dot11_hdr->addr1, sizeof(dst_mac_addr));
		memcpy(src_mac_addr, dot11_hdr->addr2, sizeof(src_mac_addr));

		/* Skip the 802.11 header, QoS (if any) and SNAP, but leave spaces for
		 * for two MAC addresses
		 */
		skb_pull(skb, dot11_hdr_len + qos_len + snap_len - sizeof(src_mac_addr) * 2);
		pdata = (unsigned char*)skb->data;
		memcpy(pdata, dst_mac_addr, sizeof(dst_mac_addr));
		memcpy(pdata + sizeof(dst_mac_addr), src_mac_addr, sizeof(src_mac_addr));
		PKTSETPRIO(skb, PRIO_8021D_BE);

		/* Use the real net device to transmit the packet */
		ret = real_ndev->netdev_ops->ndo_start_xmit(skb, real_ndev);

		return ret;
	}
fail:
	kfree_skb(skb);
	return 0;
}
#else
static inline void
dhd_pcap_tap(dhd_pub_t *pub, void *event_info, enum dhd_wq_event event)
{
	return;
}

static int
dhd_pcap_if_subif_start_xmit(struct sk_buff *skb, struct net_device *ndev)
{
	return NETDEV_TX_OK;
}
#endif /* WL_MONITOR */

/* Look up dhd's net device table to find a match (e.g. interface "eth0" is a match for "mon.eth0"
 * "p2p-eth0-0" is a match for "mon.p2p-eth0-0")
 */
static struct net_device*
lookup_real_netdev(dhd_pub_t *pub)
{
	return dhd_idx2net(pub, 0);
}

static pcap_interface*
ndev_to_pcapif(struct net_device *ndev)
{
	dhd_linux_pcap_t **dhd_pcap = (dhd_linux_pcap_t **) netdev_priv(ndev);
	return &(*dhd_pcap)->pcap_if;
}

static int
dhd_pcap_if_open(struct net_device *ndev)
{
	int ret = 0;

	return ret;
}

static int
dhd_pcap_if_stop(struct net_device *ndev)
{
	int ret = 0;

	return ret;
}

static void
dhd_pcap_if_set_multicast_list(struct net_device *ndev)
{
	pcap_interface* pcap_if;

	pcap_if = ndev_to_pcapif(ndev);
	if (pcap_if == NULL || pcap_if->real_ndev == NULL) {
		PCAP_PRINT(" cannot find matched net dev, skip the packet\n");
	}
}

static int
dhd_pcap_if_change_mac(struct net_device *ndev, void *addr)
{
	int ret = 0;
	pcap_interface* pcap_if;

	pcap_if = ndev_to_pcapif(ndev);
	if (pcap_if == NULL || pcap_if->real_ndev == NULL) {
		PCAP_PRINT(" cannot find matched net dev, skip the packet\n");
	}
	return ret;
}

/* yet another abstraction to copy a PKT into an SKB */
struct sk_buff*
dhd_pcap_copy(dhd_pub_t *pub, void *pkt, uint8 pcap_pkt_type)
{
	struct sk_buff *out_skb;
	BCM_REFERENCE(pub);
#if defined(BCM_NBUFF_PKT_BPM)
	if (IS_FKBUFF_PTR(pkt)) {
		out_skb = dhd_xlate_to_skb(pub, pkt);
	} else
#endif /* BCM_NBUFF_PKT_BPM */
	{
		out_skb = skb_copy(pkt, GFP_NOWAIT);
	}

	if (out_skb) {
		PCAP_SKB_PKTTYPE(out_skb) = pcap_pkt_type;
	}
	return out_skb;
}

#ifdef BCM_PCAP
/* Max PCAP pkt skb headroom including the partial LLC header that DHD adds  */
#define PCAP_RTAP_HDR		sizeof(wl_radiotap_elem_t)
#define PCAP_AMSDU_SUBFRAM	(ETHER_HDR_LEN + DOT11_LLC_SNAP_HDR_LEN)
#define MAX_PCAP_HEADROOM	(PCAP_MAX_D11_HDR_LEN + DOT11_LLC_SNAP_HDR_LEN + \
				PCAP_AMSDU_SUBFRAM + PCAP_RTAP_HDR)

/* For a TX_COPY pkt, the skb data pointer does not point to begining of each data frame.
 * Due to the snap header was pushed into the packet, the actual data pointer needs to
 * be adjusted, either 6-bytes Ethernet or 8-bytes (Ethernet address + 2), based on the
 * AMSDU/MPDU frame type. This function is used to calculate correct offset for skb
 * headroom to adjust the data pointer and pkt length accordingly. The pcap_tx_hdr_ext_t
 * also needs being added for MPDU frame and the first-AMSDU frame.
 */
int
dhd_pcap_headroom_adj(uint8 pcap_type, bool add_txhdr, int *err)
{
	int hdr_adj_len;
	*err = BCME_OK;
	switch (pcap_type) {
		case PCAP_PKT_TX_COPY:
			/* MPDU has the pcap_tx header */
			if (add_txhdr) {
				hdr_adj_len = sizeof(pcap_tx_hdr_ext_t) - ETHER_ADDR_LEN;
			} else {
				hdr_adj_len = -ETHER_ADDR_LEN;
			}
			break;

		case PCAP_PKT_TX_COPY_AMSDU_FIRST:
			/* First AMSDU has the pcap_tx header */
			if (add_txhdr) {
				hdr_adj_len = sizeof(pcap_tx_hdr_ext_t) + ETHER_ADDR_LEN + 2;
			} else {
				hdr_adj_len = ETHER_ADDR_LEN + 2;
			}
			break;

		case PCAP_PKT_TX_COPY_AMSDU_INTRM:
		case PCAP_PKT_TX_COPY_AMSDU_LAST:
			/* Non-First AMSDU do not have a pcap_tx header */
			hdr_adj_len = ETHER_ADDR_LEN + 2;
			break;

		case PCAP_PKT_TX_LOCAL_COPY:
		case PCAP_PKT_RX_PROMISC:
		case PCAP_PKT_RX_PROMISC_MULTI_FIRST:
		case PCAP_PKT_RX_PROMISC_MULTI_INTRM:
		case PCAP_PKT_RX_PROMISC_MULTI_LAST:
		case PCAP_PKT_RX_COPY:
			hdr_adj_len = 0;
			break;

		default:
			*err = BCME_ERROR;
			hdr_adj_len = 0;
			break;
	}

	return (hdr_adj_len);
}

/*
 * yet another abstraction to duplicate a PKT into an SKB, but this allocates a whole new fresh
 * skb.  This function includes support for creating an out_skb with a larger data section (size).
 * Note that if an FKB is processed, if expansion is not required the FKB will be translated to
 * an SKB. In this case, the FKB data buffer persists and is used by the SKB. Additionally, the
 * fkb_xlated return value is set TRUE. It is important in this case that the FKB is not used
 * any further as the data buffer is now associated with the output SKB.
 */
struct sk_buff*
dhd_pcap_dup(dhd_pub_t *pub, void *pkt, uint16 size, uint8 pcap_pkt_type)
{
	struct sk_buff *out_skb;
	struct sk_buff *in_skb = (struct sk_buff *)pkt;
	int pcap_hdr_adj_len, pcap_tailroom;

	BCM_REFERENCE(pub);

#if defined(BCM_NBUFF_PKT)
	if (IS_FKBUFF_PTR(pkt)) {
		/*
		 * If the fkb packet data buffer size is less than the requeested size, we need to
		 * allocate a new bigger skb and copy the data from the fkb data buffer into the new
		 * skb then free the input fkb
		 */
		FkBuff_t *fkb = PNBUFF_2_FKBUFF(pkt);
		uint8 *data_ptr;

		out_skb = alloc_skb(size + BCM_PKT_HEADROOM, GFP_KERNEL);

		if (out_skb) {
			skb_reserve(out_skb, BCM_PKT_HEADROOM);

			data_ptr = skb_put(out_skb, fkb->len);

			/* Make sure to copy any data in the headroom area too */
			memcpy((data_ptr - BCM_PKT_HEADROOM),
					((uint8 *)fkb->data) - BCM_PKT_HEADROOM,
					fkb->len + BCM_PKT_HEADROOM);
		}
	} else
#endif /* BCM_NBUFF_PKT */
	{
		pcap_tailroom = skb_tailroom(in_skb);
		if (size > in_skb->len) {
			pcap_tailroom += (size - in_skb->len);
		}
		out_skb = skb_copy_expand(in_skb, MAX_PCAP_HEADROOM, pcap_tailroom, GFP_NOWAIT);
	}
	if (out_skb == NULL) {
		/* this error can happen when SKB's are exhausted. */
		pub->pcap.pcap_dup_err++;
		DHD_PKT_PCAP(("wl%d: %s: skb copy fail\n", pub->unit, __FUNCTION__));
	} else {
		int err;
		/* Trim the out_skb if the size is smaller than the output buffer */
		skb_trim(out_skb, size);
		PCAP_SKB_PKTTYPE(out_skb) = pcap_pkt_type;
		pcap_hdr_adj_len = dhd_pcap_headroom_adj(pcap_pkt_type, TRUE, &err);
		if (err != BCME_OK) {
			DHD_ERROR(("wl%d: %s: Unexpected pcap_pkt_type 0x%x\n",
				pub->unit, __FUNCTION__, pcap_pkt_type));
			pub->pcap.pcap_dup_err++;
		} else {
			/* We should always have enought headroom, but just in case... */
			ASSERT((int)skb_headroom(out_skb) >= pcap_hdr_adj_len);
			if (pcap_hdr_adj_len) {
				skb_push(out_skb, pcap_hdr_adj_len);
			}
		}
	}

	return (out_skb);
}
#endif /* BCM_PCAP */

/* Number of buffers queued to the dhd_pcap_tap thread before it is (deferred) scheduled */
#define PCAP_TAP_SCHED_THRESH 32

/* Max number of buffers that can be queued to the dhd_pcap_tap thread */
#define PCAP_TAP_MAX_LIST_DEPTH 1536

/**
 * dhd_pcap_queue(dhd_pub_t *pub, struct sk_buff *skb)
 *
 * Post the passed skb to the dhd_pcap_tap() deferred thread for processing of the
 * radiotap header.
 *
 * Return:
 *   BCME_ERROR : The skb was not queued, the caller should handle disposition (e.g., freeing)
 *   BCME_OK    : The skb was successfully queued
 */
int
dhd_pcap_queue(dhd_pub_t *pub, struct sk_buff *skb)
{
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 19, 0))
	unsigned long flags;
	bool schedule;

	/* If there are too many buffers queued to the dhd_pcap_tap thread already,
	 * don't queue this one, just return
	 */

	if (pub->pcap.skb_list_cnt >= pub->pcap.pcap_tap_q_drop_thresh) {
		pub->pcap.pcap_tap_q_drop++;
		return BCME_ERROR;
	}

	DHD_PCAP_LOCK(pub->pcap.skb_pcap_tap_lock, flags);
	list_add_tail(&skb->list, &pub->pcap.skb_list);
	pub->pcap.skb_list_cnt++; /* Count that a buffer has been added to the list */

	/*
	 * If the dhd_pcap_tap deferred thread has not been scheduled AND the number of buffers
	 * exceeds the defined schedule threashold, then schedule the execution of the
	 * dhd_pcap_tap deferred thread now
	 */
	if (!pub->pcap.pcap_tap_proc_pend && (pub->pcap.skb_list_cnt > PCAP_TAP_SCHED_THRESH)) {
		schedule = TRUE;
	} else {
		schedule = FALSE;
	}

	if (schedule) {
		if (dhd_deferred_schedule_work(pub, NULL, DHD_WQ_WORK_PCAP_TAP,
				dhd_pcap_tap, DHD_WORK_PRIORITY_LOW) == DHD_WQ_STS_OK) {
			pub->pcap.pcap_tap_proc_pend = TRUE;

			pub->pcap.last_scheduled = OSL_SYSUPTIME();
		} else {
			PCAP_PRINT("%s() : Failed to schedule deferred work\n",
					__FUNCTION__);
		}
	}
	DHD_PCAP_UNLOCK(pub->pcap.skb_pcap_tap_lock, flags);

	return BCME_OK;
#else
	return BCME_ERROR;
#endif /* LINUX_VERSION_CODE >= KERNEL_VERSION(4, 19, 0) */
}

/**
 *   PCAP rx-copy creation of buffer for passing on to the radiotap
 *   API.  This format of the buffer for radiotap :
 *	H/W header
 *	PHY RX status header
 *	PLCP/802.11 MAC HDR
 *	PKT Data
 *  Return:
 *     0  : Successful creation of radiotap buffer and handoff of SKB to radiotap API
 *    -1  : Failed to create radiotap buffer
 */

#ifdef BCM_PCAP
int
dhd_pcap_to_rtap_rxcpy(dhd_pub_t *dhd_pub, struct sk_buff *data_skb,
                       uint8 *mac_hdr, uint16 mac_hdr_len,
                       uint8 *hw_hdr, uint16 hw_hdr_len,
                       uint8 *rxphysts, uint16 physts_len, uint32 pcap_flags)
{
	int ret_val;
	uint8 *data_ptr;

	/*
	 * Sanity check that we have room in this skb to add the
	 * radiotap headers
	 */
	if (skb_headroom(data_skb) < (mac_hdr_len + physts_len + hw_hdr_len)) {
		DHD_ERROR(("%s() Insufficient headroom for radiotap header %d %d\n",
				__FUNCTION__, skb_headroom(data_skb),
				(mac_hdr_len + physts_len + hw_hdr_len)));
		ret_val = -1;
		PKTFREE(dhd_pub->osh, data_skb, FALSE);
	} else {

		/* prepend the plcp and d11 mac header */
		data_ptr = skb_push(data_skb, mac_hdr_len);
		memcpy(data_ptr, mac_hdr, mac_hdr_len);

#ifdef BCM_PCAP_PHYSTS
		/* prepend the phy header next */
		data_ptr = skb_push(data_skb, physts_len);
		memcpy(data_ptr, rxphysts, physts_len);

		/* prepend the h/w rx header */
		data_ptr = skb_push(data_skb, hw_hdr_len);
#else
		/* prepend the h/w rx header */
		data_ptr = skb_push(data_skb, (hw_hdr_len + physts_len));
#endif /* BCM_PCAP_PHYSTS */
		memcpy(data_ptr, hw_hdr, hw_hdr_len);

		/* Send the SKB off for radiotap creation */
		PCAP_SKB_PKTTYPE(data_skb) = PCAP_PKT_RX_COPY;
		PCAP_SKB_PKTFLAGS(data_skb) = pcap_flags;
		if (dhd_pcap_queue(dhd_pub, data_skb) == BCME_OK) {
			dhd_pub->pcap.rxstats.rxpkt_enqd++;
		} else {
			/* Failed to queue buf to pcap, free the skb */
			PKTFREE(dhd_pub->osh, data_skb, FALSE);
		}
		ret_val = 0;
	}

	return (ret_val);
}
#endif /* BCM_PCAP */

int
dhd_pcap_add(const char *name, struct net_device **new_ndev, dhd_pub_t *pub)
{
	int ret = 0;
	struct net_device* ndev = NULL;
	dhd_linux_pcap_t **dhd_pcap;

	PCAP_TRACE("enter, if name: %s\n", name);

	if (!name || !new_ndev) {
		PCAP_PRINT("invalid parameters\n");
		ret = -EINVAL;
		goto out;
	}

	if (pub->pcap.pcap_if.pcap_ndev) {
		return 0;
	}

	/* alloc_etherdev call may go to sleep,so give up dhd lock(s) and reaquire it once done */
	DHD_UNLOCK(pub);
	DHD_OS_WAKE_UNLOCK(pub);
	ndev = alloc_etherdev(sizeof(dhd_linux_pcap_t*));
	DHD_OS_WAKE_LOCK(pub);
	DHD_LOCK(pub);

	if (!ndev) {
		return -ENOMEM;
	}

	ndev->type = ARPHRD_IEEE80211_RADIOTAP;
	strlcpy(ndev->name, name, sizeof(ndev->name));
	ndev->netdev_ops = &dhd_pcap_if_ops;

	/* register_netdevice can call schedule() so give up dhd lock(s) */
	DHD_UNLOCK(pub);
	DHD_OS_WAKE_UNLOCK(pub);
	ret = register_netdevice(ndev);
	DHD_OS_WAKE_LOCK(pub);
	DHD_LOCK(pub);

	if (ret) {
		PCAP_PRINT(" register_netdevice failed (%d)\n", ret);
		goto out;
	}

	*new_ndev = ndev;
	pub->pcap.pcap_if.radiotap_enabled = TRUE;
	pub->pcap.pcap_if.pcap_ndev = ndev;
	pub->pcap.pcap_if.real_ndev = lookup_real_netdev(pub);
	dhd_pcap = (dhd_linux_pcap_t **)netdev_priv(ndev);
	*dhd_pcap = &pub->pcap;
	pub->pcap.pcap_state = PCAP_STATE_INTERFACE_ADDED;
	pub->pcap.last_scheduled = 0;
	pub->pcap.pcap_filter = PCAP_CAPT_ALL_FRAMES;
	pub->pcap.pcap_tap_q_drop_thresh = PCAP_TAP_MAX_LIST_DEPTH;

#ifdef WL_MONITOR
	ret = bcmwifi_monitor_create(&pub->monitor_info);
	if (ret == TRUE) {
		pub->monitor_info->corerev = dhd_macdbg_corerev(pub);
#ifdef BCM_PCAP
		pub->monitor_info->pcap = TRUE;
#endif
		ret = 0;
	} else {
		ret = -ENOMEM;
	}
#endif /* WL_MONITOR */

out:
	if (ret && ndev)
		free_netdev(ndev);

	return ret;

}

int
dhd_pcap_del(dhd_pub_t *pub)
{
	if (pub->pcap.pcap_if.pcap_ndev) {
			pub->pcap.pcap_if.radiotap_enabled = FALSE;
			pub->pcap.pcap_if.real_ndev = NULL;
			DHD_UNLOCK(pub);
			DHD_OS_WAKE_UNLOCK(pub);
			unregister_netdevice(pub->pcap.pcap_if.pcap_ndev);
			rtnl_unlock();
			free_netdev(pub->pcap.pcap_if.pcap_ndev);
			rtnl_lock();
			DHD_OS_WAKE_LOCK(pub);
			DHD_LOCK(pub);
			pub->pcap.pcap_if.pcap_ndev = NULL;
			pub->pcap.pcap_state = PCAP_STATE_INTERFACE_DELETED;
	}
	return 0;
}

int
dhd_pcap_init(dhd_pub_t *dhd_pub)
{
	if (dhd_pub->pcap.pcap_state == PCAP_STATE_DEINIT) {
		dhd_pub->pcap.pcap_state = PCAP_STATE_INIT;
		INIT_LIST_HEAD(&dhd_pub->pcap.skb_list);
		dhd_pub->pcap.skb_pcap_tap_lock = dhd_os_spin_lock_init(dhd_pub->osh);
		if (dhd_pub->pcap.skb_pcap_tap_lock == NULL) {
			ASSERT(dhd_pub->pcap.skb_pcap_tap_lock);
			DHD_ERROR(("dhd_pcap_init: lock alloc failure\n"));
			return BCME_NOMEM;
		}
		/* Init rx-cpy storage */
#if defined(BCM_PCAP)
		dhd_pub->pcap.pcap_config = 0;
#endif /* BCM_PCAP */
	}

	return 0;
}

int
dhd_pcap_uninit(dhd_pub_t *dhd_pub)
{
	struct net_device *ndev;
	if (dhd_pub->pcap.pcap_state != PCAP_STATE_DEINIT) {
		ndev = dhd_pub->pcap.pcap_if.pcap_ndev;
		if (ndev) {
			unregister_netdev(ndev);
			free_netdev(ndev);
			dhd_pub->pcap.pcap_if.real_ndev = NULL;
			dhd_pub->pcap.pcap_if.pcap_ndev = NULL;
			dhd_pub->pcap.pcap_if.radiotap_enabled = FALSE;
		}
		if (dhd_pub->pcap.skb_pcap_tap_lock) {
			dhd_os_spin_lock_deinit(dhd_pub->osh, dhd_pub->pcap.skb_pcap_tap_lock);
		}
	}
	dhd_pub->pcap.pcap_state = PCAP_STATE_DEINIT;
	return 0;
}
