/******************************************************************************
 *
 * Copyright(c) 2009-2013  Realtek Corporation.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * The full GNU General Public License is included in this distribution in the
 * file called LICENSE.
 *
 * Contact Information:
 * wlanfae <wlanfae@realtek.com>
 * Realtek Corporation, No. 2, Innovation Road II, Hsinchu Science Park,
 * Hsinchu 300, Taiwan.
 *
 * Larry Finger <Larry.Finger@lwfinger.net>
 *
 *****************************************************************************/

#include "../wifi.h"
#include "../pci.h"
#include "../base.h"
#include "../stats.h"
#include "reg.h"
#include "def.h"
#include "phy.h"
#include "trx.h"
#include "led.h"
#include "dm.h"
#include "phy.h"

static u8 _rtl88ee_map_hwqueue_to_fwqueue(struct sk_buff *skb, u8 hw_queue)
{
	__le16 fc = rtl_get_fc(skb);

	if (unlikely(ieee80211_is_beacon(fc)))
		return QSLT_BEACON;
	if (ieee80211_is_mgmt(fc) || ieee80211_is_ctl(fc))
		return QSLT_MGNT;

	return skb->priority;
}

static void _rtl88ee_query_rxphystatus(struct ieee80211_hw *hw,
			struct rtl_stats *pstatus, u8 *pdesc,
			struct rx_fwinfo_88e *p_drvinfo,
			bool bpacket_match_bssid,
			bool bpacket_toself, bool packet_beacon)
{
	struct rtl_priv *rtlpriv = rtl_priv(hw);
	struct rtl_ps_ctl *ppsc = rtl_psc(rtlpriv);
	struct phy_sts_cck_8192s_t *cck_buf;
	struct phy_status_rpt *phystrpt =
		(struct phy_status_rpt *)p_drvinfo;
	struct rtl_dm *rtldm = rtl_dm(rtl_priv(hw));
	char rx_pwr_all = 0, rx_pwr[4];
	u8 rf_rx_num = 0, evm, pwdb_all;
	u8 i, max_spatial_stream;
	u32 rssi, total_rssi = 0;
	bool is_cck = pstatus->is_cck;
	u8 lan_idx, vga_idx;

	/* Record it for next packet processing */
	pstatus->packet_matchbssid = bpacket_match_bssid;
	pstatus->packet_toself = bpacket_toself;
	pstatus->packet_beacon = packet_beacon;
	pstatus->rx_mimo_signalquality[0] = -1;
	pstatus->rx_mimo_signalquality[1] = -1;

	if (is_cck) {
		u8 cck_highpwr;
		u8 cck_agc_rpt;
		/* CCK Driver info Structure is not the same as OFDM packet. */
		cck_buf = (struct phy_sts_cck_8192s_t *)p_drvinfo;
		cck_agc_rpt = cck_buf->cck_agc_rpt;

		/* (1)Hardware does not provide RSSI for CCK
		 * (2)PWDB, Average PWDB cacluated by
		 * hardware (for rate adaptive)
		 */
		if (ppsc->rfpwr_state == ERFON)
			cck_highpwr =
				(u8)rtl_get_bbreg(hw, RFPGA0_XA_HSSIPARAMETER2,
						  BIT(9));
		else
			cck_highpwr = false;

		lan_idx = ((cck_agc_rpt & 0xE0) >> 5);
		vga_idx = (cck_agc_rpt & 0x1f);
		switch (lan_idx) {
		case 7:
			if (vga_idx <= 27)
				/*VGA_idx = 27~2*/
				rx_pwr_all = -100 + 2*(27-vga_idx);
			else
				rx_pwr_all = -100;
			break;
		case 6:
			/*VGA_idx = 2~0*/
			rx_pwr_all = -48 + 2*(2-vga_idx);
			break;
		case 5:
			/*VGA_idx = 7~5*/
			rx_pwr_all = -42 + 2*(7-vga_idx);
			break;
		case 4:
			/*VGA_idx = 7~4*/
			rx_pwr_all = -36 + 2*(7-vga_idx);
			break;
		case 3:
			/*VGA_idx = 7~0*/
			rx_pwr_all = -24 + 2*(7-vga_idx);
			break;
		case 2:
			if (cck_highpwr)
				/*VGA_idx = 5~0*/
				rx_pwr_all = -12 + 2*(5-vga_idx);
			else
				rx_pwr_all = -6 + 2*(5-vga_idx);
			break;
		case 1:
			rx_pwr_all = 8-2*vga_idx;
			break;
		case 0:
			rx_pwr_all = 14-2*vga_idx;
			break;
		default:
			break;
		}
		rx_pwr_all += 6;
		pwdb_all = rtl_query_rxpwrpercentage(rx_pwr_all);
		/* CCK gain is smaller than OFDM/MCS gain,  */
		/* so we add gain diff by experiences, the val is 6 */
		pwdb_all += 6;
		if (pwdb_all > 100)
			pwdb_all = 100;
		/* modify the offset to make the same
		 * gain index with OFDM.
		 */
		if (pwdb_all > 34 && pwdb_all <= 42)
			pwdb_all -= 2;
		else if (pwdb_all > 26 && pwdb_all <= 34)
			pwdb_all -= 6;
		else if (pwdb_all > 14 && pwdb_all <= 26)
			pwdb_all -= 8;
		else if (pwdb_all > 4 && pwdb_all <= 14)
			pwdb_all -= 4;
		if (!cck_highpwr) {
			if (pwdb_all >= 80)
				pwdb_all = ((pwdb_all-80)<<1) +
					   ((pwdb_all-80)>>1) + 80;
			else if ((pwdb_all <= 78) && (pwdb_all >= 20))
				pwdb_all += 3;
			if (pwdb_all > 100)
				pwdb_all = 100;
		}

		pstatus->rx_pwdb_all = pwdb_all;
		pstatus->recvsignalpower = rx_pwr_all;

		/* (3) Get Signal Quality (EVM) */
		if (bpacket_match_bssid) {
			u8 sq;

			if (pstatus->rx_pwdb_all > 40)
				sq = 100;
			else {
				sq = cck_buf->sq_rpt;
				if (sq > 64)
					sq = 0;
				else if (sq < 20)
					sq = 100;
				else
					sq = ((64 - sq) * 100) / 44;
			}

			pstatus->signalquality = sq;
			pstatus->rx_mimo_signalquality[0] = sq;
			pstatus->rx_mimo_signalquality[1] = -1;
		}
	} else {
		rtlpriv->dm.rfpath_rxenable[0] =
		    rtlpriv->dm.rfpath_rxenable[1] = true;

		/* (1)Get RSSI for HT rate */
		for (i = RF90_PATH_A; i < RF6052_MAX_PATH; i++) {
			/* we will judge RF RX path now. */
			if (rtlpriv->dm.rfpath_rxenable[i])
				rf_rx_num++;

			rx_pwr[i] = ((p_drvinfo->gain_trsw[i] &
				      0x3f) * 2) - 110;

			/* Translate DBM to percentage. */
			rssi = rtl_query_rxpwrpercentage(rx_pwr[i]);
			total_rssi += rssi;

			/* Get Rx snr value in DB */
			rtlpriv->stats.rx_snr_db[i] =
				(long)(p_drvinfo->rxsnr[i] / 2);

			/* Record Signal Strength for next packet */
			if (bpacket_match_bssid)
				pstatus->rx_mimo_signalstrength[i] = (u8)rssi;
		}

		/* (2)PWDB, Average PWDB cacluated by
		 * hardware (for rate adaptive)
		 */
		rx_pwr_all = ((p_drvinfo->pwdb_all >> 1) & 0x7f) - 110;

		pwdb_all = rtl_query_rxpwrpercentage(rx_pwr_all);
		pstatus->rx_pwdb_all = pwdb_all;
		pstatus->rxpower = rx_pwr_all;
		pstatus->recvsignalpower = rx_pwr_all;

		/* (3)EVM of HT rate */
		if (pstatus->is_ht && pstatus->rate >= DESC92C_RATEMCS8 &&
		    pstatus->rate <= DESC92C_RATEMCS15)
			max_spatial_stream = 2;
		else
			max_spatial_stream = 1;

		for (i = 0; i < max_spatial_stream; i++) {
			evm = rtl_evm_db_to_percentage(p_drvinfo->rxevm[i]);

			if (bpacket_match_bssid) {
				/* Fill value in RFD, Get the first
				 * spatial stream onlyi
				 */
				if (i == 0)
					pstatus->signalquality =
						(u8)(evm & 0xff);
				pstatus->rx_mimo_signalquality[i] =
					(u8)(evm & 0xff);
			}
		}
	}

	/* UI BSS List signal strength(in percentage),
	 * make it good looking, from 0~100.
	 */
	if (is_cck)
		pstatus->signalstrength = (u8)(rtl_signal_scale_mapping(hw,
			pwdb_all));
	else if (rf_rx_num != 0)
		pstatus->signalstrength = (u8)(rtl_signal_scale_mapping(hw,
			total_rssi /= rf_rx_num));
	/*HW antenna diversity*/
	rtldm->fat_table.antsel_rx_keep_0 = phystrpt->ant_sel;
	rtldm->fat_table.antsel_rx_keep_1 = phystrpt->ant_sel_b;
	rtldm->fat_table.antsel_rx_keep_2 = phystrpt->antsel_rx_keep_2;
}

static void _rtl88ee_smart_antenna(struct ieee80211_hw *hw,
	struct rtl_stats *pstatus)
{
	struct rtl_dm *rtldm = rtl_dm(rtl_priv(hw));
	struct rtl_efuse *rtlefuse = rtl_efuse(rtl_priv(hw));
	u8 antsel_tr_mux;
	struct fast_ant_training *pfat_table = &rtldm->fat_table;

	if (rtlefuse->antenna_div_type == CG_TRX_SMART_ANTDIV) {
		if (pfat_table->fat_state == FAT_TRAINING_STATE) {
			if (pstatus->packet_toself) {
				antsel_tr_mux =
					(pfat_table->antsel_rx_keep_2 << 2) |
					(pfat_table->antsel_rx_keep_1 << 1) |
					pfat_table->antsel_rx_keep_0;
				pfat_table->ant_sum[antsel_tr_mux] +=
					pstatus->rx_pwdb_all;
				pfat_table->ant_cnt[antsel_tr_mux]++;
			}
		}
	} else if ((rtlefuse->antenna_div_type == CG_TRX_HW_ANTDIV) ||
	(rtlefuse->antenna_div_type == CGCS_RX_HW_ANTDIV)) {
		if (pstatus->packet_toself || pstatus->packet_matchbssid) {
			antsel_tr_mux = (pfat_table->antsel_rx_keep_2 << 2) |
					(pfat_table->antsel_rx_keep_1 << 1) |
					pfat_table->antsel_rx_keep_0;
			rtl88e_dm_ant_sel_statistics(hw, antsel_tr_mux, 0,
						     pstatus->rx_pwdb_all);
		}

	}
}

static void _rtl88ee_translate_rx_signal_stuff(struct ieee80211_hw *hw,
					       struct sk_buff *skb,
					       struct rtl_stats *pstatus,
					       u8 *pdesc,
					       struct rx_fwinfo_88e *p_drvinfo)
{
	struct rtl_mac *mac = rtl_mac(rtl_priv(hw));
	struct rtl_efuse *rtlefuse = rtl_efuse(rtl_priv(hw));
	struct ieee80211_hdr *hdr;
	u8 *tmp_buf;
	u8 *praddr;
	u8 *psaddr;
	__le16 fc;
	bool packet_matchbssid, packet_toself, packet_beacon;

	tmp_buf = skb->data + pstatus->rx_drvinfo_size + pstatus->rx_bufshift;

	hdr = (struct ieee80211_hdr *)tmp_buf;
	fc = hdr->frame_control;
	praddr = hdr->addr1;
	psaddr = ieee80211_get_SA(hdr);
	memcpy(pstatus->psaddr, psaddr, ETH_ALEN);

	packet_matchbssid = ((!ieee80211_is_ctl(fc)) &&
	     (ether_addr_equal(mac->bssid, ieee80211_has_tods(fc) ?
			       hdr->addr1 : ieee80211_has_fromds(fc) ?
			       hdr->addr2 : hdr->addr3)) &&
			       (!pstatus->hwerror) &&
			       (!pstatus->crc) && (!pstatus->icv));

	packet_toself = packet_matchbssid &&
	    (ether_addr_equal(praddr, rtlefuse->dev_addr));

	if (ieee80211_is_beacon(hdr->frame_control))
		packet_beacon = true;
	else
		packet_beacon = false;

	_rtl88ee_query_rxphystatus(hw, pstatus, pdesc, p_drvinfo,
				   packet_matchbssid, packet_toself,
				   packet_beacon);
	_rtl88ee_smart_antenna(hw, pstatus);
	rtl_process_phyinfo(hw, tmp_buf, pstatus);
}

static void _rtl88ee_insert_emcontent(struct rtl_tcb_desc *ptcb_desc,
				      u8 *virtualaddress)
{
	u32 dwtmp = 0;
	memset(virtualaddress, 0, 8);

	SET_EARLYMODE_PKTNUM(virtualaddress, ptcb_desc->empkt_num);
	if (ptcb_desc->empkt_num == 1) {
		dwtmp = ptcb_desc->empkt_len[0];
	} else {
		dwtmp = ptcb_desc->empkt_len[0];
		dwtmp += ((dwtmp%4) ? (4-dwtmp%4) : 0)+4;
		dwtmp += ptcb_desc->empkt_len[1];
	}
	SET_EARLYMODE_LEN0(virtualaddress, dwtmp);

	if (ptcb_desc->empkt_num <= 3) {
		dwtmp = ptcb_desc->empkt_len[2];
	} else {
		dwtmp = ptcb_desc->empkt_len[2];
		dwtmp += ((dwtmp%4) ? (4-dwtmp%4) : 0)+4;
		dwtmp += ptcb_desc->empkt_len[3];
	}
	SET_EARLYMODE_LEN1(virtualaddress, dwtmp);
	if (ptcb_desc->empkt_num <= 5) {
		dwtmp = ptcb_desc->empkt_len[4];
	} else {
		dwtmp = ptcb_desc->empkt_len[4];
		dwtmp += ((dwtmp%4) ? (4-dwtmp%4) : 0)+4;
		dwtmp += ptcb_desc->empkt_len[5];
	}
	SET_EARLYMODE_LEN2_1(virtualaddress, dwtmp & 0xF);
	SET_EARLYMODE_LEN2_2(virtualaddress, dwtmp >> 4);
	if (ptcb_desc->empkt_num <= 7) {
		dwtmp = ptcb_desc->empkt_len[6];
	} else {
		dwtmp = ptcb_desc->empkt_len[6];
		dwtmp += ((dwtmp%4) ? (4-dwtmp%4) : 0)+4;
		dwtmp += ptcb_desc->empkt_len[7];
	}
	SET_EARLYMODE_LEN3(virtualaddress, dwtmp);
	if (ptcb_desc->empkt_num <= 9) {
		dwtmp = ptcb_desc->empkt_len[8];
	} else {
		dwtmp = ptcb_desc->empkt_len[8];
		dwtmp += ((dwtmp%4) ? (4-dwtmp%4) : 0)+4;
		dwtmp += ptcb_desc->empkt_len[9];
	}
	SET_EARLYMODE_LEN4(virtualaddress, dwtmp);
}

bool rtl88ee_rx_query_desc(struct ieee80211_hw *hw,
			   struct rtl_stats *status,
			   struct ieee80211_rx_status *rx_status,
			   u8 *pdesc, struct sk_buff *skb)
{
	struct rtl_priv *rtlpriv = rtl_priv(hw);
	struct rx_fwinfo_88e *p_drvinfo;
	struct ieee80211_hdr *hdr;

	u32 phystatus = GET_RX_DESC_PHYST(pdesc);
	status->packet_report_type = (u8)GET_RX_STATUS_DESC_RPT_SEL(pdesc);
	if (status->packet_report_type == TX_REPORT2)
		status->length = (u16)GET_RX_RPT2_DESC_PKT_LEN(pdesc);
	else
		status->length = (u16)GET_RX_DESC_PKT_LEN(pdesc);
	status->rx_drvinfo_size = (u8)GET_RX_DESC_DRV_INFO_SIZE(pdesc) *
	    RX_DRV_INFO_SIZE_UNIT;
	status->rx_bufshift = (u8)(GET_RX_DESC_SHIFT(pdesc) & 0x03);
	status->icv = (u16)GET_RX_DESC_ICV(pdesc);
	status->crc = (u16)GET_RX_DESC_CRC32(pdesc);
	status->hwerror = (status->crc | status->icv);
	status->decrypted = !GET_RX_DESC_SWDEC(pdesc);
	status->rate = (u8)GET_RX_DESC_RXMCS(pdesc);
	status->shortpreamble = (u16)GET_RX_DESC_SPLCP(pdesc);
	status->isampdu = (bool) (GET_RX_DESC_PAGGR(pdesc) == 1);
	status->isfirst_ampdu = (bool)((GET_RX_DESC_PAGGR(pdesc) == 1) &&
				(GET_RX_DESC_FAGGR(pdesc) == 1));
	if (status->packet_report_type == NORMAL_RX)
		status->timestamp_low = GET_RX_DESC_TSFL(pdesc);
	status->rx_is40Mhzpacket = (bool) GET_RX_DESC_BW(pdesc);
	status->is_ht = (bool)GET_RX_DESC_RXHT(pdesc);

	status->is_cck = RTL8188_RX_HAL_IS_CCK_RATE(status->rate);

	status->macid = GET_RX_DESC_MACID(pdesc);
	if (GET_RX_STATUS_DESC_MAGIC_MATCH(pdesc))
		status->wake_match = BIT(2);
	else if (GET_RX_STATUS_DESC_MAGIC_MATCH(pdesc))
		status->wake_match = BIT(1);
	else if (GET_RX_STATUS_DESC_UNICAST_MATCH(pdesc))
		status->wake_match = BIT(0);
	else
		status->wake_match = 0;
	if (status->wake_match)
		RT_TRACE(rtlpriv, COMP_RXDESC, DBG_LOUD,
		"GGGGGGGGGGGGGet Wakeup Packet!! WakeMatch=%d\n",
		status->wake_match);
	rx_status->freq = hw->conf.chandef.chan->center_freq;
	rx_status->band = hw->conf.chandef.chan->band;

	hdr = (struct ieee80211_hdr *)(skb->data + status->rx_drvinfo_size
			+ status->rx_bufshift);

	if (status->crc)
		rx_status->flag |= RX_FLAG_FAILED_FCS_CRC;

	if (status->rx_is40Mhzpacket)
		rx_status->flag |= RX_FLAG_40MHZ;

	if (status->is_ht)
		rx_status->flag |= RX_FLAG_HT;

	rx_status->flag |= RX_FLAG_MACTIME_START;

	/* hw will set status->decrypted true, if it finds the
	 * frame is open data frame or mgmt frame.
	 * So hw will not decryption robust managment frame
	 * for IEEE80211w but still set status->decrypted
	 * true, so here we should set it back to undecrypted
	 * for IEEE80211w frame, and mac80211 sw will help
	 * to decrypt it
	 */
	if (status->decrypted) {
		if ((!_ieee80211_is_robust_mgmt_frame(hdr)) &&
		    (ieee80211_has_protected(hdr->frame_control)))
			rx_status->flag |= RX_FLAG_DECRYPTED;
		else
			rx_status->flag &= ~RX_FLAG_DECRYPTED;
	}

	/* rate_idx: index of data rate into band's
	 * supported rates or MCS index if HT rates
	 * are use (RX_FLAG_HT)
	 * Notice: this is diff with windows define
	 */
	rx_status->rate_idx = rtlwifi_rate_mapping(hw, status->is_ht,
						   false, status->rate);

	rx_status->mactime = status->timestamp_low;
	if (phystatus == true) {
		p_drvinfo = (struct rx_fwinfo_88e *)(skb->data +
						     status->rx_bufshift);

		_rtl88ee_translate_rx_signal_stuff(hw,
						   skb, status, pdesc,
						   p_drvinfo);
	}
	rx_status->signal = status->recvsignalpower + 10;
	if (status->packet_report_type == TX_REPORT2) {
		status->macid_valid_entry[0] =
			 GET_RX_RPT2_DESC_MACID_VALID_1(pdesc);
		status->macid_valid_entry[1] =
			 GET_RX_RPT2_DESC_MACID_VALID_2(pdesc);
	}
	return true;
}

void rtl88ee_tx_fill_desc(struct ieee80211_hw *hw,
			  struct ieee80211_hdr *hdr, u8 *pdesc_tx,
			  u8 *txbd, struct ieee80211_tx_info *info,
			  struct ieee80211_sta *sta,
			  struct sk_buff *skb,
			  u8 hw_queue, struct rtl_tcb_desc *ptcb_desc)

{
	struct rtl_priv *rtlpriv = rtl_priv(hw);
	struct rtl_mac *mac = rtl_mac(rtl_priv(hw));
	struct rtl_pci *rtlpci = rtl_pcidev(rtl_pcipriv(hw));
	struct rtl_hal *rtlhal = rtl_hal(rtlpriv);
	u8 *pdesc = (u8 *)pdesc_tx;
	u16 seq_number;
	__le16 fc = hdr->frame_control;
	unsigned int buf_len = 0;
	unsigned int skb_len = skb->len;
	u8 fw_qsel = _rtl88ee_map_hwqueue_to_fwqueue(skb, hw_queue);
	bool firstseg = ((hdr->seq_ctrl &
			    cpu_to_le16(IEEE80211_SCTL_FRAG)) == 0);
	bool lastseg = ((hdr->frame_control &
			   cpu_to_le16(IEEE80211_FCTL_MOREFRAGS)) == 0);
	dma_addr_t mapping;
	u8 bw_40 = 0;
	u8 short_gi = 0;

	if (mac->opmode == NL80211_IFTYPE_STATION) {
		bw_40 = mac->bw_40;
	} else if (mac->opmode == NL80211_IFTYPE_AP ||
		mac->opmode == NL80211_IFTYPE_ADHOC) {
		if (sta)
			bw_40 = sta->ht_cap.cap &
				IEEE80211_HT_CAP_SUP_WIDTH_20_40;
	}
	seq_number = (le16_to_cpu(hdr->seq_ctrl) & IEEE80211_SCTL_SEQ) >> 4;
	rtl_get_tcb_desc(hw, info, sta, skb, ptcb_desc);
	/* reserve 8 byte for AMPDU early mode */
	if (rtlhal->earlymode_enable) {
		skb_push(skb, EM_HDR_LEN);
		memset(skb->data, 0, EM_HDR_LEN);
	}
	buf_len = skb->len;
	mapping = pci_map_single(rtlpci->pdev, skb->data, skb->len,
				 PCI_DMA_TODEVICE);
	if (pci_dma_mapping_error(rtlpci->pdev, mapping)) {
		RT_TRACE(rtlpriv, COMP_SEND, DBG_TRACE,
			 "DMA mapping error");
		return;
	}
	CLEAR_PCI_TX_DESC_CONTENT(pdesc, sizeof(struct tx_desc_88e));
	if (ieee80211_is_nullfunc(fc) || ieee80211_is_ctl(fc)) {
		firstseg = true;
		lastseg = true;
	}
	if (firstseg) {
		if (rtlhal->earlymode_enable) {
			SET_TX_DESC_PKT_OFFSET(pdesc, 1);
			SET_TX_DESC_OFFSET(pdesc, USB_HWDESC_HEADER_LEN +
					   EM_HDR_LEN);
			if (ptcb_desc->empkt_num) {
				RT_TRACE(rtlpriv, COMP_SEND, DBG_TRACE,
					 "Insert 8 byte.pTcb->EMPktNum:%d\n",
					  ptcb_desc->empkt_num);
				_rtl88ee_insert_emcontent(ptcb_desc,
							  (u8 *)(skb->data));
			}
		} else {
			SET_TX_DESC_OFFSET(pdesc, USB_HWDESC_HEADER_LEN);
		}

		ptcb_desc->use_driver_rate = true;
		SET_TX_DESC_TX_RATE(pdesc, ptcb_desc->hw_rate);
		if (ptcb_desc->hw_rate > DESC92C_RATEMCS0)
			short_gi = (ptcb_desc->use_shortgi) ? 1 : 0;
		else
			short_gi = (ptcb_desc->use_shortpreamble) ? 1 : 0;

		SET_TX_DESC_DATA_SHORTGI(pdesc, short_gi);

		if (info->flags & IEEE80211_TX_CTL_AMPDU) {
			SET_TX_DESC_AGG_ENABLE(pdesc, 1);
			SET_TX_DESC_MAX_AGG_NUM(pdesc, 0x14);
		}
		SET_TX_DESC_SEQ(pdesc, seq_number);
		SET_TX_DESC_RTS_ENABLE(pdesc, ((ptcb_desc->rts_enable &&
						!ptcb_desc->cts_enable) ? 1 : 0));
		SET_TX_DESC_HW_RTS_ENABLE(pdesc, 0);
		SET_TX_DESC_CTS2SELF(pdesc, ((ptcb_desc->cts_enable) ? 1 : 0));
		SET_TX_DESC_RTS_STBC(pdesc, ((ptcb_desc->rts_stbc) ? 1 : 0));

		SET_TX_DESC_RTS_RATE(pdesc, ptcb_desc->rts_rate);
		SET_TX_DESC_RTS_BW(pdesc, 0);
		SET_TX_DESC_RTS_SC(pdesc, ptcb_desc->rts_sc);
		SET_TX_DESC_RTS_SHORT(pdesc,
			((ptcb_desc->rts_rate <= DESC92C_RATE54M) ?
			(ptcb_desc->rts_use_shortpreamble ? 1 : 0) :
			(ptcb_desc->rts_use_shortgi ? 1 : 0)));

		if (ptcb_desc->tx_enable_sw_calc_duration)
			SET_TX_DESC_NAV_USE_HDR(pdesc, 1);

		if (bw_40) {
			if (ptcb_desc->packet_bw == HT_CHANNEL_WIDTH_20_40) {
				SET_TX_DESC_DATA_BW(pdesc, 1);
				SET_TX_DESC_TX_SUB_CARRIER(pdesc, 3);
			} else {
				SET_TX_DESC_DATA_BW(pdesc, 0);
				SET_TX_DESC_TX_SUB_CARRIER(pdesc,
							   mac->cur_40_prime_sc);
			}
		} else {
			SET_TX_DESC_DATA_BW(pdesc, 0);
			SET_TX_DESC_TX_SUB_CARRIER(pdesc, 0);
		}

		SET_TX_DESC_LINIP(pdesc, 0);
		SET_TX_DESC_PKT_SIZE(pdesc, (u16)skb_len);
		if (sta) {
			u8 ampdu_density = sta->ht_cap.ampdu_density;
			SET_TX_DESC_AMPDU_DENSITY(pdesc, ampdu_density);
		}
		if (info->control.hw_key) {
			struct ieee80211_key_conf *keyconf;

			keyconf = info->control.hw_key;
			switch (keyconf->cipher) {
			case WLAN_CIPHER_SUITE_WEP40:
			case WLAN_CIPHER_SUITE_WEP104:
			case WLAN_CIPHER_SUITE_TKIP:
				SET_TX_DESC_SEC_TYPE(pdesc, 0x1);
				break;
			case WLAN_CIPHER_SUITE_CCMP:
				SET_TX_DESC_SEC_TYPE(pdesc, 0x3);
				break;
			default:
				SET_TX_DESC_SEC_TYPE(pdesc, 0x0);
				break;

			}
		}

		SET_TX_DESC_QUEUE_SEL(pdesc, fw_qsel);
		SET_TX_DESC_DATA_RATE_FB_LIMIT(pdesc, 0x1F);
		SET_TX_DESC_RTS_RATE_FB_LIMIT(pdesc, 0xF);
		SET_TX_DESC_DISABLE_FB(pdesc, ptcb_desc->disable_ratefallback ?
				       1 : 0);
		SET_TX_DESC_USE_RATE(pdesc, ptcb_desc->use_driver_rate ? 1 : 0);

		/*SET_TX_DESC_PWR_STATUS(pdesc, pwr_status);*/
		/* Set TxRate and RTSRate in TxDesc  */
		/* This prevent Tx initial rate of new-coming packets */
		/* from being overwritten by retried  packet rate.*/
		if (!ptcb_desc->use_driver_rate) {
			/*SET_TX_DESC_RTS_RATE(pdesc, 0x08); */
			/* SET_TX_DESC_TX_RATE(pdesc, 0x0b); */
		}
		if (ieee80211_is_data_qos(fc)) {
			if (mac->rdg_en) {
				RT_TRACE(rtlpriv, COMP_SEND, DBG_TRACE,
					"Enable RDG function.\n");
				SET_TX_DESC_RDG_ENABLE(pdesc, 1);
				SET_TX_DESC_HTC(pdesc, 1);
			}
		}
	}

	SET_TX_DESC_FIRST_SEG(pdesc, (firstseg ? 1 : 0));
	SET_TX_DESC_LAST_SEG(pdesc, (lastseg ? 1 : 0));
	SET_TX_DESC_TX_BUFFER_SIZE(pdesc, (u16)buf_len);
	SET_TX_DESC_TX_BUFFER_ADDRESS(pdesc, mapping);
	if (rtlpriv->dm.useramask) {
		SET_TX_DESC_RATE_ID(pdesc, ptcb_desc->ratr_index);
		SET_TX_DESC_MACID(pdesc, ptcb_desc->mac_id);
	} else {
		SET_TX_DESC_RATE_ID(pdesc, 0xC + ptcb_desc->ratr_index);
		SET_TX_DESC_MACID(pdesc, ptcb_desc->ratr_index);
	}
	if (ieee80211_is_data_qos(fc))
		SET_TX_DESC_QOS(pdesc, 1);

	if (!ieee80211_is_data_qos(fc))
		SET_TX_DESC_HWSEQ_EN(pdesc, 1);
	SET_TX_DESC_MORE_FRAG(pdesc, (lastseg ? 0 : 1));
	if (is_multicast_ether_addr(ieee80211_get_DA(hdr)) ||
	    is_broadcast_ether_addr(ieee80211_get_DA(hdr))) {
		SET_TX_DESC_BMC(pdesc, 1);
	}

	rtl88e_dm_set_tx_ant_by_tx_info(hw, pdesc, ptcb_desc->mac_id);
	RT_TRACE(rtlpriv, COMP_SEND, DBG_TRACE, "\n");
}

void rtl88ee_tx_fill_cmddesc(struct ieee80211_hw *hw,
			     u8 *pdesc, bool firstseg,
			     bool lastseg, struct sk_buff *skb)
{
	struct rtl_priv *rtlpriv = rtl_priv(hw);
	struct rtl_pci *rtlpci = rtl_pcidev(rtl_pcipriv(hw));
	u8 fw_queue = QSLT_BEACON;

	dma_addr_t mapping = pci_map_single(rtlpci->pdev,
					    skb->data, skb->len,
					    PCI_DMA_TODEVICE);

	struct ieee80211_hdr *hdr = (struct ieee80211_hdr *)(skb->data);
	__le16 fc = hdr->frame_control;

	if (pci_dma_mapping_error(rtlpci->pdev, mapping)) {
		RT_TRACE(rtlpriv, COMP_SEND, DBG_TRACE,
			 "DMA mapping error");
		return;
	}
	CLEAR_PCI_TX_DESC_CONTENT(pdesc, TX_DESC_SIZE);

	if (firstseg)
		SET_TX_DESC_OFFSET(pdesc, USB_HWDESC_HEADER_LEN);

	SET_TX_DESC_TX_RATE(pdesc, DESC92C_RATE1M);

	SET_TX_DESC_SEQ(pdesc, 0);

	SET_TX_DESC_LINIP(pdesc, 0);

	SET_TX_DESC_QUEUE_SEL(pdesc, fw_queue);

	SET_TX_DESC_FIRST_SEG(pdesc, 1);
	SET_TX_DESC_LAST_SEG(pdesc, 1);

	SET_TX_DESC_TX_BUFFER_SIZE(pdesc, (u16)(skb->len));

	SET_TX_DESC_TX_BUFFER_ADDRESS(pdesc, mapping);

	SET_TX_DESC_RATE_ID(pdesc, 7);
	SET_TX_DESC_MACID(pdesc, 0);

	SET_TX_DESC_OWN(pdesc, 1);

	SET_TX_DESC_PKT_SIZE(pdesc, (u16)(skb->len));

	SET_TX_DESC_FIRST_SEG(pdesc, 1);
	SET_TX_DESC_LAST_SEG(pdesc, 1);

	SET_TX_DESC_OFFSET(pdesc, 0x20);

	SET_TX_DESC_USE_RATE(pdesc, 1);

	if (!ieee80211_is_data_qos(fc))
		SET_TX_DESC_HWSEQ_EN(pdesc, 1);

	RT_PRINT_DATA(rtlpriv, COMP_CMD, DBG_LOUD,
		      "H2C Tx Cmd Content\n",
		      pdesc, TX_DESC_SIZE);
}

void rtl88ee_set_desc(struct ieee80211_hw *hw, u8 *pdesc,
		      bool istx, u8 desc_name, u8 *val)
{
	if (istx == true) {
		switch (desc_name) {
		case HW_DESC_OWN:
			SET_TX_DESC_OWN(pdesc, 1);
			break;
		case HW_DESC_TX_NEXTDESC_ADDR:
			SET_TX_DESC_NEXT_DESC_ADDRESS(pdesc, *(u32 *)val);
			break;
		default:
			RT_ASSERT(false, "ERR txdesc :%d not process\n",
				  desc_name);
			break;
		}
	} else {
		switch (desc_name) {
		case HW_DESC_RXOWN:
			SET_RX_DESC_OWN(pdesc, 1);
			break;
		case HW_DESC_RXBUFF_ADDR:
			SET_RX_DESC_BUFF_ADDR(pdesc, *(u32 *)val);
			break;
		case HW_DESC_RXPKT_LEN:
			SET_RX_DESC_PKT_LEN(pdesc, *(u32 *)val);
			break;
		case HW_DESC_RXERO:
			SET_RX_DESC_EOR(pdesc, 1);
			break;
		default:
			RT_ASSERT(false, "ERR rxdesc :%d not process\n",
				  desc_name);
			break;
		}
	}
}

u32 rtl88ee_get_desc(u8 *pdesc, bool istx, u8 desc_name)
{
	u32 ret = 0;

	if (istx == true) {
		switch (desc_name) {
		case HW_DESC_OWN:
			ret = GET_TX_DESC_OWN(pdesc);
			break;
		case HW_DESC_TXBUFF_ADDR:
			ret = GET_TX_DESC_TX_BUFFER_ADDRESS(pdesc);
			break;
		default:
			RT_ASSERT(false, "ERR txdesc :%d not process\n",
				  desc_name);
			break;
		}
	} else {
		switch (desc_name) {
		case HW_DESC_OWN:
			ret = GET_RX_DESC_OWN(pdesc);
			break;
		case HW_DESC_RXPKT_LEN:
			ret = GET_RX_DESC_PKT_LEN(pdesc);
			break;
		case HW_DESC_RXBUFF_ADDR:
			ret = GET_RX_DESC_BUFF_ADDR(pdesc);
			break;
		default:
			RT_ASSERT(false, "ERR rxdesc :%d not process\n",
				  desc_name);
			break;
		}
	}
	return ret;
}

bool rtl88ee_is_tx_desc_closed(struct ieee80211_hw *hw, u8 hw_queue, u16 index)
{
	struct rtl_pci *rtlpci = rtl_pcidev(rtl_pcipriv(hw));
	struct rtl8192_tx_ring *ring = &rtlpci->tx_ring[hw_queue];
	u8 *entry = (u8 *)(&ring->desc[ring->idx]);
	u8 own = (u8)rtl88ee_get_desc(entry, true, HW_DESC_OWN);

	/*beacon packet will only use the first
	 *descriptor defautly,and the own may not
	 *be cleared by the hardware
	 */
	if (own)
		return false;
	return true;
}

void rtl88ee_tx_polling(struct ieee80211_hw *hw, u8 hw_queue)
{
	struct rtl_priv *rtlpriv = rtl_priv(hw);
	if (hw_queue == BEACON_QUEUE) {
		rtl_write_word(rtlpriv, REG_PCIE_CTRL_REG, BIT(4));
	} else {
		rtl_write_word(rtlpriv, REG_PCIE_CTRL_REG,
			       BIT(0) << (hw_queue));
	}
}

u32 rtl88ee_rx_command_packet(struct ieee80211_hw *hw,
			      struct rtl_stats status,
			      struct sk_buff *skb)
{
	return 0;
}
