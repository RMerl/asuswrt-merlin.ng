/******************************************************************************
 *
 * This file is provided under a dual BSD/GPLv2 license.  When using or
 * redistributing this file, you may do so under either license.
 *
 * GPL LICENSE SUMMARY
 *
 * Copyright(c) 2012 - 2014 Intel Corporation. All rights reserved.
 * Copyright(c) 2013 - 2015 Intel Mobile Communications GmbH
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110,
 * USA
 *
 * The full GNU General Public License is included in this distribution
 * in the file called COPYING.
 *
 * Contact Information:
 *  Intel Linux Wireless <ilw@linux.intel.com>
 * Intel Corporation, 5200 N.E. Elam Young Parkway, Hillsboro, OR 97124-6497
 *
 * BSD LICENSE
 *
 * Copyright(c) 2012 - 2014 Intel Corporation. All rights reserved.
 * Copyright(c) 2013 - 2015 Intel Mobile Communications GmbH
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 *  * Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *  * Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 *  * Neither the name Intel Corporation nor the names of its
 *    contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *****************************************************************************/
#include "iwl-trans.h"
#include "mvm.h"
#include "fw-api.h"

/*
 * iwl_mvm_rx_rx_phy_cmd - REPLY_RX_PHY_CMD handler
 *
 * Copies the phy information in mvm->last_phy_info, it will be used when the
 * actual data will come from the fw in the next packet.
 */
int iwl_mvm_rx_rx_phy_cmd(struct iwl_mvm *mvm, struct iwl_rx_cmd_buffer *rxb,
			  struct iwl_device_cmd *cmd)
{
	struct iwl_rx_packet *pkt = rxb_addr(rxb);

	memcpy(&mvm->last_phy_info, pkt->data, sizeof(mvm->last_phy_info));
	mvm->ampdu_ref++;

#ifdef CONFIG_IWLWIFI_DEBUGFS
	if (mvm->last_phy_info.phy_flags & cpu_to_le16(RX_RES_PHY_FLAGS_AGG)) {
		spin_lock(&mvm->drv_stats_lock);
		mvm->drv_rx_stats.ampdu_count++;
		spin_unlock(&mvm->drv_stats_lock);
	}
#endif

	return 0;
}

/*
 * iwl_mvm_pass_packet_to_mac80211 - builds the packet for mac80211
 *
 * Adds the rxb to a new skb and give it to mac80211
 */
static void iwl_mvm_pass_packet_to_mac80211(struct iwl_mvm *mvm,
					    struct sk_buff *skb,
					    struct ieee80211_hdr *hdr, u16 len,
					    u32 ampdu_status, u8 crypt_len,
					    struct iwl_rx_cmd_buffer *rxb)
{
	unsigned int hdrlen, fraglen;

	/* If frame is small enough to fit in skb->head, pull it completely.
	 * If not, only pull ieee80211_hdr (including crypto if present, and
	 * an additional 8 bytes for SNAP/ethertype, see below) so that
	 * splice() or TCP coalesce are more efficient.
	 *
	 * Since, in addition, ieee80211_data_to_8023() always pull in at
	 * least 8 bytes (possibly more for mesh) we can do the same here
	 * to save the cost of doing it later. That still doesn't pull in
	 * the actual IP header since the typical case has a SNAP header.
	 * If the latter changes (there are efforts in the standards group
	 * to do so) we should revisit this and ieee80211_data_to_8023().
	 */
	hdrlen = (len <= skb_tailroom(skb)) ? len :
					      sizeof(*hdr) + crypt_len + 8;

	memcpy(skb_put(skb, hdrlen), hdr, hdrlen);
	fraglen = len - hdrlen;

	if (fraglen) {
		int offset = (void *)hdr + hdrlen -
			     rxb_addr(rxb) + rxb_offset(rxb);

		skb_add_rx_frag(skb, 0, rxb_steal_page(rxb), offset,
				fraglen, rxb->truesize);
	}

	ieee80211_rx(mvm->hw, skb);
}

/*
 * iwl_mvm_get_signal_strength - use new rx PHY INFO API
 * values are reported by the fw as positive values - need to negate
 * to obtain their dBM.  Account for missing antennas by replacing 0
 * values by -256dBm: practically 0 power and a non-feasible 8 bit value.
 */
static void iwl_mvm_get_signal_strength(struct iwl_mvm *mvm,
					struct iwl_rx_phy_info *phy_info,
					struct ieee80211_rx_status *rx_status)
{
	int energy_a, energy_b, energy_c, max_energy;
	u32 val;

	val =
	    le32_to_cpu(phy_info->non_cfg_phy[IWL_RX_INFO_ENERGY_ANT_ABC_IDX]);
	energy_a = (val & IWL_RX_INFO_ENERGY_ANT_A_MSK) >>
						IWL_RX_INFO_ENERGY_ANT_A_POS;
	energy_a = energy_a ? -energy_a : S8_MIN;
	energy_b = (val & IWL_RX_INFO_ENERGY_ANT_B_MSK) >>
						IWL_RX_INFO_ENERGY_ANT_B_POS;
	energy_b = energy_b ? -energy_b : S8_MIN;
	energy_c = (val & IWL_RX_INFO_ENERGY_ANT_C_MSK) >>
						IWL_RX_INFO_ENERGY_ANT_C_POS;
	energy_c = energy_c ? -energy_c : S8_MIN;
	max_energy = max(energy_a, energy_b);
	max_energy = max(max_energy, energy_c);

	IWL_DEBUG_STATS(mvm, "energy In A %d B %d C %d , and max %d\n",
			energy_a, energy_b, energy_c, max_energy);

	rx_status->signal = max_energy;
	rx_status->chains = (le16_to_cpu(phy_info->phy_flags) &
				RX_RES_PHY_FLAGS_ANTENNA)
					>> RX_RES_PHY_FLAGS_ANTENNA_POS;
	rx_status->chain_signal[0] = energy_a;
	rx_status->chain_signal[1] = energy_b;
	rx_status->chain_signal[2] = energy_c;
}

/*
 * iwl_mvm_set_mac80211_rx_flag - translate fw status to mac80211 format
 * @mvm: the mvm object
 * @hdr: 80211 header
 * @stats: status in mac80211's format
 * @rx_pkt_status: status coming from fw
 *
 * returns non 0 value if the packet should be dropped
 */
static u32 iwl_mvm_set_mac80211_rx_flag(struct iwl_mvm *mvm,
					struct ieee80211_hdr *hdr,
					struct ieee80211_rx_status *stats,
					u32 rx_pkt_status,
					u8 *crypt_len)
{
	if (!ieee80211_has_protected(hdr->frame_control) ||
	    (rx_pkt_status & RX_MPDU_RES_STATUS_SEC_ENC_MSK) ==
			     RX_MPDU_RES_STATUS_SEC_NO_ENC)
		return 0;

	/* packet was encrypted with unknown alg */
	if ((rx_pkt_status & RX_MPDU_RES_STATUS_SEC_ENC_MSK) ==
					RX_MPDU_RES_STATUS_SEC_ENC_ERR)
		return 0;

	switch (rx_pkt_status & RX_MPDU_RES_STATUS_SEC_ENC_MSK) {
	case RX_MPDU_RES_STATUS_SEC_CCM_ENC:
		/* alg is CCM: check MIC only */
		if (!(rx_pkt_status & RX_MPDU_RES_STATUS_MIC_OK))
			return -1;

		stats->flag |= RX_FLAG_DECRYPTED;
		IWL_DEBUG_WEP(mvm, "hw decrypted CCMP successfully\n");
		*crypt_len = IEEE80211_CCMP_HDR_LEN;
		return 0;

	case RX_MPDU_RES_STATUS_SEC_TKIP_ENC:
		/* Don't drop the frame and decrypt it in SW */
		if (!(rx_pkt_status & RX_MPDU_RES_STATUS_TTAK_OK))
			return 0;
		*crypt_len = IEEE80211_TKIP_IV_LEN;
		/* fall through if TTAK OK */

	case RX_MPDU_RES_STATUS_SEC_WEP_ENC:
		if (!(rx_pkt_status & RX_MPDU_RES_STATUS_ICV_OK))
			return -1;

		stats->flag |= RX_FLAG_DECRYPTED;
		if ((rx_pkt_status & RX_MPDU_RES_STATUS_SEC_ENC_MSK) ==
				RX_MPDU_RES_STATUS_SEC_WEP_ENC)
			*crypt_len = IEEE80211_WEP_IV_LEN;
		return 0;

	case RX_MPDU_RES_STATUS_SEC_EXT_ENC:
		if (!(rx_pkt_status & RX_MPDU_RES_STATUS_MIC_OK))
			return -1;
		stats->flag |= RX_FLAG_DECRYPTED;
		return 0;

	default:
		IWL_ERR(mvm, "Unhandled alg: 0x%x\n", rx_pkt_status);
	}

	return 0;
}

/*
 * iwl_mvm_rx_rx_mpdu - REPLY_RX_MPDU_CMD handler
 *
 * Handles the actual data of the Rx packet from the fw
 */
int iwl_mvm_rx_rx_mpdu(struct iwl_mvm *mvm, struct iwl_rx_cmd_buffer *rxb,
		       struct iwl_device_cmd *cmd)
{
	struct ieee80211_hdr *hdr;
	struct ieee80211_rx_status *rx_status;
	struct iwl_rx_packet *pkt = rxb_addr(rxb);
	struct iwl_rx_phy_info *phy_info;
	struct iwl_rx_mpdu_res_start *rx_res;
	struct ieee80211_sta *sta;
	struct sk_buff *skb;
	u32 len;
	u32 ampdu_status;
	u32 rate_n_flags;
	u32 rx_pkt_status;
	u8 crypt_len = 0;

	phy_info = &mvm->last_phy_info;
	rx_res = (struct iwl_rx_mpdu_res_start *)pkt->data;
	hdr = (struct ieee80211_hdr *)(pkt->data + sizeof(*rx_res));
	len = le16_to_cpu(rx_res->byte_count);
	rx_pkt_status = le32_to_cpup((__le32 *)
		(pkt->data + sizeof(*rx_res) + len));

	/* Dont use dev_alloc_skb(), we'll have enough headroom once
	 * ieee80211_hdr pulled.
	 */
	skb = alloc_skb(128, GFP_ATOMIC);
	if (!skb) {
		IWL_ERR(mvm, "alloc_skb failed\n");
		return 0;
	}

	rx_status = IEEE80211_SKB_RXCB(skb);

	/*
	 * drop the packet if it has failed being decrypted by HW
	 */
	if (iwl_mvm_set_mac80211_rx_flag(mvm, hdr, rx_status, rx_pkt_status,
					 &crypt_len)) {
		IWL_DEBUG_DROP(mvm, "Bad decryption results 0x%08x\n",
			       rx_pkt_status);
		kfree_skb(skb);
		return 0;
	}

	if ((unlikely(phy_info->cfg_phy_cnt > 20))) {
		IWL_DEBUG_DROP(mvm, "dsp size out of range [0,20]: %d\n",
			       phy_info->cfg_phy_cnt);
		kfree_skb(skb);
		return 0;
	}

	/*
	 * Keep packets with CRC errors (and with overrun) for monitor mode
	 * (otherwise the firmware discards them) but mark them as bad.
	 */
	if (!(rx_pkt_status & RX_MPDU_RES_STATUS_CRC_OK) ||
	    !(rx_pkt_status & RX_MPDU_RES_STATUS_OVERRUN_OK)) {
		IWL_DEBUG_RX(mvm, "Bad CRC or FIFO: 0x%08X.\n", rx_pkt_status);
		rx_status->flag |= RX_FLAG_FAILED_FCS_CRC;
	}

	/* This will be used in several places later */
	rate_n_flags = le32_to_cpu(phy_info->rate_n_flags);

	/* rx_status carries information about the packet to mac80211 */
	rx_status->mactime = le64_to_cpu(phy_info->timestamp);
	rx_status->device_timestamp = le32_to_cpu(phy_info->system_timestamp);
	rx_status->band =
		(phy_info->phy_flags & cpu_to_le16(RX_RES_PHY_FLAGS_BAND_24)) ?
				IEEE80211_BAND_2GHZ : IEEE80211_BAND_5GHZ;
	rx_status->freq =
		ieee80211_channel_to_frequency(le16_to_cpu(phy_info->channel),
					       rx_status->band);
	/*
	 * TSF as indicated by the fw is at INA time, but mac80211 expects the
	 * TSF at the beginning of the MPDU.
	 */
	/*rx_status->flag |= RX_FLAG_MACTIME_MPDU;*/

	iwl_mvm_get_signal_strength(mvm, phy_info, rx_status);

	IWL_DEBUG_STATS_LIMIT(mvm, "Rssi %d, TSF %llu\n", rx_status->signal,
			      (unsigned long long)rx_status->mactime);

	rcu_read_lock();
	/*
	 * We have tx blocked stations (with CS bit). If we heard frames from
	 * a blocked station on a new channel we can TX to it again.
	 */
	if (unlikely(mvm->csa_tx_block_bcn_timeout)) {
		sta = ieee80211_find_sta(
			rcu_dereference(mvm->csa_tx_blocked_vif), hdr->addr2);
		if (sta)
			iwl_mvm_sta_modify_disable_tx_ap(mvm, sta, false);
	}

	/* This is fine since we don't support multiple AP interfaces */
	sta = ieee80211_find_sta_by_ifaddr(mvm->hw, hdr->addr2, NULL);
	if (sta) {
		struct iwl_mvm_sta *mvmsta;
		mvmsta = iwl_mvm_sta_from_mac80211(sta);
		rs_update_last_rssi(mvm, &mvmsta->lq_sta, rx_status);

		if (iwl_fw_dbg_trigger_enabled(mvm->fw, FW_DBG_TRIGGER_RSSI) &&
		    ieee80211_is_beacon(hdr->frame_control)) {
			struct iwl_fw_dbg_trigger_tlv *trig;
			struct iwl_fw_dbg_trigger_low_rssi *rssi_trig;
			bool trig_check;
			s32 rssi;

			trig = iwl_fw_dbg_get_trigger(mvm->fw,
						      FW_DBG_TRIGGER_RSSI);
			rssi_trig = (void *)trig->data;
			rssi = le32_to_cpu(rssi_trig->rssi);

			trig_check =
				iwl_fw_dbg_trigger_check_stop(mvm, mvmsta->vif,
							      trig);
			if (trig_check && rx_status->signal < rssi)
				iwl_mvm_fw_dbg_collect_trig(mvm, trig, NULL);
		}
	}

	rcu_read_unlock();

	/* set the preamble flag if appropriate */
	if (phy_info->phy_flags & cpu_to_le16(RX_RES_PHY_FLAGS_SHORT_PREAMBLE))
		rx_status->flag |= RX_FLAG_SHORTPRE;

	if (phy_info->phy_flags & cpu_to_le16(RX_RES_PHY_FLAGS_AGG)) {
		/*
		 * We know which subframes of an A-MPDU belong
		 * together since we get a single PHY response
		 * from the firmware for all of them
		 */
		rx_status->flag |= RX_FLAG_AMPDU_DETAILS;
		rx_status->ampdu_reference = mvm->ampdu_ref;
	}

	/* Set up the HT phy flags */
	switch (rate_n_flags & RATE_MCS_CHAN_WIDTH_MSK) {
	case RATE_MCS_CHAN_WIDTH_20:
		break;
	case RATE_MCS_CHAN_WIDTH_40:
		rx_status->flag |= RX_FLAG_40MHZ;
		break;
	case RATE_MCS_CHAN_WIDTH_80:
		rx_status->vht_flag |= RX_VHT_FLAG_80MHZ;
		break;
	case RATE_MCS_CHAN_WIDTH_160:
		rx_status->vht_flag |= RX_VHT_FLAG_160MHZ;
		break;
	}
	if (rate_n_flags & RATE_MCS_SGI_MSK)
		rx_status->flag |= RX_FLAG_SHORT_GI;
	if (rate_n_flags & RATE_HT_MCS_GF_MSK)
		rx_status->flag |= RX_FLAG_HT_GF;
	if (rate_n_flags & RATE_MCS_LDPC_MSK)
		rx_status->flag |= RX_FLAG_LDPC;
	if (rate_n_flags & RATE_MCS_HT_MSK) {
		u8 stbc = (rate_n_flags & RATE_MCS_HT_STBC_MSK) >>
				RATE_MCS_STBC_POS;
		rx_status->flag |= RX_FLAG_HT;
		rx_status->rate_idx = rate_n_flags & RATE_HT_MCS_INDEX_MSK;
		rx_status->flag |= stbc << RX_FLAG_STBC_SHIFT;
	} else if (rate_n_flags & RATE_MCS_VHT_MSK) {
		u8 stbc = (rate_n_flags & RATE_MCS_VHT_STBC_MSK) >>
				RATE_MCS_STBC_POS;
		rx_status->vht_nss =
			((rate_n_flags & RATE_VHT_MCS_NSS_MSK) >>
						RATE_VHT_MCS_NSS_POS) + 1;
		rx_status->rate_idx = rate_n_flags & RATE_VHT_MCS_RATE_CODE_MSK;
		rx_status->flag |= RX_FLAG_VHT;
		rx_status->flag |= stbc << RX_FLAG_STBC_SHIFT;
		if (rate_n_flags & RATE_MCS_BF_MSK)
			rx_status->vht_flag |= RX_VHT_FLAG_BF;
	} else {
		rx_status->rate_idx =
			iwl_mvm_legacy_rate_to_mac80211_idx(rate_n_flags,
							    rx_status->band);
	}

#ifdef CONFIG_IWLWIFI_DEBUGFS
	iwl_mvm_update_frame_stats(mvm, rate_n_flags,
				   rx_status->flag & RX_FLAG_AMPDU_DETAILS);
#endif
	iwl_mvm_pass_packet_to_mac80211(mvm, skb, hdr, len, ampdu_status,
					crypt_len, rxb);
	return 0;
}

static void iwl_mvm_update_rx_statistics(struct iwl_mvm *mvm,
					 struct mvm_statistics_rx *rx_stats)
{
	lockdep_assert_held(&mvm->mutex);

	mvm->rx_stats = *rx_stats;
}

struct iwl_mvm_stat_data {
	struct iwl_mvm *mvm;
	__le32 mac_id;
	__s8 beacon_filter_average_energy;
	struct mvm_statistics_general_v8 *general;
};

static void iwl_mvm_stat_iterator(void *_data, u8 *mac,
				  struct ieee80211_vif *vif)
{
	struct iwl_mvm_stat_data *data = _data;
	struct iwl_mvm *mvm = data->mvm;
	int sig = -data->beacon_filter_average_energy;
	int last_event;
	int thold = vif->bss_conf.cqm_rssi_thold;
	int hyst = vif->bss_conf.cqm_rssi_hyst;
	u16 id = le32_to_cpu(data->mac_id);
	struct iwl_mvm_vif *mvmvif = iwl_mvm_vif_from_mac80211(vif);

	/* This doesn't need the MAC ID check since it's not taking the
	 * data copied into the "data" struct, but rather the data from
	 * the notification directly.
	 */
	if (data->general) {
		mvmvif->beacon_stats.num_beacons =
			le32_to_cpu(data->general->beacon_counter[mvmvif->id]);
		mvmvif->beacon_stats.avg_signal =
			-data->general->beacon_average_energy[mvmvif->id];
	}

	if (mvmvif->id != id)
		return;

	if (vif->type != NL80211_IFTYPE_STATION)
		return;

	if (sig == 0) {
		IWL_DEBUG_RX(mvm, "RSSI is 0 - skip signal based decision\n");
		return;
	}

	mvmvif->bf_data.ave_beacon_signal = sig;

	/* BT Coex */
	if (mvmvif->bf_data.bt_coex_min_thold !=
	    mvmvif->bf_data.bt_coex_max_thold) {
		last_event = mvmvif->bf_data.last_bt_coex_event;
		if (sig > mvmvif->bf_data.bt_coex_max_thold &&
		    (last_event <= mvmvif->bf_data.bt_coex_min_thold ||
		     last_event == 0)) {
			mvmvif->bf_data.last_bt_coex_event = sig;
			IWL_DEBUG_RX(mvm, "cqm_iterator bt coex high %d\n",
				     sig);
			iwl_mvm_bt_rssi_event(mvm, vif, RSSI_EVENT_HIGH);
		} else if (sig < mvmvif->bf_data.bt_coex_min_thold &&
			   (last_event >= mvmvif->bf_data.bt_coex_max_thold ||
			    last_event == 0)) {
			mvmvif->bf_data.last_bt_coex_event = sig;
			IWL_DEBUG_RX(mvm, "cqm_iterator bt coex low %d\n",
				     sig);
			iwl_mvm_bt_rssi_event(mvm, vif, RSSI_EVENT_LOW);
		}
	}

	if (!(vif->driver_flags & IEEE80211_VIF_SUPPORTS_CQM_RSSI))
		return;

	/* CQM Notification */
	last_event = mvmvif->bf_data.last_cqm_event;
	if (thold && sig < thold && (last_event == 0 ||
				     sig < last_event - hyst)) {
		mvmvif->bf_data.last_cqm_event = sig;
		IWL_DEBUG_RX(mvm, "cqm_iterator cqm low %d\n",
			     sig);
		ieee80211_cqm_rssi_notify(
			vif,
			NL80211_CQM_RSSI_THRESHOLD_EVENT_LOW,
			GFP_KERNEL);
	} else if (sig > thold &&
		   (last_event == 0 || sig > last_event + hyst)) {
		mvmvif->bf_data.last_cqm_event = sig;
		IWL_DEBUG_RX(mvm, "cqm_iterator cqm high %d\n",
			     sig);
		ieee80211_cqm_rssi_notify(
			vif,
			NL80211_CQM_RSSI_THRESHOLD_EVENT_HIGH,
			GFP_KERNEL);
	}
}

static inline void
iwl_mvm_rx_stats_check_trigger(struct iwl_mvm *mvm, struct iwl_rx_packet *pkt)
{
	struct iwl_fw_dbg_trigger_tlv *trig;
	struct iwl_fw_dbg_trigger_stats *trig_stats;
	u32 trig_offset, trig_thold;

	if (!iwl_fw_dbg_trigger_enabled(mvm->fw, FW_DBG_TRIGGER_STATS))
		return;

	trig = iwl_fw_dbg_get_trigger(mvm->fw, FW_DBG_TRIGGER_STATS);
	trig_stats = (void *)trig->data;

	if (!iwl_fw_dbg_trigger_check_stop(mvm, NULL, trig))
		return;

	trig_offset = le32_to_cpu(trig_stats->stop_offset);
	trig_thold = le32_to_cpu(trig_stats->stop_threshold);

	if (WARN_ON_ONCE(trig_offset >= iwl_rx_packet_payload_len(pkt)))
		return;

	if (le32_to_cpup((__le32 *) (pkt->data + trig_offset)) < trig_thold)
		return;

	iwl_mvm_fw_dbg_collect_trig(mvm, trig, NULL);
}

void iwl_mvm_handle_rx_statistics(struct iwl_mvm *mvm,
				  struct iwl_rx_packet *pkt)
{
	size_t v8_len = sizeof(struct iwl_notif_statistics_v8);
	size_t v10_len = sizeof(struct iwl_notif_statistics_v10);
	struct iwl_mvm_stat_data data = {
		.mvm = mvm,
	};
	u32 temperature;

	if (mvm->fw->ucode_capa.api[0] & IWL_UCODE_TLV_API_STATS_V10) {
		struct iwl_notif_statistics_v10 *stats = (void *)&pkt->data;

		if (iwl_rx_packet_payload_len(pkt) != v10_len)
			goto invalid;

		temperature = le32_to_cpu(stats->general.radio_temperature);
		data.mac_id = stats->rx.general.mac_id;
		data.beacon_filter_average_energy =
			stats->general.beacon_filter_average_energy;

		iwl_mvm_update_rx_statistics(mvm, &stats->rx);

		mvm->radio_stats.rx_time = le64_to_cpu(stats->general.rx_time);
		mvm->radio_stats.tx_time = le64_to_cpu(stats->general.tx_time);
		mvm->radio_stats.on_time_rf =
			le64_to_cpu(stats->general.on_time_rf);
		mvm->radio_stats.on_time_scan =
			le64_to_cpu(stats->general.on_time_scan);

		data.general = &stats->general;
	} else {
		struct iwl_notif_statistics_v8 *stats = (void *)&pkt->data;

		if (iwl_rx_packet_payload_len(pkt) != v8_len)
			goto invalid;

		temperature = le32_to_cpu(stats->general.radio_temperature);
		data.mac_id = stats->rx.general.mac_id;
		data.beacon_filter_average_energy =
			stats->general.beacon_filter_average_energy;

		iwl_mvm_update_rx_statistics(mvm, &stats->rx);
	}

	iwl_mvm_rx_stats_check_trigger(mvm, pkt);

	/* Only handle rx statistics temperature changes if async temp
	 * notifications are not supported
	 */
	if (!(mvm->fw->ucode_capa.api[0] & IWL_UCODE_TLV_API_ASYNC_DTM))
		iwl_mvm_tt_temp_changed(mvm, temperature);

	ieee80211_iterate_active_interfaces(mvm->hw,
					    IEEE80211_IFACE_ITER_NORMAL,
					    iwl_mvm_stat_iterator,
					    &data);
	return;
 invalid:
	IWL_ERR(mvm, "received invalid statistics size (%d)!\n",
		iwl_rx_packet_payload_len(pkt));
}

int iwl_mvm_rx_statistics(struct iwl_mvm *mvm,
			  struct iwl_rx_cmd_buffer *rxb,
			  struct iwl_device_cmd *cmd)
{
	iwl_mvm_handle_rx_statistics(mvm, rxb_addr(rxb));
	return 0;
}
