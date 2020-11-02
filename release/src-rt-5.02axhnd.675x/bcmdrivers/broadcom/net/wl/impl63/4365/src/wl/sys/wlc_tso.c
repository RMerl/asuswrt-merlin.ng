/*
 * TSO (TCP Segmentation/Checksumming Offload) support source file
 * Broadcom 802.11abg Networking Device Driver
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
 * $Id: wlc_tso.c 708017 2017-06-29 14:11:45Z $
 */

/**
 * @file
 * @brief
 * The D11ac core contains a Rx offload engine (ROE) that offloads IP and TCP checksum calculations
 * for received frames, and a Tx offload engine that can perform CheckSum Offload (CSO) and Tx
 * Segmentation Offload (TSO) functions.
 */

/* XXX: Define wlc_cfg.h to be the first header file included as some builds
 * get their feature flags thru this file.
 */

#include <wlc_cfg.h>

#ifdef WLTOEHW
#include <typedefs.h>
#include <bcmdefs.h>
#include <osl.h>
#include <bcmutils.h>
#include <siutils.h>
#include <bcmendian.h>
#include <proto/802.1d.h>
#include <proto/802.11.h>
#include <wlioctl.h>
#include <bcmwpa.h>

#include <d11.h>
#include <wlc_rate.h>
#include <wlc_pub.h>
#include <wlc_keymgmt.h>
#include <wlc_bsscfg.h>
#include <wlc.h>

#include <wlc_scb.h>
#include <wlc_frmutil.h>
#include <wlc_tso.h>

#ifdef WLCSO

static uint8* pktdataoffset_delta(osl_t *osh, pkt_data_ptr_t* pdt,  uint delta);
static uint16 csum_pseudohdr(uint8 *tcph, uint32 sip, uint32 dip, uint8 proto,
	uint16 proto_len);
static uint16 csum_inet(void *dataptr, uint16 len);
static uint16 csum_with_pseudohdr(uint8 *tcph, uint32 sip, uint32 dip,
	uint8 proto, uint16 proto_len);

static uint8 *
pktdataoffset_delta(osl_t *osh, pkt_data_ptr_t* pdt,  uint delta)
{
	void *p = pdt->cur_p;
	void *pt = pdt->cur_d;
	uint8 *pdata = (uint8 *) PKTDATA(osh, p);
	uint offset = (uint) ((uint8*)pt - pdata), len = 0;
	uint pkt_off = 0;

	ASSERT(delta > 0);

	offset += delta;
	for (; p; p = PKTNEXT(osh, p)) {
		pdata = (uint8 *) PKTDATA(osh, p);
		pkt_off = offset - len;
		len += PKTLEN(osh, p);
		if (len > offset)
			break;
	}

	pt = (void*) (pdata+pkt_off);
	pdt->cur_p = p;
	pdt->cur_d = pt;

	return (uint8*)pt;
}

#ifdef PKTSIZ_FIXUP
static void
wlc_tso_pktsize_fixup(osl_t *osh, void* p)
{
	void *pt, *ptnext;
	void *pnext;
	uint len;
	uint adjlen = 0;

	len = PKTLEN(osh, p);
	if (len % 4)
		adjlen = 4 - len % 4;

	if (adjlen && ((uint)PKTTAILROOM(osh, p) >= adjlen) && (pnext = PKTNEXT(osh, p))) {
		PKTSETLEN(osh, p, len + adjlen);
		pt = PKTDATA(osh, p) + len;
		ptnext = PKTDATA(osh, pnext);
		bcopy(ptnext, pt, adjlen);
		PKTPULL(osh, pnext, adjlen);
	}
}
#endif /* PKTSIZ_FIXUP */

static int
wlc_tso_parse_d11(wlc_info_t *wlc, d11ac_tso_t* tso, void* pt, void* p, struct scb *scb,
	const wlc_key_info_t *key_info, uint nfrags, uint offset)
{
	struct dot11_header * h = (struct dot11_header *) pt;
	uint phylen = 0, tot_len = 0;
	uint16 fc, type;
	wlc_pkttag_t *pkttag;
	bool qos;
	osl_t *osh = wlc->osh;
	wlc_bsscfg_t *bsscfg;

	bsscfg = SCB_BSSCFG(scb);
	ASSERT(bsscfg != NULL);

	tot_len = pkttotlen(osh, p);

	if ((tot_len - offset) < DOT11_A3_HDR_LEN) {
		WL_TSO(("wl%d: %s: short d11 frame (%d)\n",
			wlc->pub->unit, __FUNCTION__, tot_len - offset));
		return -1;
	}

	pkttag = WLPKTTAG(p);
	fc = ltoh16(h->fc);
	type = FC_TYPE(fc);
	qos = (type == FC_TYPE_DATA && FC_SUBTYPE_ANY_QOS(FC_SUBTYPE(fc)));

	/* if it is A3 or A4 */
	if ((fc & FC_TODS) && (fc & FC_FROMDS))
		phylen += DOT11_A4_HDR_LEN;
	else
		phylen += DOT11_A3_HDR_LEN;

	if (qos)
		phylen += DOT11_QOS_LEN;

	/* add room for iv, icv and tkip mic to phylen */
	if (key_info != NULL) {
		phylen += key_info.icv_len;

		/* external crypto adds iv to the pkt, include it in phylen */
		if (WLC_KEY_IS_LINUX_CRYPTO(key_info))
			phylen += key_info->iv_len;

		if (WLC_KEY_FRAG_HAS_TKIP_MIC(p, key_info, frag, nfrags))
			phylen += TKIP_MIC_SIZE;
	}

#ifdef DEBUG_TSO
	prhex("d11 hdr dump:", (uint8 *)h, phylen);
#endif // endif

	return phylen;
}

static int
wlc_tso_parse_eth(wlc_info_t *wlc, void* pt, void* p, uint offset)
{
	int len = 0, remainder = 0;
	uint tot_len = pkttotlen(wlc->osh, p);

	const uint8 llc_snap_hdr[SNAP_HDR_LEN] = {0xaa, 0xaa, 0x03, 0x00, 0x00, 0x00};

	remainder = tot_len - offset;

	WL_TSO(("wl%d: %s: remainder: %d\n",
		wlc->pub->unit, __FUNCTION__, remainder));
	/* Process Ethernet II or SNAP-encapsulated 802.3 frames */
	if (remainder < ETHER_TYPE_LEN) {
		WL_TSO(("wl%d: %s: short eth frame (%d)\n",
		          wlc->pub->unit, __FUNCTION__, remainder));
		len = -1;
	} else if (ntoh16(*(uint16 *)pt) >= ETHER_TYPE_MIN &&
		bcmp(llc_snap_hdr, pt, SNAP_HDR_LEN)) {
		/* Frame is Ethernet II */
		WL_TSO(("wl%d: %s: ethernet II (%d)\n",
		          wlc->pub->unit, __FUNCTION__, remainder));
	} else if (remainder >= SNAP_HDR_LEN + ETHER_TYPE_LEN &&
	           !bcmp(llc_snap_hdr, pt, SNAP_HDR_LEN)) {
		WL_TSO(("wl%d: %s: 802.3 LLC/SNAP\n", wlc->pub->unit, __FUNCTION__));
		len += SNAP_HDR_LEN;
	} else {
		WL_TSO(("wl%d: %s: non-SNAP 802.3 frame\n",
			wlc->pub->unit, __FUNCTION__));
		len = -1;
	}

#ifdef DEBUG_TSO_ETH
	prhex("eth hdr dump:", (uint8 *)pt, ((len > 0)? len : SNAP_HDR_LEN));
#endif // endif
	return len;
}

static bool
wlc_tso_parse_ip(wlc_info_t *wlc, d11ac_tso_t *tso, void* pt, void* p, uint offset, bool sw_only)
{
	struct ipv4_hdr *iph;
	uint16 ihl = 0, ipl = 0;
	uint8 prot = 0;
	bool passthrough = FALSE;
	int payload_len = 0;
	uint payload_offset = 0;
	uint32 sip, dip;
	uint16* csum_ptr = NULL;
	uint8* csum_start = NULL;
#ifdef DEBUG_TSO_IP
	uint16 sw_csum = 0, p_csum = 0;
#endif // endif

	iph = (struct ipv4_hdr *)pt;
	tso->ip_hdr_offset = htol16((uint16)offset);

	WL_TSO(("wl%d: %s: sw_only %d\n", wlc->pub->unit, __FUNCTION__, sw_only));
#ifdef DEBUG_TSO_IP
	WL_TSO(("wl%d: %s: IP ver: %d IP prot: %d\n",
		wlc->pub->unit, __FUNCTION__, IP_VER(iph), IPV4_PROT(iph)));
#endif // endif

	if (IP_VER(iph) == IP_VER_4) {
		tso->flag[0] |= TOE_F0_IPV4;

		if (IPV4_PROT(iph) == IP_PROT_TCP) {
			prot = IP_PROT_TCP;
			tso->flag[0] |= TOE_F0_TCP;
		}
		else if (IPV4_PROT(iph) == IP_PROT_UDP) {
			prot = IP_PROT_UDP;
			tso->flag[0] |= TOE_F0_UDP;
		}
		else {
			prot = IPV4_PROT(iph);
			WL_TSO(("wl%d: %s: not TCP/UDP frame (prot: 0x%x)\n",
				wlc->pub->unit, __FUNCTION__, prot));
			passthrough = TRUE;
			goto pass;
		}

		tso->flag[1] |= TOE_F1_IPV4_CSUM_EN;
		ipl = ntoh16(iph->tot_len);
		ihl = IPV4_HLEN(iph);

		/* for potential pseudo header calc */
		bcopy(iph->src_ip, (void *)&sip, sizeof(struct ipv4_addr));
		bcopy(iph->dst_ip, (void *)&dip, sizeof(struct ipv4_addr));

		offset += (uint)IPV4_HLEN(iph);
		pt += ihl;

		/* clear out the IPV4 checksum field */
		bzero(&iph->hdr_chksum, sizeof(uint16));
	}
	else if (IP_VER(iph) == IP_VER_6) {
		tso->flag[0] |= TOE_F0_IPV6;

		if (IPV6_PROT(iph) == IP_PROT_TCP) {
			prot = IP_PROT_TCP;
			tso->flag[0] |= TOE_F0_TCP;
		}
		else if (IPV6_PROT(iph) == IP_PROT_UDP) {
			prot = IP_PROT_UDP;
			tso->flag[0] |= TOE_F0_UDP;
		}
		else {
			prot = IPV6_PROT(iph);
			WL_TSO(("wl%d: %s: not TCP/UDP frame (prot: 0x%x)\n",
				wlc->pub->unit, __FUNCTION__, prot));
			passthrough = TRUE;
			goto pass;
		}

		ipl = (uint16)IPV6_PAYLOAD_LEN(iph);
		ihl = (uint16)IPV6_HLEN(iph);

		offset += (uint)IPV6_HLEN(iph);

		pt += ihl;
		ipl = ipl + ihl;
	}

#ifdef DEBUG_TSO_AMSDU
	WL_TSO(("wl%d: %s: ipl: %d IP ihl: %d\n",
		wlc->pub->unit, __FUNCTION__, ipl, ihl));
	prhex("ip header dump:", (uint8 *)iph, ihl);
#endif // endif

	tso->tcp_hdr_offset = htol16((uint16)offset);
	tso->flag[1] |= TOE_F1_TCPUDP_CSUM_EN;

	/* 4360A0 WAR (PR104302) Part 1
	 * SW cannot enable Pseudo Header checksum computation on sub-frames within
	 * a SW_AMSDU
	 */
	if (!(D11REV_IS(wlc->pub->corerev, 40) && WLPKTFLAG_AMSDU(WLPKTTAG(p))))
		tso->flag[1] |=	TOE_F1_PSEUDO_CSUM_EN;

	if (prot == IP_PROT_TCP) {
		/* start of tcp, not necessarily true for ipv6 */
		struct bcmtcp_hdr *tcph = (struct bcmtcp_hdr *)pt;
		int tcpl = (int)(ipl - ihl);
		uint8* hdrlen = (uint8*)&(tcph->hdrlen_rsvd_flags);
		int tcphl = (int)(TCP_HDRLEN(hdrlen[0]) * 4);
		payload_len = (int)(tcpl - tcphl);
		payload_offset = (uint)(offset + tcphl);

		/* clear out the checksum field */
		bzero(&tcph->chksum, sizeof(uint16));
		csum_ptr = &tcph->chksum;
		csum_start = (uint8 *) tcph;

#ifdef DEBUG_TSO_IP
		sw_csum = csum_with_pseudohdr(csum_start, sip, dip, prot, (uint16)(ipl - ihl));
		p_csum = csum_pseudohdr(csum_start, sip, dip, prot, (uint16)(ipl - ihl));

		WL_TSO(("wl%d: %s: sw_csum: 0x%x pseudo hdr csum: 0x%x\n",
			wlc->pub->unit, __FUNCTION__, sw_csum, p_csum));
#endif // endif
		if (D11REV_IS(wlc->pub->corerev, 40) && WLPKTFLAG_AMSDU(WLPKTTAG(p)) &&
			(IP_VER(iph) == IP_VER_4)) {
			if (sw_only) {
				passthrough = TRUE;
				tso->flag[0] &= ~(TOE_F1_TCPUDP_CSUM_EN | TOE_F1_IPV4_CSUM_EN);
				goto pass;
			} else {
				uint16 csum = 0;
				csum = csum_pseudohdr((void *)tcph, sip, dip, IP_PROT_TCP,
					(uint16)tcpl);
				tcph->chksum = (uint16)~csum;
			}
		}
#ifdef DEBUG_TSO_AMSDU
		WL_TSO(("wl%d: %s: TCP: hdrlen: %x  tcphl: %d payload_len: %d\n",
			wlc->pub->unit, __FUNCTION__, *hdrlen, tcphl, payload_len));
		prhex("tcp header dump:", (uint8 *)pt, (tcphl < 20)? 20 : tcphl);
#endif // endif
	}
	else if (prot == IP_PROT_UDP) {
		struct bcmudp_hdr *udph = (struct bcmudp_hdr *)pt;
		uint16 udpl = ntoh16(udph->len);
		uint16 udphl = (uint16)UDP_HDR_LEN;
		payload_len = (int)(udpl - udphl);
		payload_offset = (uint)(offset + udphl);

		/* clear out the checksum field */
		bzero(&udph->chksum, sizeof(uint16));
		csum_ptr = &udph->chksum;
		csum_start = (uint8 *)udph;

#ifdef DEBUG_TSO_IP
		sw_csum = csum_with_pseudohdr(csum_start, sip, dip, prot, (uint16)(ipl - ihl));
		p_csum = csum_pseudohdr(csum_start, sip, dip, prot, (uint16)(ipl - ihl));

		WL_TSO(("wl%d: %s: sw_csum: 0x%x pseudo hdr csum: 0x%x\n",
			wlc->pub->unit, __FUNCTION__, sw_csum, p_csum));
#endif // endif
		if (D11REV_IS(wlc->pub->corerev, 40) && WLPKTFLAG_AMSDU(WLPKTTAG(p)) &&
			(IP_VER(iph) == IP_VER_4)) {
			if (sw_only) {
				passthrough = TRUE;
				tso->flag[0] &= ~(TOE_F1_TCPUDP_CSUM_EN | TOE_F1_IPV4_CSUM_EN);
				goto pass;
			} else {
				uint16 csum = 0;
				csum = csum_pseudohdr((void *)udph, sip, dip, IP_PROT_UDP,
					(uint16)udpl);
				udph->chksum = (uint16)~csum;
			}
		}

#ifdef DEBUG_TSO_IP
		WL_TSO(("wl%d: %s: UDP: hdrlen: %d payload_len: %d udph->len: 0x%x\n",
			wlc->pub->unit, __FUNCTION__, udphl, payload_len, udph->len));
		prhex("udp header dump:", (uint8 *)pt, UDP_HDR_LEN);
#endif // endif
	}

	if (payload_len < 0) {
		WL_TSO(("wl%d: %s: invalid payload length: %d\n",
			wlc->pub->unit, __FUNCTION__, payload_len));
		passthrough = TRUE;
		goto pass;
	}

	/* for now tso_mss is same as payload size, change when tso is enabled */

	tso->tso_payload_siz = htol32((uint32)payload_len);
	tso->tso_mss = htol16((uint16)payload_len);

	/* 4360A0 WAR (PR104299)
	 * Do not enable csum for 0 payload length in a sw amsdu sequence
	 */
	if (D11REV_IS(wlc->pub->corerev, 40) && (payload_len == 0) &&
		WLPKTFLAG_AMSDU(WLPKTTAG(p))) {
		WL_TSO(("wl%d: %s: invalid payload len in sw_amsdu: %d\n",
			wlc->pub->unit, __FUNCTION__, payload_len));
		passthrough = TRUE;
		goto pass;
	}

	/* 4360A0 WAR (PR104342)
	 * tcp/udp payload offset (relative to start of TxD Header) <= 256
	 */
	if (D11REV_IS(wlc->pub->corerev, 40) && (payload_offset > 256)) {
		WL_TSO(("wl%d: %s: invalid payload offset: %d\n",
			wlc->pub->unit, __FUNCTION__, payload_offset));
		passthrough = TRUE;
		goto pass;
	}

	/* 4360A0 WAR (PR104295)
	 * ensure the offset for payload is on a word boundary if
	 * payload_len % mss == 3
	 */
	if (D11REV_IS(wlc->pub->corerev, 40) && (payload_len % 4 == 3) &&
		(payload_offset % 4)) {
		WL_TSO(("wl%d: %s: invalid payload offset (boundary) %d\n",
			wlc->pub->unit, __FUNCTION__, payload_offset));
		passthrough = TRUE;
		goto pass;
	}

pass:
	/* fall back mechanism here ?? */
	if (passthrough && (prot == IP_PROT_TCP || prot == IP_PROT_UDP)) {
		iph->hdr_chksum = csum_inet(iph, ihl);
		*csum_ptr = 0;
		*csum_ptr = csum_with_pseudohdr(csum_start, sip, dip, prot, (uint16)(ipl - ihl));
		WL_TSO(("wl%d: %s: iphdr: 0x%x, sw_csum: 0x%x\n", wlc->pub->unit, __FUNCTION__,
			(uint32)iph->hdr_chksum, (uint32)*csum_ptr));
	}
	return passthrough;

}

static int
wlc_tso_parse_amsdu(wlc_info_t *wlc, d11ac_tso_t *tso, void* pt, void* p, uint offset)
{
	int ret = 0;
	struct ether_header *eh;
	uint num_sf = 0;
	uint16 sflen = 0, len = 0, tmplen, pad;
	void *newpkt;
	void *newpkt0;
	int resid = 0;
	bool passthrough = TRUE, all_pass = FALSE;
	bool csum_needed = FALSE;
	wlc_pkttag_t *pkttag;
	uint delta = 0;
	pkt_data_ptr_t pdt;
	pkt_data_ptr_t *pdt_p = &pdt;
	uint16 ethertype;

	/* FOR AMSDU
	 *  each subframe is in the form of | 8023hdr (SFH) | body | pad |
	 *  subframe other than the last one may have pad bytes
	 * The top TSO header is actually for the first subframe, from the second subframe
	 *  and on, a TSO header needs to be inserted before SFH
	 *
	 * TxH TxD D11 ... SFH1 | body1 ->  TxH2 SFH2 | body2 -> ... TxHn SFHn | bodyn
	 *
	 * body format: llc_snap_hdr + eth_type] + [ip + tcp/udp]
	 */

	ASSERT(WLPKTFLAG_AMSDU(WLPKTTAG(p)));

	newpkt = pktoffset(wlc->osh, p, offset);
	if (newpkt) {
		resid = pkttotlen(wlc->osh, newpkt);
		WL_TSO(("wl%d: %s newpkt: len: %d, resid: %d \n", wlc->pub->unit, __FUNCTION__,
			PKTLEN(wlc->osh, newpkt), resid));
	}

	pdt_p->cur_p = newpkt;
	pdt_p->cur_d = pt;

	while (newpkt != NULL) {

		WL_TSO(("wl%d: %s: num_sf: %d\n", wlc->pub->unit, __FUNCTION__, num_sf));
#ifdef DEBUG_TSO_AMSDU
		prhex("amsdu dump:", (uint8 *)pt, PKTLEN(wlc->osh, newpkt));
#endif // endif
		newpkt0 = newpkt;
		csum_needed = FALSE;
		passthrough = TRUE;
		pkttag = WLPKTTAG(newpkt);
		eh = (struct ether_header*) pt;

		if ((((uintptr)eh + (uint)ETHER_HDR_LEN) % 4)  != 0) {
			WL_ERROR(("%s: sf body is not 4 bytes aligned!\n", __FUNCTION__));
			ret = -1;
			goto pass;
		}

		if (num_sf == 0)
			tso->sfh_hdr_offset = (uint8)offset;

		pt =  pktdataoffset_delta(wlc->osh, pdt_p, ETHER_HDR_LEN);
		tmplen = ETHER_HDR_LEN;
		offset += ETHER_HDR_LEN;

		if ((delta = wlc_tso_parse_eth(wlc, pt, p, offset)) < 0) {
			passthrough = TRUE;
			goto pass;
		}

		pt = pktdataoffset_delta(wlc->osh, pdt_p, delta);
		newpkt = pdt_p->cur_p;
		tmplen += delta;
		offset += delta;

		ethertype = ntoh16(*(uint16 *)pt);
#ifdef DEBUG_TSO_AMSDU
		WL_TSO(("wl%d: %s: ethertype: 0x%4x\n",
			wlc->pub->unit, __FUNCTION__, ethertype));
#endif // endif
		/* Skip VLAN tag, if any */
		if (ethertype == ETHER_TYPE_8021Q) {
			pt = pktdataoffset_delta(wlc->osh, pdt_p, VLAN_TAG_LEN);
			tmplen +=  VLAN_TAG_LEN;
			offset +=  VLAN_TAG_LEN;
			ethertype = ntoh16(*(uint16 *)pt);
		}

		if ((ethertype != ETHER_TYPE_IP) && (ethertype != ETHER_TYPE_IPV6)) {
			WL_TSO(("wl%d: %s: non-IP frame (ethertype 0x%x)\n",
				wlc->pub->unit, __FUNCTION__, ethertype));
			passthrough = TRUE;
			/* this is a valid case ?? */
		}

		pt = pktdataoffset_delta(wlc->osh, pdt_p, ETHER_TYPE_LEN);
		newpkt = pdt_p->cur_p;
		tmplen += ETHER_TYPE_LEN;
		offset += ETHER_TYPE_LEN;

		len = (uint16)PKTLEN(wlc->osh, newpkt);
		sflen = ntoh16(eh->ether_type) + ETHER_HDR_LEN;

		if (len < sflen)
			len += tmplen;
		pad = len - sflen;

#ifdef DEBUG_TSO_AMSDU
		WL_TSO(("wl%d: %s: len: %d, sflen: %d pad: %d \n", wlc->pub->unit, __FUNCTION__,
			len, sflen, pad));
#endif // endif

		/* last MSDU: has FCS, but no pad, other MSDU: has pad, but no FCS */
		if (len != (PKTNEXT(osh, newpkt) ? ROUNDUP(sflen, 4) : sflen)) {
			WL_ERROR(("%s: len mismatch buflen %d sflen %d, sf %d\n",
				__FUNCTION__, len, sflen, num_sf));
			ret = -1;
			goto pass;
		}

		/* for first sf, use the tso hdr passed in */
		if (num_sf == 0) {
			int len_new;

			if (pkttag->flags2 & WLF2_HW_CSUM)
				csum_needed = TRUE;

			/* 4360A0 WAR: do passthrough if subframe > 1 */
			if (D11REV_IS(wlc->pub->corerev, 40) && WLPKTFLAG_AMSDU(WLPKTTAG(p)) &&
				(resid > len))
				all_pass = TRUE;
			else
				all_pass = FALSE;

			if (csum_needed)
				passthrough = wlc_tso_parse_ip(wlc, tso, pt, p, offset, all_pass);

			if (csum_needed && !passthrough) {
				tso->flag[2] |= TOE_F2_AMSDU_CSUM_EN;
				tso->flag[0] |= TOE_F0_HDRSIZ_NORMAL;

				tso->tso_payload_siz += htol32((uint32)pad);
				tso->tso_mss += htol16((uint16)pad);

				/* is it possible to have a single frame amsdu ?? */
				if (resid == len)
					tso->flag[2] |= TOE_F2_AMSDU_FS_LAST;

				WLCNTINCR(wlc->pub->_cnt->cso_normal);
			}
			else {
				bzero(tso, TSO_HEADER_PASSTHROUGH_LENGTH);
				tso->flag[0] |= TOE_F0_PASSTHROUGH;
				WLCNTINCR(wlc->pub->_cnt->cso_passthrough);
			}

			len_new = wlc_tso_hdr_length(tso);
#ifdef DEBUG_TSO_AMSDU
			WL_TSO(("wl%d: %s: subframe: %d csum_needed: %d passthrough: %d\n",
			wlc->pub->unit, __FUNCTION__, num_sf, csum_needed, passthrough));
			prhex("TSO header dump:", (uint8 *)tso, len_new);
#endif // endif
		}
		else {
			/* for the 2nd sf and on, need to create a new tso hdr */
			d11ac_tso_t tso_new;
			d11ac_tso_t * tsohdr;
			int len_new;

			bzero(&tso_new, sizeof(d11ac_tso_t));

			if (pkttag->flags2 & WLF2_HW_CSUM)
				csum_needed = TRUE;

			/* 4360A0 WAR: do passthrough if subframe > 1 */
			if (D11REV_IS(wlc->pub->corerev, 40) && WLPKTFLAG_AMSDU(WLPKTTAG(p))) {
				all_pass = TRUE;
				WLCNTINCR(wlc->pub->_cnt->cso_passthrough);
			}
			else
				all_pass = FALSE;

			if (csum_needed)
				passthrough = wlc_tso_parse_ip(wlc, &tso_new, pt, p, tmplen,
					all_pass);

			if (!all_pass) {
				if (csum_needed && !passthrough) {
					tso_new.flag[0] |= TOE_F0_HDRSIZ_NORMAL;
					tso_new.flag[2] |= TOE_F2_AMSDU_CSUM_EN;
					tso_new.tso_payload_siz += htol32((uint32)pad);
					tso_new.tso_mss += htol16((uint16)pad);
					WLCNTINCR(wlc->pub->_cnt->cso_normal);
				}
				else {
					tso_new.flag[0] |= TOE_F0_PASSTHROUGH;
					tso_new.flag[0] |= TOE_F0_HDRSIZ_NORMAL;
					tso_new.flag[2] |= TOE_F2_AMSDU_CSUM_EN;
					WLCNTINCR(wlc->pub->_cnt->cso_passthrough);
				}

				if (resid - len > 0)
					tso_new.flag[2] |= TOE_F2_AMSDU_FS_MID;
				else
					tso_new.flag[2] |= TOE_F2_AMSDU_FS_LAST;

				len_new = TSO_HEADER_LENGTH;
#ifdef DEBUG_TSO_AMSDU
				WL_TSO(("wl%d: %s: subframe: %d csum_needed: %d passthrough: %d\n",
					wlc->pub->unit, __FUNCTION__, num_sf, csum_needed,
					passthrough));
				prhex("TSO header dump:", (uint8 *)&tso_new, len_new);
#endif // endif
				tsohdr = (d11ac_tso_t*)PKTPUSH(wlc->osh, newpkt0, len_new);
				bcopy(&tso_new, tsohdr, len_new);
#ifdef DEBUG_TSO_AMSDU
				prhex("TSO header dump:", (uint8 *)tsohdr, len_new + ETHER_HDR_LEN);
#endif // endif
			}
		}
		offset += len - tmplen;
		num_sf++;
		pt = pktdataoffset_delta(wlc->osh, pdt_p, len - tmplen);

		newpkt = pdt_p->cur_p;
		resid -= len;
	}
#ifdef PKTSIZ_FIXUP
	if ((num_sf > 1) && D11REV_IS(wlc->pub->corerev, 40) && WLPKTFLAG_AMSDU(WLPKTTAG(p))) {
		while (p) {
			wlc_tso_pktsize_fixup(wlc->osh, p);
			p = PKTNEXT(wlc->osh, p);
		}
	}
#endif  /* PKTSIZ_FIXUP */
pass:
	if (ret != 0)
		WL_TSO(("wl%d: %s: encountered error condition in sw amsdu cso\n",
			wlc->pub->unit, __FUNCTION__));
	return ret;
}

static int
wlc_tso_ext_info(wlc_info_t *wlc, d11ac_tso_t* tso, void *p, struct scb *scb,
	const wlc_key_info_t *key_info, uint nfrags)
{
	struct d11actxh *txh = (struct d11actxh*) PKTDATA(wlc->osh, p);
	struct dot11_header *h;
	uint16 ethertype;
	uint offset = 0;
	bool passthrough = FALSE;
	int delta;
	int ret = 0;
	uint8 *pt = (uint8 *)txh;
	wlc_pkttag_t *pkttag = WLPKTTAG(p);
	bool csum_needed = FALSE;
	bool amsdu = WLPKTFLAG_AMSDU(pkttag);

#define OFFSET_INCR(a) \
	do {  \
		if ((a) > 0) { \
			offset += (uint)(a);				 \
			pt = pktdataoffset(wlc->osh, p, offset); \
			if (pt == NULL)  { \
				passthrough = TRUE;			   \
				goto tso_exit; \
			} \
		} \
	} while (0)

	bzero(tso, sizeof(d11ac_tso_t));
	if (!(pkttag->flags2 & WLF2_HW_CSUM) && !amsdu)
		goto tso_exit;

	csum_needed = TRUE;

	/* txh: long format 124; short format 20 */
	if (txh->PktInfo.MacTxControlLow & D11AC_TXC_HDR_FMT_SHORT) {
		tso->flag[2] |= TOE_F2_TXD_HEAD_SHORT;
		OFFSET_INCR(D11AC_TXH_SHORT_LEN);
	}
	else
		OFFSET_INCR(D11AC_TXH_LEN);

#ifdef DEBUG_TSO
	WL_TSO(("wl%d: %s: total: %d, first: %d, offset: %d\n",
	   wlc->pub->unit, __FUNCTION__, pkttotlen(wlc->osh, p), PKTLEN(wlc->osh, p), offset));
#endif // endif
	h = (struct dot11_header *) pt;

	if ((delta = wlc_tso_parse_d11(wlc, tso, h, p, scb, key_info, nfrags, offset)) < 0) {
		passthrough = TRUE;
		goto tso_exit;
	}
	OFFSET_INCR(delta);

	if (amsdu) {

		/* sw amsdu processing here */
		ret = wlc_tso_parse_amsdu(wlc, tso, pt, p, offset);
		return ret;
	}

	if ((delta = wlc_tso_parse_eth(wlc, pt, p, offset)) < 0) {
		passthrough = TRUE;
		goto tso_exit;
	}

	OFFSET_INCR(delta);
	ethertype = ntoh16(*(uint16 *)pt);

#ifdef DEBUG_TSO
	WL_TSO(("wl%d: %s: ethertype: 0x%4x\n",
		wlc->pub->unit, __FUNCTION__, ethertype));
#endif // endif

	/* Skip VLAN tag, if any */
	if (ethertype == ETHER_TYPE_8021Q) {
		OFFSET_INCR(VLAN_TAG_LEN);
		ethertype = ntoh16(*(uint16 *)pt);
	}

	if ((ethertype != ETHER_TYPE_IP) && (ethertype != ETHER_TYPE_IPV6)) {
		WL_TSO(("wl%d: %s: non-IP frame (ethertype 0x%x)\n",
		          wlc->pub->unit, __FUNCTION__, ethertype));
		passthrough = TRUE;
		goto tso_exit;
	}
	OFFSET_INCR(ETHER_TYPE_LEN);

	if ((passthrough = wlc_tso_parse_ip(wlc, tso, pt, p, offset, FALSE)))
		goto tso_exit;

tso_exit:
	if (csum_needed && passthrough) {
		WL_ERROR(("wl%d: %s: hw csum is expected but passthrough is set!\n",
			wlc->pub->unit, __FUNCTION__));
		WLCNTINCR(wlc->pub->_cnt->cso_passthrough);
	}

	if (!csum_needed || passthrough) {
		tso->flag[0] |= TOE_F0_PASSTHROUGH;
		ret = -1;
	}
	else {
		tso->flag[0] |= TOE_F0_HDRSIZ_NORMAL;
		WLCNTINCR(wlc->pub->_cnt->cso_normal);
	}
	return ret;
}

bool
wlc_tso_support(wlc_info_t *wlc)
{
#ifdef DEBUG_TSO
	WL_TSO(("wl%d: %s: tso capable: %d toe_bypass: %d\n",
		wlc->pub->unit, __FUNCTION__, wlc->toe_capable, wlc->toe_bypass));
#endif // endif
	return (wlc->toe_capable && !wlc->toe_bypass);
}

void
wlc_set_tx_csum(wlc_info_t *wlc, uint32 on_off)
{
	/* it is not really turning off the hardware. */
	if (on_off)
		wlc->toe_bypass = FALSE;
	else
		wlc->toe_bypass = TRUE;

}

static uint16
csum_pseudohdr(uint8 *tcph, uint32 sip, uint32 dip,
                    uint8 proto, uint16 proto_len)
{
	uint32 acc;
	uint8 swapped;

	acc = 0;
	swapped = 0;

	/* pseudo header */
	acc += (sip & 0xffffUL);
	acc += ((sip >> 16) & 0xffffUL);
	acc += (dip & 0xffffUL);
	acc += ((dip >> 16) & 0xffffUL);

	acc += (uint32)hton16((uint16)proto);
	acc += (uint32)hton16(proto_len);

	while (acc >> 16) {
		acc = (acc & 0xffffUL) + (acc >> 16);
	}

	return (uint16)~(acc & 0xffffUL);
}

/* code for csum calc */
static uint32
csum_part(const uint8 *pb, int len)
{
	const uint16 *ps;
	uint16 t = 0;
	const uint32 *pl;
	uint32 sum = 0, tmp;
	int odd = ((uint32)(uintptr)pb & 1);	/* starts at odd byte address? */

	if (odd && len > 0) {
		((uint8 *)&t)[1] = *pb++;
		len--;
	}
	ps = (const uint16 *)pb;

	if (((uint32)(uintptr)ps & 3) && len > 1) {
		sum += *ps++;
		len -= 2;
	}
	pl = (const uint32 *)ps;

	while (len & ~15)  {
		tmp = sum + *pl++;	/* ping */
		if (tmp < sum)
			tmp++;		/* add back carry */

		sum = tmp + *pl++;	/* pong */
		if (sum < tmp)
			sum++;		/* add back carry */

		tmp = sum + *pl++;	/* ping */
		if (tmp < sum)
			tmp++;		/* add back carry */

		sum = tmp + *pl++;	/* pong */
		if (sum < tmp)
			sum++;		/* add back carry */

		len -= 16;
	}

	/* fold to make room in upper bits */
	sum = (sum >> 16) + (sum & 0xffff);
	ps = (const uint16 *)pl;

	/* zero to seven 16-bit aligned shorts remaining */
	while (len > 1) {
		sum += *ps++;
		len -= 2;
	}

	/* dangling tail byte remaining? */
	if (len > 0)			/* include odd byte */
		((uint8 *)&t)[0] = *(const uint8 *)ps;

	sum += t;			/* add end bytes */

	/* fold twice */
	sum = (sum >> 16) + (sum & 0xffff);
	sum = (sum >> 16) + (sum & 0xffff);

	/* byte-swap sum if original alignment was odd */
	if (odd)
		sum = (sum << 8 | sum >> 8) & 0xffff;

	return sum;
}

static uint16
csum_inet(void *dataptr, uint16 len)
{
	uint32 acc;

	acc = csum_part(dataptr, len);
	while (acc >> 16) {
		acc = (acc & 0xffff) + (acc >> 16);
	}
	return (uint16)~(acc & 0xffff);
}

static uint16
csum_with_pseudohdr(uint8 *tcph, uint32 sip, uint32 dip,
                    uint8 proto, uint16 proto_len)
{
	uint32 acc;
	uint8 swapped;

	acc = 0;
	swapped = 0;

	acc += csum_part(tcph, proto_len);
	/* pseudo header */
	acc += (sip & 0xffffUL);
	acc += ((sip >> 16) & 0xffffUL);
	acc += (dip & 0xffffUL);
	acc += ((dip >> 16) & 0xffffUL);

	acc += (uint32)hton16((uint16)proto);
	acc += (uint32)hton16(proto_len);

	while (acc >> 16) {
		acc = (acc & 0xffffUL) + (acc >> 16);
	}

	return (uint16)~(acc & 0xffffUL);
}
#endif /* WLCSO */

/*
 * Add Transmit Offload Engine header
 */
void
wlc_toe_add_hdr(wlc_info_t *wlc, void *p, struct scb *scb, const wlc_key_info_t *key_info,
	uint nfrags, uint16 *pushlen)
{
	d11ac_tso_t tso;
	d11ac_tso_t * tsohdr;
	int len;

	memset(&tso, 0, sizeof(tso));

#ifdef WLCSO
	/* May need checksum for CSO */
	wlc_tso_ext_info(wlc, &tso, p, scb, key_info, nfrags);
	len = wlc_tso_hdr_length(&tso);

#ifdef DEBUG_TSO
	prhex("TSO header dump:", (uint8 *)&tso, len);
#endif // endif
#else
	/* No CSO, prepare a passthrough TOE header */
	tso.flag[0] |= TOE_F0_PASSTHROUGH;
	len = TSO_HEADER_PASSTHROUGH_LENGTH;

	BCM_REFERENCE(scb);
	BCM_REFERENCE(key_info);
	BCM_REFERENCE(nfrags);

#endif /* WLCSO */
	tsohdr = (d11ac_tso_t*)PKTPUSH(wlc->osh, p, len);
	bcopy(&tso, tsohdr, len);

	if (pushlen != NULL)
		*pushlen = (uint16)len;
}

uint
wlc_tso_hdr_length(d11ac_tso_t* tso)
{
	uint len;

	if (tso->flag[0] & TOE_F0_HDRSIZ_NORMAL)
		len = TSO_HEADER_LENGTH;
	else
		len = TSO_HEADER_PASSTHROUGH_LENGTH;

	return len;
}
#endif /* WLTOEHW */
