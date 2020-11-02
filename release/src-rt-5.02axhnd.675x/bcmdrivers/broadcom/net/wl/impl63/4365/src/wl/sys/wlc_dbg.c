/*
 * driver debug and print functions
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
 * $Id: wlc_dbg.c 467328 2014-04-03 01:23:40Z $
 */

#include <wlc_cfg.h>

#include <typedefs.h>
#include <bcmutils.h>
#include <wlc_types.h>
#include <bcmwifi_channels.h>
#include <siutils.h>
#include <wlioctl.h>
#include <wlc_pub.h>
#include <bcmendian.h>
#include <proto/802.11.h>
#include <wlioctl.h>
#include <d11.h>
#include <wlc_rate.h>
#include <wlc.h>
#include <wlc_tso.h>
#include <wlc_dbg.h>

#if defined(BCMDBG) || defined(WLMSG_PRHDRS) || defined(WLMSG_PRPKT) || \
	defined(WLMSG_ASSOC)
static const char* errstr = "802.11 Header INCOMPLETE\n";
static const char* fillstr = "------------";

static void
wlc_print_dot11_plcp(uint8* buf, int len)
{
	char hexbuf[(2*D11B_PHY_HDR_LEN)+1];

	if (len < D11B_PHY_HDR_LEN) {
		bcm_format_hex(hexbuf, buf, len);
		strncpy(hexbuf + (2 * len), fillstr, 2 * (D11B_PHY_HDR_LEN - len));
		hexbuf[sizeof(hexbuf) - 1] = '\0';
	} else {
		bcm_format_hex(hexbuf, buf, D11B_PHY_HDR_LEN);
	}

	printf("PLCPHdr: %s ", hexbuf);
	if (len < D11B_PHY_HDR_LEN) {
		printf("%s\n", errstr);
	}
}

static void
wlc_print_dot11hdr(uint8* buf, int len)
{
	if (len == 0) {
		printf("802.11 Header MISSING\n");
		return;
	}

	wlc_print_dot11_plcp(buf, len);

	if (len < D11B_PHY_HDR_LEN) {
		return;
	}

	len -= D11B_PHY_HDR_LEN;
	buf += D11B_PHY_HDR_LEN;

	wlc_print_dot11_mac_hdr(buf, len);
}

void
wlc_print_dot11_mac_hdr(uint8* buf, int len)
{
	char hexbuf[(2*D11B_PHY_HDR_LEN)+1];
	char a1[(2*ETHER_ADDR_LEN)+1], a2[(2*ETHER_ADDR_LEN)+1];
	char a3[(2*ETHER_ADDR_LEN)+1];
	char flagstr[64];
	uint16 fc, kind, toDS, fromDS;
	uint16 v;
	int fill_len = 0;
	const bcm_bit_desc_t fc_flags[] = {
		{FC_TODS, "ToDS"},
		{FC_FROMDS, "FromDS"},
		{FC_MOREFRAG, "MoreFrag"},
		{FC_RETRY, "Retry"},
		{FC_PM, "PM"},
		{FC_MOREDATA, "MoreData"},
		{FC_WEP, "WEP"},
		{FC_ORDER, "Order"},
		{0, NULL}
	};

	if (len < 2) {
		printf("FC: ------ ");
		printf("%s\n", errstr);
		return;
	}

	fc = buf[0] | (buf[1] << 8);
	kind = fc & FC_KIND_MASK;
	toDS = (fc & FC_TODS) != 0;
	fromDS = (fc & FC_FROMDS) != 0;

	bcm_format_flags(fc_flags, fc, flagstr, 64);

	printf("FC: 0x%04x ", fc);
	if (flagstr[0] != '\0')
		printf("(%s) ", flagstr);

	len -= 2;
	buf += 2;

	if (len < 2) {
		printf("Dur/AID: ----- ");
		printf("%s\n", errstr);
		return;
	}

	v = buf[0] | (buf[1] << 8);
	if (kind == FC_PS_POLL) {
		printf("AID: 0x%04x", v);
	} else {
		printf("Dur: 0x%04x", v);
	}
	printf("\n");
	len -= 2;
	buf += 2;

	strncpy(a1, fillstr, sizeof(a1)-1);
	a1[2*ETHER_ADDR_LEN] = '\0';

	strncpy(a2, fillstr, sizeof(a2)-1);
	a2[2*ETHER_ADDR_LEN] = '\0';

	strncpy(a3, fillstr, sizeof(a3)-1);
	a3[2*ETHER_ADDR_LEN] = '\0';

	if (len < ETHER_ADDR_LEN) {
		bcm_format_hex(a1, buf, len);
		strncpy(a1+(2*len), fillstr, 2*(ETHER_ADDR_LEN-len));
	} else if (len < 2*ETHER_ADDR_LEN) {
		bcm_format_hex(a1, buf, ETHER_ADDR_LEN);
		bcm_format_hex(a2, buf+ETHER_ADDR_LEN, len-ETHER_ADDR_LEN);
		fill_len = len - ETHER_ADDR_LEN;
		strncpy(a2+(2*fill_len), fillstr, 2*(ETHER_ADDR_LEN-fill_len));
	} else if (len < 3*ETHER_ADDR_LEN) {
		bcm_format_hex(a1, buf, ETHER_ADDR_LEN);
		bcm_format_hex(a2, buf+ETHER_ADDR_LEN, ETHER_ADDR_LEN);
		bcm_format_hex(a3, buf+(2*ETHER_ADDR_LEN), len-(2*ETHER_ADDR_LEN));
		fill_len = len - (2*ETHER_ADDR_LEN);
		strncpy(a3+(2*fill_len), fillstr, 2*(ETHER_ADDR_LEN-fill_len));
	} else {
		bcm_format_hex(a1, buf, ETHER_ADDR_LEN);
		bcm_format_hex(a2, buf+ETHER_ADDR_LEN, ETHER_ADDR_LEN);
		bcm_format_hex(a3, buf+(2*ETHER_ADDR_LEN), ETHER_ADDR_LEN);
	}

	if (kind == FC_RTS) {
		printf("RA: %s ", a1);
		printf("TA: %s ", a2);
		if (len < 2*ETHER_ADDR_LEN)
			printf("%s ", errstr);
	} else if (kind == FC_CTS || kind == FC_ACK) {
		printf("RA: %s ", a1);
		if (len < ETHER_ADDR_LEN)
			printf("%s ", errstr);
	} else if (kind == FC_PS_POLL) {
		printf("BSSID: %s", a1);
		printf("TA: %s ", a2);
		if (len < 2*ETHER_ADDR_LEN)
			printf("%s ", errstr);
	} else if (kind == FC_CF_END || kind == FC_CF_END_ACK) {
		printf("RA: %s ", a1);
		printf("BSSID: %s ", a2);
		if (len < 2*ETHER_ADDR_LEN)
			printf("%s ", errstr);
	} else if (FC_TYPE(fc) == FC_TYPE_DATA) {
		if (!toDS) {
			printf("DA: %s ", a1);
			if (!fromDS) {
				printf("SA: %s ", a2);
				printf("BSSID: %s ", a3);
			} else {
				printf("BSSID: %s ", a2);
				printf("SA: %s ", a3);
			}
		} else if (!fromDS) {
			printf("BSSID: %s ", a1);
			printf("SA: %s ", a2);
			printf("DA: %s ", a3);
		} else {
			printf("RA: %s ", a1);
			printf("TA: %s ", a2);
			printf("DA: %s ", a3);
		}
		if (len < 3*ETHER_ADDR_LEN) {
			printf("%s ", errstr);
		} else if (len < 20) {
			printf("SeqCtl: ------ ");
			printf("%s ", errstr);
		} else {
			len -= 3*ETHER_ADDR_LEN;
			buf += 3*ETHER_ADDR_LEN;
			v = buf[0] | (buf[1] << 8);
			printf("SeqCtl: 0x%04x ", v);
			len -= 2;
			buf += 2;
		}
	} else if (FC_TYPE(fc) == FC_TYPE_MNG) {
		printf("DA: %s ", a1);
		printf("SA: %s ", a2);
		printf("BSSID: %s ", a3);
		if (len < 3*ETHER_ADDR_LEN) {
			printf("%s ", errstr);
		} else if (len < 20) {
			printf("SeqCtl: ------ ");
			printf("%s ", errstr);
		} else {
			len -= 3*ETHER_ADDR_LEN;
			buf += 3*ETHER_ADDR_LEN;
			v = buf[0] | (buf[1] << 8);
			printf("SeqCtl: 0x%04x ", v);
			len -= 2;
			buf += 2;
		}
	}

	if ((FC_TYPE(fc) == FC_TYPE_DATA) && toDS && fromDS) {

		if (len < ETHER_ADDR_LEN) {
			bcm_format_hex(hexbuf, buf, len);
			strncpy(hexbuf+(2*len), fillstr, 2*(ETHER_ADDR_LEN-len));
		} else {
			bcm_format_hex(hexbuf, buf, ETHER_ADDR_LEN);
		}

		printf("SA: %s ", hexbuf);

		if (len < ETHER_ADDR_LEN) {
			printf("%s ", errstr);
		} else {
			len -= ETHER_ADDR_LEN;
			buf += ETHER_ADDR_LEN;
		}
	}

	if ((FC_TYPE(fc) == FC_TYPE_DATA) && (kind == FC_QOS_DATA)) {
		if (len < 2) {
			printf("QoS: ------");
			printf("%s ", errstr);
		} else {
			v = buf[0] | (buf[1] << 8);
			printf("QoS: 0x%04x ", v);
			len -= 2;
			buf += 2;
		}
	}

	printf("\n");
	return;
}
#endif /* BCMDBG || WLMSG_PRHDRS || WLMSG_PRPKT || WLMSG_ASSOC */

#if defined(WL11AC) && defined(BCMDBG)
static void
wlc_dump_vht_plcp(uint8 *plcp)
{
	/* compute VHT plcp - 6 B */
	uint32 plcp0 = 0, plcp1 = 0;
	char flagstr[16];

	const bcm_bit_desc_t plcp0_flags_bf[] = {
		{VHT_SIGA1_20MHZ_VAL, "20MHz"},
		{VHT_SIGA1_40MHZ_VAL, "40MHz"},
		{VHT_SIGA1_80MHZ_VAL, "80MHz"},
		{VHT_SIGA1_160MHZ_VAL, "160MHz"},
		{0, NULL}
	};

	const bcm_bit_desc_ex_t plcp0_flags = {
			VHT_SIGA1_BW_MASK,
			&(plcp0_flags_bf[0])
	};

	plcp0 = (plcp[0] & 0xff);
	plcp0 |= ((uint32)plcp[1] << 8);
	plcp0 |= ((uint32)plcp[2] << 16);
	plcp1 = (plcp[3] & 0xff);
	plcp1 |= ((uint32)plcp[4] << 8);
	plcp1 |= ((uint32)plcp[5] << 16);

	if (!(plcp0 & VHT_SIGA1_CONST_MASK) ||
		!(plcp1 & VHT_SIGA2_B9_RESERVED)) {
		printf("non-vht plcp");
		return;
	}

	if (plcp0 & VHT_SIGA1_STBC) {
		printf("stbc ");
	}

	if (bcm_format_field(&(plcp0_flags), plcp0, flagstr, 16)) {
		printf("%s ", flagstr);
	}

	/* (in SU) bit4-9 if pkt addressed to AP = 0, else 63 */
	printf("gid=%d ", (plcp0 & VHT_SIGA1_GID_MASK) >> VHT_SIGA1_GID_SHIFT);

	/* b10-b13 NSTS */
	/* for SU b10-b12 set to num STS-1 (fr 0-7) */
	printf("nsts=%d ",
		(plcp0 & VHT_SIGA1_NSTS_SHIFT_MASK_USER0) >> VHT_SIGA1_NSTS_SHIFT);

	/* b13-b21 partial AID: Set to value of TXVECTOR param PARTIAL_AID */
	printf("partialAID=%x ",
		(plcp0 & VHT_SIGA1_PARTIAL_AID_MASK) >> VHT_SIGA1_PARTIAL_AID_SHIFT);

	if (plcp1 & VHT_SIGA2_GI_SHORT) {
		printf("sgi ");
	}

	if (plcp1 & VHT_SIGA2_CODING_LDPC) {
		printf("ldpc ");
	}
	printf("mcs_idx=%x ", (plcp1 >> VHT_SIGA2_MCS_SHIFT) & (RSPEC_VHT_MCS_MASK));

	if (plcp1 & VHT_SIGA2_BEAMFORM_ENABLE) {
		printf("txbf");
	}
}
#endif	/* BCMDBG && WL11AC */

#if defined(BCMDBG) || defined(WLMSG_PRHDRS)
static void
wlc_print_d11txh(d11txh_t* txh)
{
	uint16 mtcl = ltoh16(txh->MacTxControlLow);
	uint16 mtch = ltoh16(txh->MacTxControlHigh);
	uint16 mfc = ltoh16(txh->MacFrameControl);
	uint16 tfest = ltoh16(txh->TxFesTimeNormal);
	uint16 ptcw = ltoh16(txh->PhyTxControlWord);
	uint16 ptcw_1 = ltoh16(txh->PhyTxControlWord_1);
	uint16 ptcw_1_Fbr = ltoh16(txh->PhyTxControlWord_1_Fbr);
	uint16 ptcw_1_Rts = ltoh16(txh->PhyTxControlWord_1_Rts);
	uint16 ptcw_1_FbrRts = ltoh16(txh->PhyTxControlWord_1_FbrRts);
	uint16 mainrates = ltoh16(txh->MainRates);
	uint16 xtraft = ltoh16(txh->XtraFrameTypes);
	uint8 *iv = txh->IV;
	uint8 *ra = txh->TxFrameRA;
	uint16 tfestfb = ltoh16(txh->TxFesTimeFallback);
	uint8 *rtspfb = txh->RTSPLCPFallback;
	uint16 rtsdfb = ltoh16(txh->RTSDurFallback);
	uint8 *fragpfb = txh->FragPLCPFallback;
	uint16 fragdfb = ltoh16(txh->FragDurFallback);
	uint16 mmodelen = ltoh16(txh->MModeLen);
	uint16 mmodefbrlen = ltoh16(txh->MModeFbrLen);
	uint16 tfid = ltoh16(txh->TxFrameID);
	uint16 txs = ltoh16(txh->TxStatus);
	uint16 mnmpdu = ltoh16(txh->MaxNMpdus);
	uint16 maxdur = ltoh16(txh->u1.MaxAggDur);
	uint8 maxrnum = txh->u2.s1.MaxRNum;
	uint8 maxaggbytes = txh->u2.s1.MaxAggBytes;
	uint16 mmbyte = ltoh16(txh->MinMBytes);

	uint8 *rtsph = txh->RTSPhyHeader;
	struct dot11_rts_frame rts = txh->rts_frame;
	char hexbuf[256];

	prhex("Raw TxDesc", (uchar *) txh, sizeof(d11txh_t));

	printf("TxCtlLow: %04x ", mtcl);
	printf("TxCtlHigh: %04x ", mtch);
	printf("FC: %04x ", mfc);
	printf("FES Time: %04x\n", tfest);
	printf("PhyCtl: %04x%s ", ptcw, (ptcw & PHY_TXC_SHORT_HDR) ? " short" : "");
	printf("PhyCtl_1: %04x ", ptcw_1);
	printf("PhyCtl_1_Fbr: %04x\n", ptcw_1_Fbr);
	printf("PhyCtl_1_Rts: %04x ", ptcw_1_Rts);
	printf("PhyCtl_1_Fbr_Rts: %04x\n", ptcw_1_FbrRts);
	printf("MainRates: %04x ", mainrates);
	printf("XtraFrameTypes: %04x ", xtraft);
	printf("\n");

	bcm_format_hex(hexbuf, iv, sizeof(txh->IV));
	printf("SecIV:       %s\n", hexbuf);
	bcm_format_hex(hexbuf, ra, sizeof(txh->TxFrameRA));
	printf("RA:          %s\n", hexbuf);

	printf("Fb FES Time: %04x ", tfestfb);
	bcm_format_hex(hexbuf, rtspfb, sizeof(txh->RTSPLCPFallback));
	printf("RTS PLCP: %s ", hexbuf);
	printf("RTS DUR: %04x ", rtsdfb);
	bcm_format_hex(hexbuf, fragpfb, sizeof(txh->FragPLCPFallback));
	printf("PLCP: %s ", hexbuf);
	printf("DUR: %04x", fragdfb);
	printf("\n");

	printf("MModeLen: %04x ", mmodelen);
	printf("MModeFbrLen: %04x\n", mmodefbrlen);

	printf("FrameID:     %04x\n", tfid);
	printf("TxStatus:    %04x\n", txs);

	printf("MaxNumMpdu:  %04x\n", mnmpdu);
	printf("MaxAggDur:   %04x\n", maxdur);
	printf("MaxRNum:     %04x\n", maxrnum);
	printf("MaxAggBytes: %04x\n", maxaggbytes);
	printf("MinByte:     %04x\n", mmbyte);

	bcm_format_hex(hexbuf, rtsph, sizeof(txh->RTSPhyHeader));
	printf("RTS PLCP: %s ", hexbuf);
	bcm_format_hex(hexbuf, (uint8 *) &rts, sizeof(txh->rts_frame));
	printf("RTS Frame: %s", hexbuf);
	printf("\n");

	if (mtcl & TXC_SENDRTS) {
		wlc_print_dot11hdr((uint8 *) &rts, sizeof(txh->rts_frame));
	}
}

/* print byte/word without new line */
static void
wlc_print_byte(const char* desc, uint8 val)
{
	printf("%s: %02x", desc, val);
}

static void
wlc_print_word(const char* desc, uint16 val)
{
	printf("%s: %04x", desc, val);
}

/* print byte/word with new line */
static void
wlc_printn_byte(const char* desc, uint8 val)
{
	printf("%s: %02x\n", desc, val);
}

static void
wlc_print_per_pkt_desc_ac(d11actxh_t* acHdrPtr)
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
	       (mcl & D11AC_TXC_IPMQ) != 0);
	printf(" MBurst %u ASeq %u Aging %u AMIC %u STMSDU %u RIFS %u ~FCS %u FixRate %u MU %u\n",
	       (mcl & D11AC_TXC_MBURST) != 0,
	       (mcl & D11AC_TXC_ASEQ) != 0,
	       (mcl & D11AC_TXC_AGING) != 0,
	       (mcl & D11AC_TXC_AMIC) != 0,
	       (mcl & D11AC_TXC_STMSDU) != 0,
	       (mcl & D11AC_TXC_URIFS) != 0,
	       (mch & D11AC_TXC_DISFCS) != 0,
	       (mch & D11AC_TXC_FIX_RATE) != 0,
	       (mch & D11AC_TXC_MU) != 0);

	printf(" IVOffset %u PktCacheLen %u FrameLen %u\n",
	       pi->IVOffset, pi->PktCacheLen, ltoh16(pi->FrameLen));
	printf(" Seq 0x%04X TxFrameID 0x%04X Tstamp 0x%04X TxStatus 0x%04X\n",
	       ltoh16(pi->Seq), ltoh16(pi->TxFrameID),
	       ltoh16(pi->Tstamp), ltoh16(pi->TxStatus));
}

static void
wlc_print_per_rate_info(wlc_info_t *wlc, d11actxh_t* acHdrPtr, uint8 rateIdx)
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
	printf(" plcp: %02X %02X %02X %02X %02X %02X [",
	       ri->plcp[0], ri->plcp[1], ri->plcp[2],
	       ri->plcp[3], ri->plcp[4], ri->plcp[5]);
#if defined(WL11AC) && defined(BCMDBG)
	wlc_dump_vht_plcp(ri->plcp);
#endif /* WL11AC */
	printf("]\n");
	printf(" FbwInfo 0x%04X TxRate 0x%04X RtsCtsControl 0x%04X Bfm0 0x%04X\n",
		ltoh16(ri->FbwInfo), ltoh16(ri->TxRate), ltoh16(ri->RtsCtsControl),
		ltoh16(ri->Bfm0));
}

static void
wlc_print_per_pkt_cache_ac(wlc_info_t *wlc, d11actxh_t* acHdrPtr)
{
	d11actxh_cache_t	*cache_info;

	cache_info = WLC_TXD_CACHE_INFO_GET(acHdrPtr, wlc->pub->corerev);

	printf("TxD Pkt More Info:\n");
	wlc_print_byte(" BssIdEncAlg", cache_info->BssIdEncAlg);
	wlc_printn_byte(" KeyIdx", cache_info->KeyIdx);
	wlc_print_byte(" MpduMax: primary", cache_info->PrimeMpduMax);
	wlc_printn_byte(" fbr", cache_info->FallbackMpduMax);
	wlc_print_word(" AmpduDur", ltoh16(cache_info->AmpduDur));
	wlc_print_byte(" BAWin", cache_info->BAWin);
	wlc_printn_byte(" MaxAggLen", cache_info->MaxAggLen);
	prhex(" TkipPH1Key", (uchar *)cache_info->TkipPH1Key, 10);
	prhex(" TSCPN", (uchar *)cache_info->TSCPN, 6);
}

void
wlc_print_txdesc_ac(wlc_info_t *wlc, void* hdrsBegin)
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
	wlc_print_per_pkt_desc_ac(acHdrPtr);

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
			wlc_print_per_rate_info(wlc, acHdrPtr, rateNum);
		}
		wlc_print_per_pkt_cache_ac(wlc, acHdrPtr);
	}
}

void
wlc_print_txdesc(wlc_info_t *wlc, wlc_txd_t *txd)
{
	if (WLCISACPHY(wlc->band)) {
#ifdef WLTOEHW
		if (wlc->toe_capable && !wlc->toe_bypass) {
			uint8 *tsoPtr = (uint8*)txd;
			uint tsoLen = wlc_tso_hdr_length((d11ac_tso_t*)tsoPtr);

			prhex("TSO hdr:", (uint8 *)tsoPtr, tsoLen);
			txd = (wlc_txd_t*)(tsoPtr + tsoLen);
		}
#endif /* WLTOEHW */
		wlc_print_txdesc_ac(wlc, txd);
	} else {
		wlc_print_d11txh(&txd->d11txh);
	}
}
#endif /* BCMDBG || WLMSG_PRHDRS */

#if defined(BCMDBG) || defined(WLMSG_PRHDRS)
/* Debug print of short receive status header */
static void
wlc_recv_print_srxh(d11rxhdrshort_t *srxh)
{
	uint16 len = srxh->RxFrameSize;
	char lenbuf[20];
	const char *agg_type_str[] = {"Intermediate", "First", "Last", "Single"};
	uint16 agg_type = (srxh->mrxs & RXSS_AGGTYPE_MASK) >> RXSS_AGGTYPE_SHIFT;

	prhex("\nRaw RxDesc (short format)", (uchar *)srxh, sizeof(d11rxhdrshort_t));

	snprintf(lenbuf, sizeof(lenbuf), "0x%x", len);
	printf("RxFrameSize:   %-6s (%d)\n", lenbuf, len);
	printf("dma_flags:     %#x\n", srxh->dma_flags);
	printf("fifo:          %u\n", srxh->fifo);
	printf("mrxs:          %04x (", srxh->mrxs);
	if (srxh->mrxs & RXSS_AMSDU_MASK) {
		printf("AMSDU, ");
	}
	printf("%s, ", agg_type_str[agg_type]);
	if (srxh->mrxs & RXSS_PBPRES)
		printf("Pad, ");
	printf("Hdrconv %s, ",
	       ((srxh->mrxs & RXSS_HDRSTS) == RXSS_HDRSTS) ? "on" : "off");
	printf("MSDU cnt %u)\n",
	       (srxh->mrxs & RXSS_MSDU_CNT_MASK) >> RXSS_MSDU_CNT_SHIFT);
	printf("RxTSFTime:     %04x\n", srxh->RxTSFTime);
}

void
wlc_recv_print_rxh(wlc_info_t *wlc, wlc_d11rxhdr_t *wrxh)
{
	d11rxhdr_t * rxh = &wrxh->rxhdr;
	uint16 len = rxh->RxFrameSize;
	uint16 phystatus_0 = rxh->PhyRxStatus_0;
	uint16 phystatus_1 = rxh->PhyRxStatus_1;
	uint16 phystatus_2 = rxh->PhyRxStatus_2;
	uint16 phystatus_3 = rxh->PhyRxStatus_3;
	uint16 macstatus1 = rxh->RxStatus1;
	uint16 macstatus2 = rxh->RxStatus2;
	char flagstr[64];
	char lenbuf[20];
	char chanbuf[CHANSPEC_STR_LEN];
	uint16 agg_type;
	const char *agg_type_str[] = {"Intermediate", "First", "Last", "Single"};
	const bcm_bit_desc_t macstat_flags[] = {
		{RXS_FCSERR, "FCSErr"},
		{RXS_RESPFRAMETX, "Reply"},
		{RXS_PBPRES, "PadPres"},
		{RXS_DECATMPT, "DeCr"},
		{RXS_DECERR, "DeCrErr"},
		{RXS_BCNSENT, "Bcn"},
		{0, NULL}
	};

	/* check if rx status header is short format */
	if ((D11REV_GE(wlc->pub->corerev, 64)) && (rxh->dma_flags & RXS_SHORT_MASK)) {
		wlc_recv_print_srxh((d11rxhdrshort_t*) rxh);
		return;
	}
	agg_type = (macstatus2 & RXS_AGGTYPE_MASK) >> RXS_AGGTYPE_SHIFT;
	prhex("Raw RxDesc", (uchar *)rxh, sizeof(d11rxhdr_t));

	bcm_format_flags(macstat_flags, macstatus1, flagstr, 64);

	snprintf(lenbuf, sizeof(lenbuf), "0x%x", len);
	printf("RxFrameSize:   %6s (%d)%s\n", lenbuf, len,
		(rxh->PhyRxStatus_0 & PRXS0_SHORTH) ? " short preamble" : "");
	printf("dma_flags:     %#x\n", rxh->dma_flags);
	printf("fifo:          %u\n", rxh->fifo);
	printf("RxPHYStatus:   %04x %04x %04x %04x\n",
		phystatus_0, phystatus_1, phystatus_2, phystatus_3);
	printf("RxMACStatus:   %x %s\n", macstatus1, flagstr);
	printf("RXMACaggtype:  %x (%s)\n", agg_type, agg_type_str[agg_type]);
	printf("RxTSFTime:     %04x\n", rxh->RxTSFTime);
	printf("RxChanspec:    %s (%04x)\n", wf_chspec_ntoa(rxh->RxChan, chanbuf), rxh->RxChan);
}

void
wlc_print_hdrs(wlc_info_t *wlc, const char *prefix, uint8 *frame,
               wlc_txd_t *txd, wlc_d11rxhdr_t *wrxh, uint len)
{
#if defined(WL11AC) && defined(BCMDBG)
	uint8 *plcp;
#endif // endif
	ASSERT(!(txd && wrxh));

	printf("\nwl%d: %s: len %d\n", wlc->pub->unit, prefix, len);

	if (txd) {
		wlc_print_txdesc(wlc, txd);
	} else if (wrxh) {
		wlc_recv_print_rxh(wlc, wrxh);
	}
	if (len > 0) {
		ASSERT(frame != NULL);

#if defined(WL11AC) && defined(BCMDBG)
		/* If 11ac rx, dump plcp */
		if (wrxh && ((wrxh->rxhdr.PhyRxStatus_0 & PRXS0_FT_MASK) == PRXS0_STDN)) {
			plcp = frame - D11_PHY_HDR_LEN;
			wlc_dump_vht_plcp(plcp);
		}
#endif // endif
		wlc_print_dot11_mac_hdr(frame, len);
	}
}
#endif /* BCMDBG || WLMSG_PRHDRS */

#if defined(WLTINYDUMP) || defined(BCMDBG) || defined(WLMSG_ASSOC) || \
	defined(WLMSG_PRPKT) || defined(WLMSG_OID) || defined(BCMDBG_DUMP) || \
	defined(WLMSG_INFORM) || defined(WLMSG_WSEC) || defined(WLEXTLOG) || defined(WLTEST) || \
	defined(BCMDBG_MU)
int
wlc_format_ssid(char* buf, const uchar ssid[], uint ssid_len)
{
	uint i, c;
	char *p = buf;
	char *endp = buf + SSID_FMT_BUF_LEN;

	if (ssid_len > DOT11_MAX_SSID_LEN) ssid_len = DOT11_MAX_SSID_LEN;

	for (i = 0; i < ssid_len; i++) {
		c = (uint)ssid[i];
		if (c == '\\') {
			*p++ = '\\';
			*p++ = '\\';
		} else if (bcm_isprint((uchar)c)) {
			*p++ = (char)c;
		} else {
			p += snprintf(p, (endp - p), "\\x%02X", c);
		}
	}
	*p = '\0';
	ASSERT(p < endp);

	return (int)(p - buf);
}
#endif /* WLTINYDUMP || BCMDBG || WLMSG_ASSOC || WLMSG_PRPKT || WLMSG_OID || BCMDBG_DUMP */

#if defined(BCMDBG) || defined(WLMSG_PRPKT) || defined(WLMSG_ASSOC)
static const bcm_bit_desc_t cap_flags[] = {
	{DOT11_CAP_ESS, "ESS"},
	{DOT11_CAP_IBSS, "IBSS"},
	{DOT11_CAP_POLLABLE, "Pollable"},
	{DOT11_CAP_POLL_RQ, "PollReq"},
	{DOT11_CAP_PRIVACY, "WEP"},
	{DOT11_CAP_SHORT, "ShortPre"},
	{DOT11_CAP_PBCC, "PBCC"},
	{DOT11_CAP_AGILITY, "Agility"},
	{DOT11_CAP_SHORTSLOT, "ShortSlot"},
	{DOT11_CAP_CCK_OFDM, "CCK-OFDM"},
	{0, NULL}
};

void
wlc_print_bcn_prb(uint8 *frame, int len)
{
	struct dot11_bcn_prb *fixed;
	uint16 *sp;
	uint8 *cp;
	uint32 t_hi, t_lo;
	uint16 cap;
	char flagstr[128];
	char hexbuf[65];
	static const char *id_names[] = {
		"SSID",
		"Rates",
		"FH Params",
		"DS Params",
		"CF Params",
		"TIM",
		"IBSS Params"
	};

	ASSERT(ISALIGNED(frame, sizeof(uint16)));

	wlc_print_dot11hdr(frame, len);

	fixed = (struct dot11_bcn_prb *)(frame + D11_PHY_HDR_LEN + DOT11_MAC_HDR_LEN);

	sp = (uint16*)fixed;
	t_lo = ltoh16(sp[0]) | ltoh16(sp[1]) << NBITS(uint16);
	t_hi = ltoh16(sp[2]) | ltoh16(sp[3]) << NBITS(uint16);
	if (t_hi) {
		printf("timestamp: %X:%08X\n", t_hi, t_lo);
	} else {
		printf("timestamp: %08X\n", t_lo);
	}
	printf("beacon interval: %d\n", ltoh16(fixed->beacon_interval));

	cap = ltoh16(fixed->capability);

	bcm_format_flags(cap_flags, cap, flagstr, 128);

	if (flagstr[0] != '\0') {
		printf("capabilities: %04x (%s)\n", cap, flagstr);
	} else {
		printf("capabilities: %04x\n", cap);
	}

	cp = (uint8*)&fixed[1];
	while (cp - frame < len) {
		const char* name = "";
		char* valstr = NULL;
		char ssidbuf[SSID_FMT_BUF_LEN + 2];

		if (cp[0] < 7)
			name = id_names[cp[0]];
		else if (cp[0] == DOT11_MNG_CHALLENGE_ID)
			name = "Challenge text";
		else if (cp[0] == DOT11_MNG_ERP_ID)
			name = "ERP Info";
		else if (cp[0] == DOT11_MNG_NONERP_ID)
			name = "Legacy ERP Info";
		else if (cp[0] == DOT11_MNG_EXT_RATES_ID)
			name = "Extended Rates";
		else if (cp[0] == DOT11_MNG_PROPR_ID)
			name = "Proprietary IE";

		if (cp[0] == DOT11_MNG_SSID_ID) {
			int i = wlc_format_ssid(ssidbuf+1, cp+2, cp[1]);
			ssidbuf[0] = '\"';
			ssidbuf[i+1] = '\"';
			ssidbuf[i+2] = '\0';
			valstr = ssidbuf;
		} else if (cp[1] <= 32) {
			bcm_format_hex(hexbuf, cp+2, cp[1]);
			valstr = hexbuf;
		}

		if (valstr != NULL) {
			printf("ID %d %s [%d]: %s\n", cp[0], name, cp[1], valstr);
		} else {
			printf("ID %d %s [%d]:\n", cp[0], name, cp[1]);
			prhex(NULL, cp+2, cp[1]);
		}

		cp += 2 + cp[1];
	}
	printf("\n");
}
#endif /* BCMDBG || WLMSG_PRPKT || WLMSG_ASSOC */

#ifdef STA

#if defined(BCMDBG) || defined(WLMSG_PRPKT)
void
wlc_print_assoc(wlc_info_t *wlc, struct dot11_management_header *mng, int len)
{
	bool is_assoc;
	uint8* body;
	uint body_len;
	uint8 *ies;
	uint ies_len;
	uint16 fc;
	uint16 cap;
	uint16 listen_interval;
	struct ether_addr *current_ap = NULL;
	char flagstr[128];

	ASSERT(ISALIGNED(mng, sizeof(uint16)));

	if (len < DOT11_MGMT_HDR_LEN) {
		printf("wl%d: 802.11 frame too short to be a management frame\n", wlc->pub->unit);
		printf("wl%d: packet [%d bytes]:\n", wlc->pub->unit, len);
		prhex(NULL, (uint8*)mng, len);
		return;
	}

	fc = ltoh16(mng->fc);

	body_len = len - DOT11_MGMT_HDR_LEN;
	body = (uint8*)((uint8*)mng + DOT11_MGMT_HDR_LEN);

	if ((fc & FC_KIND_MASK) == FC_ASSOC_REQ) {
		is_assoc = TRUE;
	} else if ((fc & FC_KIND_MASK) == FC_REASSOC_REQ) {
		is_assoc = FALSE;
	} else {
		printf("wl%d: 802.11 frame was not Assoc Req or Reassoc Req\n", wlc->pub->unit);
		printf("wl%d: frame body [%d bytes]:\n", wlc->pub->unit, body_len);
		prhex(NULL, body, body_len);
		return;
	}

	if ((is_assoc && body_len < DOT11_ASSOC_REQ_FIXED_LEN) ||
	    (!is_assoc && body_len < DOT11_REASSOC_REQ_FIXED_LEN)) {
		printf("wl%d: 802.11 %s Req body too short [%d bytes]:\n", wlc->pub->unit,
		       is_assoc?"Assoc":"Reassoc", body_len);
		prhex(NULL, body, body_len);
		return;
	}

	if (is_assoc) {
		struct dot11_assoc_req *assoc;
		assoc = (struct dot11_assoc_req *)body;
		cap = ltoh16(assoc->capability);
		listen_interval = ltoh16(assoc->listen);

		ies = body + DOT11_ASSOC_REQ_FIXED_LEN;
		ies_len = body_len - DOT11_ASSOC_REQ_FIXED_LEN;
	} else {
		struct dot11_reassoc_req *reassoc;
		reassoc = (struct dot11_reassoc_req *)body;
		cap = ltoh16(reassoc->capability);
		listen_interval = ltoh16(reassoc->listen);
		current_ap = &reassoc->ap;

		ies = body + DOT11_REASSOC_REQ_FIXED_LEN;
		ies_len = body_len - DOT11_REASSOC_REQ_FIXED_LEN;
	}

	bcm_format_flags(cap_flags, cap, flagstr, 128);

	if (flagstr[0] != '\0') {
		printf("wl%d: capabilities: %04x (%s)\n", wlc->pub->unit, cap, flagstr);
	} else {
		printf("wl%d: capabilities: %04x\n", wlc->pub->unit, cap);
	}

	printf("wl%d: listen interval: %u BIs\n", wlc->pub->unit, listen_interval);

	if (current_ap) {
		char eabuf[ETHER_ADDR_STR_LEN];
		printf("wl%d: current ap: %s\n", wlc->pub->unit, bcm_ether_ntoa(current_ap, eabuf));
	}

	wlc_print_ies(wlc, ies, ies_len);

	printf("\n");
}
#endif /* BCMDBG || WLMSG_PRPKT */

#endif /* STA */
