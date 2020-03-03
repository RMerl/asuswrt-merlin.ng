/*
 * VHT handling
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <linux/ieee80211.h>
#include <linux/export.h>
#include <net/mac80211.h>
#include "ieee80211_i.h"
#include "rate.h"


static void __check_vhtcap_disable(struct ieee80211_sub_if_data *sdata,
				   struct ieee80211_sta_vht_cap *vht_cap,
				   u32 flag)
{
	__le32 le_flag = cpu_to_le32(flag);

	if (sdata->u.mgd.vht_capa_mask.vht_cap_info & le_flag &&
	    !(sdata->u.mgd.vht_capa.vht_cap_info & le_flag))
		vht_cap->cap &= ~flag;
}

void ieee80211_apply_vhtcap_overrides(struct ieee80211_sub_if_data *sdata,
				      struct ieee80211_sta_vht_cap *vht_cap)
{
	int i;
	u16 rxmcs_mask, rxmcs_cap, rxmcs_n, txmcs_mask, txmcs_cap, txmcs_n;

	if (!vht_cap->vht_supported)
		return;

	if (sdata->vif.type != NL80211_IFTYPE_STATION)
		return;

	__check_vhtcap_disable(sdata, vht_cap,
			       IEEE80211_VHT_CAP_RXLDPC);
	__check_vhtcap_disable(sdata, vht_cap,
			       IEEE80211_VHT_CAP_SHORT_GI_80);
	__check_vhtcap_disable(sdata, vht_cap,
			       IEEE80211_VHT_CAP_SHORT_GI_160);
	__check_vhtcap_disable(sdata, vht_cap,
			       IEEE80211_VHT_CAP_TXSTBC);
	__check_vhtcap_disable(sdata, vht_cap,
			       IEEE80211_VHT_CAP_SU_BEAMFORMER_CAPABLE);
	__check_vhtcap_disable(sdata, vht_cap,
			       IEEE80211_VHT_CAP_SU_BEAMFORMEE_CAPABLE);
	__check_vhtcap_disable(sdata, vht_cap,
			       IEEE80211_VHT_CAP_RX_ANTENNA_PATTERN);
	__check_vhtcap_disable(sdata, vht_cap,
			       IEEE80211_VHT_CAP_TX_ANTENNA_PATTERN);

	/* Allow user to decrease AMPDU length exponent */
	if (sdata->u.mgd.vht_capa_mask.vht_cap_info &
	    cpu_to_le32(IEEE80211_VHT_CAP_MAX_A_MPDU_LENGTH_EXPONENT_MASK)) {
		u32 cap, n;

		n = le32_to_cpu(sdata->u.mgd.vht_capa.vht_cap_info) &
			IEEE80211_VHT_CAP_MAX_A_MPDU_LENGTH_EXPONENT_MASK;
		n >>= IEEE80211_VHT_CAP_MAX_A_MPDU_LENGTH_EXPONENT_SHIFT;
		cap = vht_cap->cap & IEEE80211_VHT_CAP_MAX_A_MPDU_LENGTH_EXPONENT_MASK;
		cap >>= IEEE80211_VHT_CAP_MAX_A_MPDU_LENGTH_EXPONENT_SHIFT;

		if (n < cap) {
			vht_cap->cap &=
				~IEEE80211_VHT_CAP_MAX_A_MPDU_LENGTH_EXPONENT_MASK;
			vht_cap->cap |=
				n << IEEE80211_VHT_CAP_MAX_A_MPDU_LENGTH_EXPONENT_SHIFT;
		}
	}

	/* Allow the user to decrease MCSes */
	rxmcs_mask =
		le16_to_cpu(sdata->u.mgd.vht_capa_mask.supp_mcs.rx_mcs_map);
	rxmcs_n = le16_to_cpu(sdata->u.mgd.vht_capa.supp_mcs.rx_mcs_map);
	rxmcs_n &= rxmcs_mask;
	rxmcs_cap = le16_to_cpu(vht_cap->vht_mcs.rx_mcs_map);

	txmcs_mask =
		le16_to_cpu(sdata->u.mgd.vht_capa_mask.supp_mcs.tx_mcs_map);
	txmcs_n = le16_to_cpu(sdata->u.mgd.vht_capa.supp_mcs.tx_mcs_map);
	txmcs_n &= txmcs_mask;
	txmcs_cap = le16_to_cpu(vht_cap->vht_mcs.tx_mcs_map);
	for (i = 0; i < 8; i++) {
		u8 m, n, c;

		m = (rxmcs_mask >> 2*i) & IEEE80211_VHT_MCS_NOT_SUPPORTED;
		n = (rxmcs_n >> 2*i) & IEEE80211_VHT_MCS_NOT_SUPPORTED;
		c = (rxmcs_cap >> 2*i) & IEEE80211_VHT_MCS_NOT_SUPPORTED;

		if (m && ((c != IEEE80211_VHT_MCS_NOT_SUPPORTED && n < c) ||
			  n == IEEE80211_VHT_MCS_NOT_SUPPORTED)) {
			rxmcs_cap &= ~(3 << 2*i);
			rxmcs_cap |= (rxmcs_n & (3 << 2*i));
		}

		m = (txmcs_mask >> 2*i) & IEEE80211_VHT_MCS_NOT_SUPPORTED;
		n = (txmcs_n >> 2*i) & IEEE80211_VHT_MCS_NOT_SUPPORTED;
		c = (txmcs_cap >> 2*i) & IEEE80211_VHT_MCS_NOT_SUPPORTED;

		if (m && ((c != IEEE80211_VHT_MCS_NOT_SUPPORTED && n < c) ||
			  n == IEEE80211_VHT_MCS_NOT_SUPPORTED)) {
			txmcs_cap &= ~(3 << 2*i);
			txmcs_cap |= (txmcs_n & (3 << 2*i));
		}
	}
	vht_cap->vht_mcs.rx_mcs_map = cpu_to_le16(rxmcs_cap);
	vht_cap->vht_mcs.tx_mcs_map = cpu_to_le16(txmcs_cap);
}

void
ieee80211_vht_cap_ie_to_sta_vht_cap(struct ieee80211_sub_if_data *sdata,
				    struct ieee80211_supported_band *sband,
				    const struct ieee80211_vht_cap *vht_cap_ie,
				    struct sta_info *sta)
{
	struct ieee80211_sta_vht_cap *vht_cap = &sta->sta.vht_cap;
	struct ieee80211_sta_vht_cap own_cap;
	u32 cap_info, i;

	memset(vht_cap, 0, sizeof(*vht_cap));

	if (!sta->sta.ht_cap.ht_supported)
		return;

	if (!vht_cap_ie || !sband->vht_cap.vht_supported)
		return;

	/*
	 * A VHT STA must support 40 MHz, but if we verify that here
	 * then we break a few things - some APs (e.g. Netgear R6300v2
	 * and others based on the BCM4360 chipset) will unset this
	 * capability bit when operating in 20 MHz.
	 */

	vht_cap->vht_supported = true;

	own_cap = sband->vht_cap;
	/*
	 * If user has specified capability overrides, take care
	 * of that if the station we're setting up is the AP that
	 * we advertised a restricted capability set to. Override
	 * our own capabilities and then use those below.
	 */
	if (sdata->vif.type == NL80211_IFTYPE_STATION &&
	    !test_sta_flag(sta, WLAN_STA_TDLS_PEER))
		ieee80211_apply_vhtcap_overrides(sdata, &own_cap);

	/* take some capabilities as-is */
	cap_info = le32_to_cpu(vht_cap_ie->vht_cap_info);
	vht_cap->cap = cap_info;
	vht_cap->cap &= IEEE80211_VHT_CAP_MAX_MPDU_LENGTH_3895 |
			IEEE80211_VHT_CAP_MAX_MPDU_LENGTH_7991 |
			IEEE80211_VHT_CAP_MAX_MPDU_LENGTH_11454 |
			IEEE80211_VHT_CAP_RXLDPC |
			IEEE80211_VHT_CAP_VHT_TXOP_PS |
			IEEE80211_VHT_CAP_HTC_VHT |
			IEEE80211_VHT_CAP_MAX_A_MPDU_LENGTH_EXPONENT_MASK |
			IEEE80211_VHT_CAP_VHT_LINK_ADAPTATION_VHT_UNSOL_MFB |
			IEEE80211_VHT_CAP_VHT_LINK_ADAPTATION_VHT_MRQ_MFB |
			IEEE80211_VHT_CAP_RX_ANTENNA_PATTERN |
			IEEE80211_VHT_CAP_TX_ANTENNA_PATTERN;

	/* and some based on our own capabilities */
	switch (own_cap.cap & IEEE80211_VHT_CAP_SUPP_CHAN_WIDTH_MASK) {
	case IEEE80211_VHT_CAP_SUPP_CHAN_WIDTH_160MHZ:
		vht_cap->cap |= cap_info &
				IEEE80211_VHT_CAP_SUPP_CHAN_WIDTH_160MHZ;
		break;
	case IEEE80211_VHT_CAP_SUPP_CHAN_WIDTH_160_80PLUS80MHZ:
		vht_cap->cap |= cap_info &
				IEEE80211_VHT_CAP_SUPP_CHAN_WIDTH_MASK;
		break;
	default:
		/* nothing */
		break;
	}

	/* symmetric capabilities */
	vht_cap->cap |= cap_info & own_cap.cap &
			(IEEE80211_VHT_CAP_SHORT_GI_80 |
			 IEEE80211_VHT_CAP_SHORT_GI_160);

	/* remaining ones */
	if (own_cap.cap & IEEE80211_VHT_CAP_SU_BEAMFORMEE_CAPABLE)
		vht_cap->cap |= cap_info &
				(IEEE80211_VHT_CAP_SU_BEAMFORMER_CAPABLE |
				 IEEE80211_VHT_CAP_SOUNDING_DIMENSIONS_MASK);

	if (own_cap.cap & IEEE80211_VHT_CAP_SU_BEAMFORMER_CAPABLE)
		vht_cap->cap |= cap_info &
				(IEEE80211_VHT_CAP_SU_BEAMFORMEE_CAPABLE |
				 IEEE80211_VHT_CAP_BEAMFORMEE_STS_MASK);

	if (own_cap.cap & IEEE80211_VHT_CAP_MU_BEAMFORMER_CAPABLE)
		vht_cap->cap |= cap_info &
				IEEE80211_VHT_CAP_MU_BEAMFORMEE_CAPABLE;

	if (own_cap.cap & IEEE80211_VHT_CAP_MU_BEAMFORMEE_CAPABLE)
		vht_cap->cap |= cap_info &
				IEEE80211_VHT_CAP_MU_BEAMFORMER_CAPABLE;

	if (own_cap.cap & IEEE80211_VHT_CAP_TXSTBC)
		vht_cap->cap |= cap_info & IEEE80211_VHT_CAP_RXSTBC_MASK;

	if (own_cap.cap & IEEE80211_VHT_CAP_RXSTBC_MASK)
		vht_cap->cap |= cap_info & IEEE80211_VHT_CAP_TXSTBC;

	/* Copy peer MCS info, the driver might need them. */
	memcpy(&vht_cap->vht_mcs, &vht_cap_ie->supp_mcs,
	       sizeof(struct ieee80211_vht_mcs_info));

	/* but also restrict MCSes */
	for (i = 0; i < 8; i++) {
		u16 own_rx, own_tx, peer_rx, peer_tx;

		own_rx = le16_to_cpu(own_cap.vht_mcs.rx_mcs_map);
		own_rx = (own_rx >> i * 2) & IEEE80211_VHT_MCS_NOT_SUPPORTED;

		own_tx = le16_to_cpu(own_cap.vht_mcs.tx_mcs_map);
		own_tx = (own_tx >> i * 2) & IEEE80211_VHT_MCS_NOT_SUPPORTED;

		peer_rx = le16_to_cpu(vht_cap->vht_mcs.rx_mcs_map);
		peer_rx = (peer_rx >> i * 2) & IEEE80211_VHT_MCS_NOT_SUPPORTED;

		peer_tx = le16_to_cpu(vht_cap->vht_mcs.tx_mcs_map);
		peer_tx = (peer_tx >> i * 2) & IEEE80211_VHT_MCS_NOT_SUPPORTED;

		if (peer_tx != IEEE80211_VHT_MCS_NOT_SUPPORTED) {
			if (own_rx == IEEE80211_VHT_MCS_NOT_SUPPORTED)
				peer_tx = IEEE80211_VHT_MCS_NOT_SUPPORTED;
			else if (own_rx < peer_tx)
				peer_tx = own_rx;
		}

		if (peer_rx != IEEE80211_VHT_MCS_NOT_SUPPORTED) {
			if (own_tx == IEEE80211_VHT_MCS_NOT_SUPPORTED)
				peer_rx = IEEE80211_VHT_MCS_NOT_SUPPORTED;
			else if (own_tx < peer_rx)
				peer_rx = own_tx;
		}

		vht_cap->vht_mcs.rx_mcs_map &=
			~cpu_to_le16(IEEE80211_VHT_MCS_NOT_SUPPORTED << i * 2);
		vht_cap->vht_mcs.rx_mcs_map |= cpu_to_le16(peer_rx << i * 2);

		vht_cap->vht_mcs.tx_mcs_map &=
			~cpu_to_le16(IEEE80211_VHT_MCS_NOT_SUPPORTED << i * 2);
		vht_cap->vht_mcs.tx_mcs_map |= cpu_to_le16(peer_tx << i * 2);
	}

	/* finally set up the bandwidth */
	switch (vht_cap->cap & IEEE80211_VHT_CAP_SUPP_CHAN_WIDTH_MASK) {
	case IEEE80211_VHT_CAP_SUPP_CHAN_WIDTH_160MHZ:
	case IEEE80211_VHT_CAP_SUPP_CHAN_WIDTH_160_80PLUS80MHZ:
		sta->cur_max_bandwidth = IEEE80211_STA_RX_BW_160;
		break;
	default:
		sta->cur_max_bandwidth = IEEE80211_STA_RX_BW_80;
	}

	sta->sta.bandwidth = ieee80211_sta_cur_vht_bw(sta);
}

enum ieee80211_sta_rx_bandwidth ieee80211_sta_cap_rx_bw(struct sta_info *sta)
{
	struct ieee80211_sta_vht_cap *vht_cap = &sta->sta.vht_cap;
	u32 cap_width;

	if (!vht_cap->vht_supported)
		return sta->sta.ht_cap.cap & IEEE80211_HT_CAP_SUP_WIDTH_20_40 ?
				IEEE80211_STA_RX_BW_40 :
				IEEE80211_STA_RX_BW_20;

	cap_width = vht_cap->cap & IEEE80211_VHT_CAP_SUPP_CHAN_WIDTH_MASK;

	if (cap_width == IEEE80211_VHT_CAP_SUPP_CHAN_WIDTH_160MHZ ||
	    cap_width == IEEE80211_VHT_CAP_SUPP_CHAN_WIDTH_160_80PLUS80MHZ)
		return IEEE80211_STA_RX_BW_160;

	return IEEE80211_STA_RX_BW_80;
}

static enum ieee80211_sta_rx_bandwidth
ieee80211_chan_width_to_rx_bw(enum nl80211_chan_width width)
{
	switch (width) {
	case NL80211_CHAN_WIDTH_20_NOHT:
	case NL80211_CHAN_WIDTH_20:
		return IEEE80211_STA_RX_BW_20;
	case NL80211_CHAN_WIDTH_40:
		return IEEE80211_STA_RX_BW_40;
	case NL80211_CHAN_WIDTH_80:
		return IEEE80211_STA_RX_BW_80;
	case NL80211_CHAN_WIDTH_160:
	case NL80211_CHAN_WIDTH_80P80:
		return IEEE80211_STA_RX_BW_160;
	default:
		WARN_ON_ONCE(1);
		return IEEE80211_STA_RX_BW_20;
	}
}

enum ieee80211_sta_rx_bandwidth ieee80211_sta_cur_vht_bw(struct sta_info *sta)
{
	struct ieee80211_sub_if_data *sdata = sta->sdata;
	enum ieee80211_sta_rx_bandwidth bw;

	bw = ieee80211_chan_width_to_rx_bw(sdata->vif.bss_conf.chandef.width);
	bw = min(bw, ieee80211_sta_cap_rx_bw(sta));
	bw = min(bw, sta->cur_max_bandwidth);

	return bw;
}

void ieee80211_sta_set_rx_nss(struct sta_info *sta)
{
	u8 ht_rx_nss = 0, vht_rx_nss = 0;

	/* if we received a notification already don't overwrite it */
	if (sta->sta.rx_nss)
		return;

	if (sta->sta.ht_cap.ht_supported) {
		if (sta->sta.ht_cap.mcs.rx_mask[0])
			ht_rx_nss++;
		if (sta->sta.ht_cap.mcs.rx_mask[1])
			ht_rx_nss++;
		if (sta->sta.ht_cap.mcs.rx_mask[2])
			ht_rx_nss++;
		if (sta->sta.ht_cap.mcs.rx_mask[3])
			ht_rx_nss++;
		/* FIXME: consider rx_highest? */
	}

	if (sta->sta.vht_cap.vht_supported) {
		int i;
		u16 rx_mcs_map;

		rx_mcs_map = le16_to_cpu(sta->sta.vht_cap.vht_mcs.rx_mcs_map);

		for (i = 7; i >= 0; i--) {
			u8 mcs = (rx_mcs_map >> (2 * i)) & 3;

			if (mcs != IEEE80211_VHT_MCS_NOT_SUPPORTED) {
				vht_rx_nss = i + 1;
				break;
			}
		}
		/* FIXME: consider rx_highest? */
	}

	ht_rx_nss = max(ht_rx_nss, vht_rx_nss);
	sta->sta.rx_nss = max_t(u8, 1, ht_rx_nss);
}

u32 __ieee80211_vht_handle_opmode(struct ieee80211_sub_if_data *sdata,
				  struct sta_info *sta, u8 opmode,
				  enum ieee80211_band band, bool nss_only)
{
	struct ieee80211_local *local = sdata->local;
	struct ieee80211_supported_band *sband;
	enum ieee80211_sta_rx_bandwidth new_bw;
	u32 changed = 0;
	u8 nss;

	sband = local->hw.wiphy->bands[band];

	/* ignore - no support for BF yet */
	if (opmode & IEEE80211_OPMODE_NOTIF_RX_NSS_TYPE_BF)
		return 0;

	nss = opmode & IEEE80211_OPMODE_NOTIF_RX_NSS_MASK;
	nss >>= IEEE80211_OPMODE_NOTIF_RX_NSS_SHIFT;
	nss += 1;

	if (sta->sta.rx_nss != nss) {
		sta->sta.rx_nss = nss;
		changed |= IEEE80211_RC_NSS_CHANGED;
	}

	if (nss_only)
		return changed;

	switch (opmode & IEEE80211_OPMODE_NOTIF_CHANWIDTH_MASK) {
	case IEEE80211_OPMODE_NOTIF_CHANWIDTH_20MHZ:
		sta->cur_max_bandwidth = IEEE80211_STA_RX_BW_20;
		break;
	case IEEE80211_OPMODE_NOTIF_CHANWIDTH_40MHZ:
		sta->cur_max_bandwidth = IEEE80211_STA_RX_BW_40;
		break;
	case IEEE80211_OPMODE_NOTIF_CHANWIDTH_80MHZ:
		sta->cur_max_bandwidth = IEEE80211_STA_RX_BW_80;
		break;
	case IEEE80211_OPMODE_NOTIF_CHANWIDTH_160MHZ:
		sta->cur_max_bandwidth = IEEE80211_STA_RX_BW_160;
		break;
	}

	new_bw = ieee80211_sta_cur_vht_bw(sta);
	if (new_bw != sta->sta.bandwidth) {
		sta->sta.bandwidth = new_bw;
		changed |= IEEE80211_RC_BW_CHANGED;
	}

	return changed;
}

void ieee80211_vht_handle_opmode(struct ieee80211_sub_if_data *sdata,
				 struct sta_info *sta, u8 opmode,
				 enum ieee80211_band band, bool nss_only)
{
	struct ieee80211_local *local = sdata->local;
	struct ieee80211_supported_band *sband = local->hw.wiphy->bands[band];

	u32 changed = __ieee80211_vht_handle_opmode(sdata, sta, opmode,
						    band, nss_only);

	if (changed > 0)
		rate_control_rate_update(local, sband, sta, changed);
}
