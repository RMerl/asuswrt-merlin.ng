/*
 * Broadcom Dongle Host Driver (DHD), Linux monitor network interface
 *
 * Copyright (C) 2023, Broadcom. All Rights Reserved.
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
#include <bcmstdlib_s.h>
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
extern int dhd_start_xmit(struct sk_buff *skb, struct net_device *net);
int dhd_pcap_del(dhd_pub_t *pub);
int dhd_pcap_init(dhd_pub_t *pub);
int dhd_pcap_uninit(dhd_pub_t *pub);
struct dhd_info;
extern void *dhd_ref_deferred_wq(struct dhd_info *dhd);

/**
 * Local declarations and defintions (not exposed)
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
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 2, 0))
	.ndo_set_rx_mode = dhd_pcap_if_set_multicast_list,
#else
	.ndo_set_multicast_list = dhd_pcap_if_set_multicast_list,
#endif
	.ndo_set_mac_address = dhd_pcap_if_change_mac,
};

/**
 * Local static function defintions
 */

#ifdef WL_MONITOR

/* This function is based off of wlc_recv_compute_rspec().
 * Given a PLCP, it calculates the rate of a received frame and returns it as a ratespec.
 * This funciton also handles a cached plcp (amsdu_pclp) to support PCAP handling of AMSDU
 * frames
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

	/* If this is the last in an AMSDU then we should have a cached plcp from
	 * the first AMSDU, so use it to calculate the rate since we know the frame
	 * type here
	 */
	if (RXHDR_GET_AMSDU(rxhdr) &&
		(RXHDR_GET_AGG_TYPE(rxhdr) == RXS_AMSDU_LAST) &&
			dhdp->monitor_info->amsdu_plcp_valid) {
		plcp = dhdp->monitor_info->amsdu_plcp;
		dhdp->monitor_info->amsdu_plcp_valid = FALSE;
	}

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
		DHD_ERROR(("%s: EHT format %d with mcs %d nss %d, where nss should be 1\n",
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
		PCAP_RESET_MON_INFO_PLCP(dhdp->monitor_info);
		return -1;
	}

	/*
	 ************
	 * pkt_info fields
	 ************
	 */
	pkt_info.marker = 0; /* Doesn't look like this is used */

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

	/* Pick up a pointer to the PLCP in the Rx status header area */
	plcp = (uint8 *)(((uint8 *)rxhdr) + phyextract.hwrxoff);

	rspec = OFDM_RSPEC(WLC_RATE_6M);

	/* If we have a valid ucode status, we know the frame type and can compute the
	 * rate from the plcp (this should always be the case except for first/middle
	 * amsdus).
	 * If not, save the plcp of the first AMSDU to use later to determine the rate
	 */
	if (RXS_GE129_VALID_UCODE_STS(rxhdr, dhdp->monitor_info->corerev)) {
		rspec = dhd_recv_compute_rspec(dhdp, rxhdr, plcp);
	} else {
		/* If this a first amsdu, cache the plcp, we will need it to calculate the rate when
		 * the last amsdu status is recvd.  Only the firs has the plcp included
		 */
		if (RXHDR_GET_AMSDU(rxhdr) &&
			(RXHDR_GET_AGG_TYPE(rxhdr) == RXS_AMSDU_FIRST)) {
			bcopy(plcp, dhdp->monitor_info->amsdu_plcp,
					sizeof(dhdp->monitor_info->amsdu_plcp));
			dhdp->monitor_info->amsdu_plcp_valid = TRUE;
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
			dhdp->monitor_skb = NULL;
			return -1;
		}
	}

	if (pkt_type == MON_PKT_AMSDU_INTERMEDIATE || pkt_type == MON_PKT_AMSDU_LAST) {
		PKTFREE(dhdp->osh, pcap_pkt, FALSE);
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
/**
 * Send Tx packets to the radiotap interface
 *
 * *mac_header  : Pointer to the 802.11 Mac header
 * *pcap_tx_hdr : Pointer to the structure that contains Tx status information from the dongle
 * *rtap_header : Pointer to where to build the radiotap header
 */
static uint
dhd_pcap_to_rtap_txcpy(struct dot11_header *mac_header, pcap_tx_hdr_t *pcap_tx_hdr,
		void *rtap_header)
{
	int channel_frequency;
	uint32 channel_flags;
	uint8 flags;
	uint16 fc = LTOH16(mac_header->fc);
	uint bsd_header_len;
	uint16 chanspec = LTOH16(pcap_tx_hdr->chanspec);

	pcap_tx_hdr->chanspec = chanspec;

	/* Do some basic sanity checking of the pcap_tx_hdr, bail if it looks funny */
	if (wf_chspec_malformed(pcap_tx_hdr->chanspec)) {
		DHD_PKT_PCAP(("%s() bad channel spec 0x%x\n", __FUNCTION__, pcap_tx_hdr->chanspec));
		return (0);
	}

	if (CHSPEC_IS2G(pcap_tx_hdr->chanspec)) {
		channel_flags = IEEE80211_CHAN_2GHZ;
		channel_frequency = wf_channel2mhz(wf_chspec_ctlchan(pcap_tx_hdr->chanspec),
			WF_CHAN_FACTOR_2_4_G);
	} else {
		channel_flags = IEEE80211_CHAN_5GHZ;
		channel_frequency = wf_channel2mhz(wf_chspec_ctlchan(pcap_tx_hdr->chanspec),
			WF_CHAN_FACTOR_5_G);
	}

	flags = 0;

	if ((fc &  FC_WEP) == FC_WEP)
		flags |= IEEE80211_RADIOTAP_F_WEP;

	if ((fc & FC_MOREFRAG) == FC_MOREFRAG)
		flags |= IEEE80211_RADIOTAP_F_FRAG;

	if ((LTOH16(pcap_tx_hdr->datarate) != 0) || 1) /* TODO */
	{
		uint32 field_map = WL_RADIOTAP_PRESENT_TX;
		struct wl_radiotap_hdr_tx *rtl = (struct wl_radiotap_hdr_tx *)rtap_header;

		/*
		 * Header length is complicated due to dynamic presence of signal and noise fields
		 * Start with length of wl_radiotap_legacy plus signal/noise/ant
		 */
		bsd_header_len = sizeof(struct wl_radiotap_hdr_tx);
		bzero((uint8 *)rtl, sizeof(struct wl_radiotap_hdr_tx));

		rtl->ieee_radiotap.it_version = 0;
		rtl->ieee_radiotap.it_pad = 0;
		rtl->ieee_radiotap.it_len = HTOL16(bsd_header_len);
		rtl->ieee_radiotap.it_present = HTOL32(field_map);

		rtl->tsft = LTOH32(pcap_tx_hdr->tsf_lo);
		rtl->tsft = (rtl->tsft << 32);
		rtl->tsft += LTOH32(pcap_tx_hdr->tsf_hi);

		rtl->flags = flags;
		rtl->u.rate = LTOH16(pcap_tx_hdr->datarate);
		rtl->channel_freq = HTOL16(channel_frequency);
		rtl->channel_flags = HTOL16(channel_flags);
	} else {
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
	struct wl_radiotap_hdr_tx *rtap_hdr;

	monitor_info = dhdp->monitor_info;

	switch (pcap_type) {
		case PCAP_PKT_TX_COPY_AMSDU_FIRST:
			/* Make room for the radiotap header */
			rtap_hdr = (struct wl_radiotap_hdr_tx *)
					skb_push(pcap_skb, sizeof(struct wl_radiotap_hdr_tx));
			ret_val = dhd_pcap_to_rtap_txcpy(mac_header, pcap_tx_hdr, rtap_hdr);
			if (ret_val != 0) {
				/* Save the new starting AMSDU subframe */
				monitor_info->tx_amsdu_len = pcap_skb->len;
				monitor_info->tx_amsdu_pkt = pcap_skb->data;
				ret_val = TXCPY_AMSDU_REASSEM_KEEP_SKB;
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
					skb_put(dhdp->tx_monitor_skb, pcap_skb->len);
					memcpy((monitor_info->tx_amsdu_pkt +
							monitor_info->tx_amsdu_len),
							pcap_skb->data, pcap_skb->len);
					monitor_info->tx_amsdu_len += pcap_skb->len;

					if (pcap_type == PCAP_PKT_TX_COPY_AMSDU_LAST) {
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
	struct wl_radiotap_hdr_tx *rtap_hdr;
	void *d11_mac_hdr;
	uint8 pcap_type;
	uint amsdu_status;
	bool is_amsdu;

	/* Sanity check the pcap_skb before using it */
	ASSERT(pcap_skb != NULL);

	is_amsdu = FALSE;

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
				if (dhdp->pcap.pcap_config & WLC_PCAP_TX_FIRST_MSDU) {
					pcap_skb->dev->stats.rx_packets++;
					skb_reset_mac_header(dhdp->tx_monitor_skb);
					netif_rx(dhdp->tx_monitor_skb);
				} else {
					/*
					 * This must mean we didn't see a LAST before this
					 * First, so ditch the in-progress AMSDU reassembly
					 */
					PKTFREE(dhdp->osh, dhdp->monitor_info->tx_amsdu_pkt, TRUE);
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
		if (!dhdp->tx_monitor_skb) {
			dhdp->tx_monitor_skb = pcap_skb;
		}

		/* Make sure we have enough room to add in another possibly max sized amsdu */
		if (skb_tailroom(dhdp->tx_monitor_skb) < PCAP_PKT_BUFSZ) {
			DHD_PKT_PCAP(("%s() Not enough tailroom %d need %d\n", __FUNCTION__,
					skb_tailroom(dhdp->tx_monitor_skb), PCAP_PKT_BUFSZ));
			if (dhdp->tx_monitor_skb != pcap_skb) {
				PKTFREE(dhdp->osh, dhdp->tx_monitor_skb, TRUE);
			}
			PKTFREE(dhdp->osh, pcap_skb, TRUE);
			dhdp->tx_monitor_skb = NULL;
			dhdp->monitor_info->tx_amsdu_len = 0;
			dhdp->monitor_info->tx_amsdu_pkt = NULL;
			return -1;
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
				kfree_skb(pcap_skb);
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
		/* Make room for the radiotap header */
		rtap_hdr = (struct wl_radiotap_hdr_tx *)
			skb_push(pcap_skb, sizeof(struct wl_radiotap_hdr_tx));

		if (dhd_pcap_to_rtap_txcpy(d11_mac_hdr,
				(pcap_tx_hdr_t *)(&local_pcap_tx_hdr), rtap_hdr))
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
dhd_pcap_tap(void *handle, void *event_info, enum dhd_wq_event event)
{
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 19, 0))
	struct sk_buff *pcap_skb, *n;
	unsigned long flags;
	int getnextpkt;

	dhd_pub_t *pub = (dhd_pub_t *) event_info;

	list_for_each_entry_safe(pcap_skb, n, &pub->pcap.skb_list, list) {
		DHD_PCAP_LOCK(pub->pcap.skb_list_lock, flags);
		list_del(&pcap_skb->list);
		DHD_PCAP_UNLOCK(pub->pcap.skb_list_lock, flags);

		/*
		 * normally the SKBs that come here are unique copies and should
		 * have a ref count of 1. But if the code path changes and we
		 * receive a shared skb, we cannot expand the header for it
		 * (will crash) so skip these SKBs.
		 */
		if (skb_shared(pcap_skb)) {
			continue;
		}
		/* send the packet to the radiotap interface */
		pcap_skb->dev = pub->pcap.pcap_if.pcap_ndev;
		pcap_skb->next = NULL;

		switch (PCAP_SKB_PKTTYPE(pcap_skb)) {
			case PCAP_PKT_RX_PROMISC:
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

#endif /* LINUX_VERSION_CODE >= KERNEL_VERSION(4, 19, 0) */
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
		ret = dhd_start_xmit(skb, pcap_if->real_ndev);

		return ret;
	}
fail:
	kfree_skb(skb);
	return 0;
}
#else
static inline void
dhd_pcap_tap(void *handle, void *event_info, enum dhd_wq_event event)
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
		out_skb = skb_copy(pkt, GFP_KERNEL);
	}

	if (out_skb) {
		PCAP_SKB_PKTTYPE(out_skb) = pcap_pkt_type;
	}
	return out_skb;
}

#ifdef BCM_PCAP
/*
 * yet another abstraction to duplicate a PKT into an SKB, but this allocates a whole new fresh
 * skb.  This function includes support for creating an out_skb with a larger data section (size).
 *
 */
struct sk_buff*
dhd_pcap_dup(dhd_pub_t *pub, void *pkt, uint16 size, uint8 pcap_pkt_type)
{
	struct sk_buff *out_skb;
	struct sk_buff *in_skb = (struct sk_buff *)pkt;
	int pcap_hdr_adj_len;

	BCM_REFERENCE(pub);

#if defined(BCM_NBUFF_PKT_BPM)
	if (IS_FKBUFF_PTR(pkt)) {
		/*
		 * If the fkb packet data buffer size is less than the requeested size, we need to
		 * allocate a new bigger skb and copy the data from the fkb data buffer into the new
		 * skb then free the input fkb
		 */

		if (BCM_MAX_PKT_LEN < size) {
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
		} else {
			out_skb = dhd_xlate_to_skb(pub, pkt);
		}

	} else
#endif /* BCM_NBUFF_PKT_BPM */
	{
		if (size > in_skb->len) {
			out_skb = skb_copy_expand(in_skb, skb_headroom(in_skb),
					skb_tailroom(in_skb) + size - in_skb->len, GFP_KERNEL);
		} else {
			out_skb = skb_copy(pkt, GFP_KERNEL);
		}
	}
	if (out_skb == NULL) {
		DHD_ERROR(("%s() NULL skb!\n", __FUNCTION__));
	} else {
		/* Trim the out_skb if the size is smaller than the output buffer */
		skb_trim(out_skb, size);

		/* If this is a dup for a TX_COPY pkt, then we need to move the skb data
		 * pointer back to the begining of the pcap_tx_hdr_ext_t.
		 * Due to how the snap header is pushed in to the packet, the data pointer
		 * actually points 6-bytes (an enet address) before the snap header, hence
		 * that adjustment here
		 */
		switch (pcap_pkt_type) {
			case PCAP_PKT_TX_COPY:
				pcap_hdr_adj_len = sizeof(pcap_tx_hdr_ext_t) - ETHER_ADDR_LEN;
				break;

			case PCAP_PKT_TX_COPY_AMSDU_FIRST:
				/* First AMSDU has a pcap_tx header */
				pcap_hdr_adj_len = sizeof(pcap_tx_hdr_ext_t) + ETHER_ADDR_LEN + 2;
				break;

			case PCAP_PKT_TX_COPY_AMSDU_INTRM:
			case PCAP_PKT_TX_COPY_AMSDU_LAST:
				/* Non-First AMSDU do not have a pcap_tx header */
				pcap_hdr_adj_len = ETHER_ADDR_LEN + 2;
				break;

			case PCAP_PKT_TX_LOCAL_COPY:
			case PCAP_PKT_RX_PROMISC:
			case PCAP_PKT_RX_COPY:
				pcap_hdr_adj_len = 0;
				break;
			default:
				DHD_ERROR(("%s() : Unexpected pcap_pkt_type 0x%x\n", __FUNCTION__,
						pcap_pkt_type));
				pcap_hdr_adj_len = 0;
				break;
		}

		PCAP_SKB_PKTTYPE(out_skb) = pcap_pkt_type;

		if (pcap_hdr_adj_len) {
			/* We should always have enought headroom, but just in case... */
			if (skb_headroom(out_skb) >= pcap_hdr_adj_len) {
				skb_push(out_skb, pcap_hdr_adj_len);
			} else {
				PCAP_PRINT("%s() Too little headroom, need %d, have %d\n",
						__FUNCTION__,
						pcap_hdr_adj_len, skb_headroom(out_skb));
				kfree_skb(out_skb);
				out_skb = (struct sk_buff *)NULL;
			}
		}
	}

	return (out_skb);
}
#endif /* BCM_PCAP */

#define DHD_PCAP_FORCE_SCHEDULE_TOUT_MS 10000

static inline bool
dhd_pcap_need_to_schedule(dhd_pub_t *pub)
{
	uint32 now = OSL_SYSUPTIME();
	if ((pub->pcap.last_scheduled == 0) && now <= DHD_PCAP_FORCE_SCHEDULE_TOUT_MS) {
		return FALSE;
	}
	/* might happen once every ~50 days */
	if (now < pub->pcap.last_scheduled) {
		return TRUE;
	}
	/* now is now guaranteed to be >= last_scheduled */
	return (now - pub->pcap.last_scheduled) > DHD_PCAP_FORCE_SCHEDULE_TOUT_MS;
}

void
dhd_pcap_queue(dhd_pub_t *pub, struct sk_buff *skb)
{
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 19, 0))
	bool schedule = false;
	unsigned long flags;
	/*
	 * If the pcap list is not empty, it's likely that a deferred work has
	 * already been scheduled or is working on the list. Defer the deferral
	 * (heh, I know) until that tasklet completes the work. But if we have
	 * not scheduled a deferred work for 10 seconds, also force a new
	 * tasklet to catch any edge cases.
	 */
	if (list_empty(&pub->pcap.skb_list)) {
		schedule = true;
	} else if (dhd_pcap_need_to_schedule(pub)) {
		PCAP_PRINT("Force pcap processing: %u, %u\n",
				OSL_SYSUPTIME(),
				pub->pcap.last_scheduled);
		schedule = true;
	}

	DHD_PCAP_LOCK(pub->pcap.skb_list_lock, flags);
	list_add_tail(&skb->list, &pub->pcap.skb_list);
	DHD_PCAP_UNLOCK(pub->pcap.skb_list_lock, flags);

	if (schedule) {
		if (dhd_deferred_schedule_work(dhd_ref_deferred_wq(pub->info),
				pub,
				DHH_WQ_WORK_PCAP_TAP,
				dhd_pcap_tap,
				DHD_WORK_PRIORITY_LOW) == DHD_WQ_STS_OK) {
			pub->pcap.last_scheduled = OSL_SYSUPTIME();
		} else {
			PCAP_PRINT("Failed to schedule defered work at %u (last %u)\n",
					OSL_SYSUPTIME(), pub->pcap.last_scheduled);
		}

	}
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
                       uint8 *rxphysts, uint16 physts_len)
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
		dhd_pcap_queue(dhd_pub, data_skb);

		dhd_pub->pcap.rxstats.rxpkt_enqd++;
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

	ndev = alloc_etherdev(sizeof(dhd_linux_pcap_t*));

	if (!ndev) {
		return -ENOMEM;
	}

	ndev->type = ARPHRD_IEEE80211_RADIOTAP;
	strncpy(ndev->name, name, IFNAMSIZ);
	ndev->name[IFNAMSIZ - 1] = 0;
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

#ifdef WL_MONITOR
	ret = bcmwifi_monitor_create(&pub->monitor_info);
	if (ret == TRUE) {
		pub->monitor_info->corerev = dhd_macdbg_corerev(pub);
		pub->monitor_info->pcap = TRUE;
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
#if defined(BCM_PCAP)
		pcap_hdr_list_info_t *pkt_list;
#endif /* BCM_PCAP */
		dhd_pub->pcap.pcap_state = PCAP_STATE_INIT;
		INIT_LIST_HEAD(&dhd_pub->pcap.skb_list);
		dhd_pub->pcap.skb_list_lock = dhd_os_spin_lock_init(dhd_pub->osh);
		if (dhd_pub->pcap.skb_list_lock == NULL) {
			ASSERT(dhd_pub->pcap.skb_list_lock);
			DHD_ERROR(("dhd_pcap_init: lock alloc failure\n"));
			return BCME_NOMEM;
		}
		/* Init rx-cpy storage */
#if defined(BCM_PCAP)
		pkt_list = &dhd_pub->pcap.pcap_list_info;
		pkt_list->frm_list_rd = 0;
		pkt_list->frm_list_wr = 0;
		pkt_list->d11hdr_list_rd = 0;
		pkt_list->d11hdr_list_wr = 0;
#ifdef BCM_PCAP_PHYSTS
		pkt_list->phy_list_rd = 0;
		pkt_list->phy_list_wr = 0;
#endif /* BCM_PCAP_PHYSTS */
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
		if (dhd_pub->pcap.skb_list_lock) {
			dhd_os_spin_lock_deinit(dhd_pub->osh, dhd_pub->pcap.skb_list_lock);
		}
	}
	dhd_pub->pcap.pcap_state = PCAP_STATE_DEINIT;
	return 0;
}
