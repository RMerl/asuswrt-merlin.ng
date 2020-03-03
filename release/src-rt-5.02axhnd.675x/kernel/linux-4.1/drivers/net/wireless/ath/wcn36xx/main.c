/*
 * Copyright (c) 2013 Eugene Krasnikov <k.eugene.e@gmail.com>
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
 */

#define pr_fmt(fmt) KBUILD_MODNAME ": " fmt

#include <linux/module.h>
#include <linux/firmware.h>
#include <linux/platform_device.h>
#include "wcn36xx.h"

unsigned int wcn36xx_dbg_mask;
module_param_named(debug_mask, wcn36xx_dbg_mask, uint, 0644);
MODULE_PARM_DESC(debug_mask, "Debugging mask");

#define CHAN2G(_freq, _idx) { \
	.band = IEEE80211_BAND_2GHZ, \
	.center_freq = (_freq), \
	.hw_value = (_idx), \
	.max_power = 25, \
}

#define CHAN5G(_freq, _idx) { \
	.band = IEEE80211_BAND_5GHZ, \
	.center_freq = (_freq), \
	.hw_value = (_idx), \
	.max_power = 25, \
}

/* The wcn firmware expects channel values to matching
 * their mnemonic values. So use these for .hw_value. */
static struct ieee80211_channel wcn_2ghz_channels[] = {
	CHAN2G(2412, 1), /* Channel 1 */
	CHAN2G(2417, 2), /* Channel 2 */
	CHAN2G(2422, 3), /* Channel 3 */
	CHAN2G(2427, 4), /* Channel 4 */
	CHAN2G(2432, 5), /* Channel 5 */
	CHAN2G(2437, 6), /* Channel 6 */
	CHAN2G(2442, 7), /* Channel 7 */
	CHAN2G(2447, 8), /* Channel 8 */
	CHAN2G(2452, 9), /* Channel 9 */
	CHAN2G(2457, 10), /* Channel 10 */
	CHAN2G(2462, 11), /* Channel 11 */
	CHAN2G(2467, 12), /* Channel 12 */
	CHAN2G(2472, 13), /* Channel 13 */
	CHAN2G(2484, 14)  /* Channel 14 */

};

static struct ieee80211_channel wcn_5ghz_channels[] = {
	CHAN5G(5180, 36),
	CHAN5G(5200, 40),
	CHAN5G(5220, 44),
	CHAN5G(5240, 48),
	CHAN5G(5260, 52),
	CHAN5G(5280, 56),
	CHAN5G(5300, 60),
	CHAN5G(5320, 64),
	CHAN5G(5500, 100),
	CHAN5G(5520, 104),
	CHAN5G(5540, 108),
	CHAN5G(5560, 112),
	CHAN5G(5580, 116),
	CHAN5G(5600, 120),
	CHAN5G(5620, 124),
	CHAN5G(5640, 128),
	CHAN5G(5660, 132),
	CHAN5G(5700, 140),
	CHAN5G(5745, 149),
	CHAN5G(5765, 153),
	CHAN5G(5785, 157),
	CHAN5G(5805, 161),
	CHAN5G(5825, 165)
};

#define RATE(_bitrate, _hw_rate, _flags) { \
	.bitrate        = (_bitrate),                   \
	.flags          = (_flags),                     \
	.hw_value       = (_hw_rate),                   \
	.hw_value_short = (_hw_rate)  \
}

static struct ieee80211_rate wcn_2ghz_rates[] = {
	RATE(10, HW_RATE_INDEX_1MBPS, 0),
	RATE(20, HW_RATE_INDEX_2MBPS, IEEE80211_RATE_SHORT_PREAMBLE),
	RATE(55, HW_RATE_INDEX_5_5MBPS, IEEE80211_RATE_SHORT_PREAMBLE),
	RATE(110, HW_RATE_INDEX_11MBPS, IEEE80211_RATE_SHORT_PREAMBLE),
	RATE(60, HW_RATE_INDEX_6MBPS, 0),
	RATE(90, HW_RATE_INDEX_9MBPS, 0),
	RATE(120, HW_RATE_INDEX_12MBPS, 0),
	RATE(180, HW_RATE_INDEX_18MBPS, 0),
	RATE(240, HW_RATE_INDEX_24MBPS, 0),
	RATE(360, HW_RATE_INDEX_36MBPS, 0),
	RATE(480, HW_RATE_INDEX_48MBPS, 0),
	RATE(540, HW_RATE_INDEX_54MBPS, 0)
};

static struct ieee80211_rate wcn_5ghz_rates[] = {
	RATE(60, HW_RATE_INDEX_6MBPS, 0),
	RATE(90, HW_RATE_INDEX_9MBPS, 0),
	RATE(120, HW_RATE_INDEX_12MBPS, 0),
	RATE(180, HW_RATE_INDEX_18MBPS, 0),
	RATE(240, HW_RATE_INDEX_24MBPS, 0),
	RATE(360, HW_RATE_INDEX_36MBPS, 0),
	RATE(480, HW_RATE_INDEX_48MBPS, 0),
	RATE(540, HW_RATE_INDEX_54MBPS, 0)
};

static struct ieee80211_supported_band wcn_band_2ghz = {
	.channels	= wcn_2ghz_channels,
	.n_channels	= ARRAY_SIZE(wcn_2ghz_channels),
	.bitrates	= wcn_2ghz_rates,
	.n_bitrates	= ARRAY_SIZE(wcn_2ghz_rates),
	.ht_cap		= {
		.cap =	IEEE80211_HT_CAP_GRN_FLD |
			IEEE80211_HT_CAP_SGI_20 |
			IEEE80211_HT_CAP_DSSSCCK40 |
			IEEE80211_HT_CAP_LSIG_TXOP_PROT,
		.ht_supported = true,
		.ampdu_factor = IEEE80211_HT_MAX_AMPDU_64K,
		.ampdu_density = IEEE80211_HT_MPDU_DENSITY_16,
		.mcs = {
			.rx_mask = { 0xff, 0, 0, 0, 0, 0, 0, 0, 0, 0, },
			.rx_highest = cpu_to_le16(72),
			.tx_params = IEEE80211_HT_MCS_TX_DEFINED,
		}
	}
};

static struct ieee80211_supported_band wcn_band_5ghz = {
	.channels	= wcn_5ghz_channels,
	.n_channels	= ARRAY_SIZE(wcn_5ghz_channels),
	.bitrates	= wcn_5ghz_rates,
	.n_bitrates	= ARRAY_SIZE(wcn_5ghz_rates),
	.ht_cap		= {
		.cap =	IEEE80211_HT_CAP_GRN_FLD |
			IEEE80211_HT_CAP_SGI_20 |
			IEEE80211_HT_CAP_DSSSCCK40 |
			IEEE80211_HT_CAP_LSIG_TXOP_PROT |
			IEEE80211_HT_CAP_SGI_40 |
			IEEE80211_HT_CAP_SUP_WIDTH_20_40,
		.ht_supported = true,
		.ampdu_factor = IEEE80211_HT_MAX_AMPDU_64K,
		.ampdu_density = IEEE80211_HT_MPDU_DENSITY_16,
		.mcs = {
			.rx_mask = { 0xff, 0, 0, 0, 0, 0, 0, 0, 0, 0, },
			.rx_highest = cpu_to_le16(72),
			.tx_params = IEEE80211_HT_MCS_TX_DEFINED,
		}
	}
};

#ifdef CONFIG_PM

static const struct wiphy_wowlan_support wowlan_support = {
	.flags = WIPHY_WOWLAN_ANY
};

#endif

static inline u8 get_sta_index(struct ieee80211_vif *vif,
			       struct wcn36xx_sta *sta_priv)
{
	return NL80211_IFTYPE_STATION == vif->type ?
	       sta_priv->bss_sta_index :
	       sta_priv->sta_index;
}

static const char * const wcn36xx_caps_names[] = {
	"MCC",				/* 0 */
	"P2P",				/* 1 */
	"DOT11AC",			/* 2 */
	"SLM_SESSIONIZATION",		/* 3 */
	"DOT11AC_OPMODE",		/* 4 */
	"SAP32STA",			/* 5 */
	"TDLS",				/* 6 */
	"P2P_GO_NOA_DECOUPLE_INIT_SCAN",/* 7 */
	"WLANACTIVE_OFFLOAD",		/* 8 */
	"BEACON_OFFLOAD",		/* 9 */
	"SCAN_OFFLOAD",			/* 10 */
	"ROAM_OFFLOAD",			/* 11 */
	"BCN_MISS_OFFLOAD",		/* 12 */
	"STA_POWERSAVE",		/* 13 */
	"STA_ADVANCED_PWRSAVE",		/* 14 */
	"AP_UAPSD",			/* 15 */
	"AP_DFS",			/* 16 */
	"BLOCKACK",			/* 17 */
	"PHY_ERR",			/* 18 */
	"BCN_FILTER",			/* 19 */
	"RTT",				/* 20 */
	"RATECTRL",			/* 21 */
	"WOW"				/* 22 */
};

static const char *wcn36xx_get_cap_name(enum place_holder_in_cap_bitmap x)
{
	if (x >= ARRAY_SIZE(wcn36xx_caps_names))
		return "UNKNOWN";
	return wcn36xx_caps_names[x];
}

static void wcn36xx_feat_caps_info(struct wcn36xx *wcn)
{
	int i;

	for (i = 0; i < MAX_FEATURE_SUPPORTED; i++) {
		if (get_feat_caps(wcn->fw_feat_caps, i))
			wcn36xx_info("FW Cap %s\n", wcn36xx_get_cap_name(i));
	}
}

static void wcn36xx_detect_chip_version(struct wcn36xx *wcn)
{
	if (get_feat_caps(wcn->fw_feat_caps, DOT11AC)) {
		wcn36xx_info("Chip is 3680\n");
		wcn->chip_version = WCN36XX_CHIP_3680;
	} else {
		wcn36xx_info("Chip is 3660\n");
		wcn->chip_version = WCN36XX_CHIP_3660;
	}
}

static int wcn36xx_start(struct ieee80211_hw *hw)
{
	struct wcn36xx *wcn = hw->priv;
	int ret;

	wcn36xx_dbg(WCN36XX_DBG_MAC, "mac start\n");

	/* SMD initialization */
	ret = wcn36xx_smd_open(wcn);
	if (ret) {
		wcn36xx_err("Failed to open smd channel: %d\n", ret);
		goto out_err;
	}

	/* Allocate memory pools for Mgmt BD headers and Data BD headers */
	ret = wcn36xx_dxe_allocate_mem_pools(wcn);
	if (ret) {
		wcn36xx_err("Failed to alloc DXE mempool: %d\n", ret);
		goto out_smd_close;
	}

	ret = wcn36xx_dxe_alloc_ctl_blks(wcn);
	if (ret) {
		wcn36xx_err("Failed to alloc DXE ctl blocks: %d\n", ret);
		goto out_free_dxe_pool;
	}

	wcn->hal_buf = kmalloc(WCN36XX_HAL_BUF_SIZE, GFP_KERNEL);
	if (!wcn->hal_buf) {
		wcn36xx_err("Failed to allocate smd buf\n");
		ret = -ENOMEM;
		goto out_free_dxe_ctl;
	}

	ret = wcn36xx_smd_load_nv(wcn);
	if (ret) {
		wcn36xx_err("Failed to push NV to chip\n");
		goto out_free_smd_buf;
	}

	ret = wcn36xx_smd_start(wcn);
	if (ret) {
		wcn36xx_err("Failed to start chip\n");
		goto out_free_smd_buf;
	}

	if (!wcn36xx_is_fw_version(wcn, 1, 2, 2, 24)) {
		ret = wcn36xx_smd_feature_caps_exchange(wcn);
		if (ret)
			wcn36xx_warn("Exchange feature caps failed\n");
		else
			wcn36xx_feat_caps_info(wcn);
	}

	wcn36xx_detect_chip_version(wcn);

	/* DMA channel initialization */
	ret = wcn36xx_dxe_init(wcn);
	if (ret) {
		wcn36xx_err("DXE init failed\n");
		goto out_smd_stop;
	}

	wcn36xx_debugfs_init(wcn);

	INIT_LIST_HEAD(&wcn->vif_list);
	spin_lock_init(&wcn->dxe_lock);

	return 0;

out_smd_stop:
	wcn36xx_smd_stop(wcn);
out_free_smd_buf:
	kfree(wcn->hal_buf);
out_free_dxe_pool:
	wcn36xx_dxe_free_mem_pools(wcn);
out_free_dxe_ctl:
	wcn36xx_dxe_free_ctl_blks(wcn);
out_smd_close:
	wcn36xx_smd_close(wcn);
out_err:
	return ret;
}

static void wcn36xx_stop(struct ieee80211_hw *hw)
{
	struct wcn36xx *wcn = hw->priv;

	wcn36xx_dbg(WCN36XX_DBG_MAC, "mac stop\n");

	wcn36xx_debugfs_exit(wcn);
	wcn36xx_smd_stop(wcn);
	wcn36xx_dxe_deinit(wcn);
	wcn36xx_smd_close(wcn);

	wcn36xx_dxe_free_mem_pools(wcn);
	wcn36xx_dxe_free_ctl_blks(wcn);

	kfree(wcn->hal_buf);
}

static int wcn36xx_config(struct ieee80211_hw *hw, u32 changed)
{
	struct wcn36xx *wcn = hw->priv;
	struct ieee80211_vif *vif = NULL;
	struct wcn36xx_vif *tmp;

	wcn36xx_dbg(WCN36XX_DBG_MAC, "mac config changed 0x%08x\n", changed);

	if (changed & IEEE80211_CONF_CHANGE_CHANNEL) {
		int ch = WCN36XX_HW_CHANNEL(wcn);
		wcn36xx_dbg(WCN36XX_DBG_MAC, "wcn36xx_config channel switch=%d\n",
			    ch);
		list_for_each_entry(tmp, &wcn->vif_list, list) {
			vif = container_of((void *)tmp,
					   struct ieee80211_vif,
					   drv_priv);
			wcn36xx_smd_switch_channel(wcn, vif, ch);
		}
	}

	return 0;
}

#define WCN36XX_SUPPORTED_FILTERS (0)

static void wcn36xx_configure_filter(struct ieee80211_hw *hw,
				     unsigned int changed,
				     unsigned int *total, u64 multicast)
{
	wcn36xx_dbg(WCN36XX_DBG_MAC, "mac configure filter\n");

	*total &= WCN36XX_SUPPORTED_FILTERS;
}

static void wcn36xx_tx(struct ieee80211_hw *hw,
		       struct ieee80211_tx_control *control,
		       struct sk_buff *skb)
{
	struct wcn36xx *wcn = hw->priv;
	struct wcn36xx_sta *sta_priv = NULL;

	if (control->sta)
		sta_priv = (struct wcn36xx_sta *)control->sta->drv_priv;

	if (wcn36xx_start_tx(wcn, sta_priv, skb))
		ieee80211_free_txskb(wcn->hw, skb);
}

static int wcn36xx_set_key(struct ieee80211_hw *hw, enum set_key_cmd cmd,
			   struct ieee80211_vif *vif,
			   struct ieee80211_sta *sta,
			   struct ieee80211_key_conf *key_conf)
{
	struct wcn36xx *wcn = hw->priv;
	struct wcn36xx_vif *vif_priv = (struct wcn36xx_vif *)vif->drv_priv;
	struct wcn36xx_sta *sta_priv = vif_priv->sta;
	int ret = 0;
	u8 key[WLAN_MAX_KEY_LEN];

	wcn36xx_dbg(WCN36XX_DBG_MAC, "mac80211 set key\n");
	wcn36xx_dbg(WCN36XX_DBG_MAC, "Key: cmd=0x%x algo:0x%x, id:%d, len:%d flags 0x%x\n",
		    cmd, key_conf->cipher, key_conf->keyidx,
		    key_conf->keylen, key_conf->flags);
	wcn36xx_dbg_dump(WCN36XX_DBG_MAC, "KEY: ",
			 key_conf->key,
			 key_conf->keylen);

	switch (key_conf->cipher) {
	case WLAN_CIPHER_SUITE_WEP40:
		vif_priv->encrypt_type = WCN36XX_HAL_ED_WEP40;
		break;
	case WLAN_CIPHER_SUITE_WEP104:
		vif_priv->encrypt_type = WCN36XX_HAL_ED_WEP40;
		break;
	case WLAN_CIPHER_SUITE_CCMP:
		vif_priv->encrypt_type = WCN36XX_HAL_ED_CCMP;
		break;
	case WLAN_CIPHER_SUITE_TKIP:
		vif_priv->encrypt_type = WCN36XX_HAL_ED_TKIP;
		break;
	default:
		wcn36xx_err("Unsupported key type 0x%x\n",
			      key_conf->cipher);
		ret = -EOPNOTSUPP;
		goto out;
	}

	switch (cmd) {
	case SET_KEY:
		if (WCN36XX_HAL_ED_TKIP == vif_priv->encrypt_type) {
			/*
			 * Supplicant is sending key in the wrong order:
			 * Temporal Key (16 b) - TX MIC (8 b) - RX MIC (8 b)
			 * but HW expects it to be in the order as described in
			 * IEEE 802.11 spec (see chapter 11.7) like this:
			 * Temporal Key (16 b) - RX MIC (8 b) - TX MIC (8 b)
			 */
			memcpy(key, key_conf->key, 16);
			memcpy(key + 16, key_conf->key + 24, 8);
			memcpy(key + 24, key_conf->key + 16, 8);
		} else {
			memcpy(key, key_conf->key, key_conf->keylen);
		}

		if (IEEE80211_KEY_FLAG_PAIRWISE & key_conf->flags) {
			sta_priv->is_data_encrypted = true;
			/* Reconfigure bss with encrypt_type */
			if (NL80211_IFTYPE_STATION == vif->type)
				wcn36xx_smd_config_bss(wcn,
						       vif,
						       sta,
						       sta->addr,
						       true);

			wcn36xx_smd_set_stakey(wcn,
				vif_priv->encrypt_type,
				key_conf->keyidx,
				key_conf->keylen,
				key,
				get_sta_index(vif, sta_priv));
		} else {
			wcn36xx_smd_set_bsskey(wcn,
				vif_priv->encrypt_type,
				key_conf->keyidx,
				key_conf->keylen,
				key);
			if ((WLAN_CIPHER_SUITE_WEP40 == key_conf->cipher) ||
			    (WLAN_CIPHER_SUITE_WEP104 == key_conf->cipher)) {
				sta_priv->is_data_encrypted = true;
				wcn36xx_smd_set_stakey(wcn,
					vif_priv->encrypt_type,
					key_conf->keyidx,
					key_conf->keylen,
					key,
					get_sta_index(vif, sta_priv));
			}
		}
		break;
	case DISABLE_KEY:
		if (!(IEEE80211_KEY_FLAG_PAIRWISE & key_conf->flags)) {
			wcn36xx_smd_remove_bsskey(wcn,
				vif_priv->encrypt_type,
				key_conf->keyidx);
		} else {
			sta_priv->is_data_encrypted = false;
			/* do not remove key if disassociated */
			if (sta_priv->aid)
				wcn36xx_smd_remove_stakey(wcn,
					vif_priv->encrypt_type,
					key_conf->keyidx,
					get_sta_index(vif, sta_priv));
		}
		break;
	default:
		wcn36xx_err("Unsupported key cmd 0x%x\n", cmd);
		ret = -EOPNOTSUPP;
		goto out;
	}

out:
	return ret;
}

static void wcn36xx_sw_scan_start(struct ieee80211_hw *hw,
				  struct ieee80211_vif *vif,
				  const u8 *mac_addr)
{
	struct wcn36xx *wcn = hw->priv;

	wcn36xx_smd_init_scan(wcn, HAL_SYS_MODE_SCAN);
	wcn36xx_smd_start_scan(wcn);
}

static void wcn36xx_sw_scan_complete(struct ieee80211_hw *hw,
				     struct ieee80211_vif *vif)
{
	struct wcn36xx *wcn = hw->priv;

	wcn36xx_smd_end_scan(wcn);
	wcn36xx_smd_finish_scan(wcn, HAL_SYS_MODE_SCAN);
}

static void wcn36xx_update_allowed_rates(struct ieee80211_sta *sta,
					 enum ieee80211_band band)
{
	int i, size;
	u16 *rates_table;
	struct wcn36xx_sta *sta_priv = (struct wcn36xx_sta *)sta->drv_priv;
	u32 rates = sta->supp_rates[band];

	memset(&sta_priv->supported_rates, 0,
		sizeof(sta_priv->supported_rates));
	sta_priv->supported_rates.op_rate_mode = STA_11n;

	size = ARRAY_SIZE(sta_priv->supported_rates.dsss_rates);
	rates_table = sta_priv->supported_rates.dsss_rates;
	if (band == IEEE80211_BAND_2GHZ) {
		for (i = 0; i < size; i++) {
			if (rates & 0x01) {
				rates_table[i] = wcn_2ghz_rates[i].hw_value;
				rates = rates >> 1;
			}
		}
	}

	size = ARRAY_SIZE(sta_priv->supported_rates.ofdm_rates);
	rates_table = sta_priv->supported_rates.ofdm_rates;
	for (i = 0; i < size; i++) {
		if (rates & 0x01) {
			rates_table[i] = wcn_5ghz_rates[i].hw_value;
			rates = rates >> 1;
		}
	}

	if (sta->ht_cap.ht_supported) {
		BUILD_BUG_ON(sizeof(sta->ht_cap.mcs.rx_mask) >
			sizeof(sta_priv->supported_rates.supported_mcs_set));
		memcpy(sta_priv->supported_rates.supported_mcs_set,
		       sta->ht_cap.mcs.rx_mask,
		       sizeof(sta->ht_cap.mcs.rx_mask));
	}
}
void wcn36xx_set_default_rates(struct wcn36xx_hal_supported_rates *rates)
{
	u16 ofdm_rates[WCN36XX_HAL_NUM_OFDM_RATES] = {
		HW_RATE_INDEX_6MBPS,
		HW_RATE_INDEX_9MBPS,
		HW_RATE_INDEX_12MBPS,
		HW_RATE_INDEX_18MBPS,
		HW_RATE_INDEX_24MBPS,
		HW_RATE_INDEX_36MBPS,
		HW_RATE_INDEX_48MBPS,
		HW_RATE_INDEX_54MBPS
	};
	u16 dsss_rates[WCN36XX_HAL_NUM_DSSS_RATES] = {
		HW_RATE_INDEX_1MBPS,
		HW_RATE_INDEX_2MBPS,
		HW_RATE_INDEX_5_5MBPS,
		HW_RATE_INDEX_11MBPS
	};

	rates->op_rate_mode = STA_11n;
	memcpy(rates->dsss_rates, dsss_rates,
		sizeof(*dsss_rates) * WCN36XX_HAL_NUM_DSSS_RATES);
	memcpy(rates->ofdm_rates, ofdm_rates,
		sizeof(*ofdm_rates) * WCN36XX_HAL_NUM_OFDM_RATES);
	rates->supported_mcs_set[0] = 0xFF;
}
static void wcn36xx_bss_info_changed(struct ieee80211_hw *hw,
				     struct ieee80211_vif *vif,
				     struct ieee80211_bss_conf *bss_conf,
				     u32 changed)
{
	struct wcn36xx *wcn = hw->priv;
	struct sk_buff *skb = NULL;
	u16 tim_off, tim_len;
	enum wcn36xx_hal_link_state link_state;
	struct wcn36xx_vif *vif_priv = (struct wcn36xx_vif *)vif->drv_priv;

	wcn36xx_dbg(WCN36XX_DBG_MAC, "mac bss info changed vif %p changed 0x%08x\n",
		    vif, changed);

	if (changed & BSS_CHANGED_BEACON_INFO) {
		wcn36xx_dbg(WCN36XX_DBG_MAC,
			    "mac bss changed dtim period %d\n",
			    bss_conf->dtim_period);

		vif_priv->dtim_period = bss_conf->dtim_period;
	}

	if (changed & BSS_CHANGED_PS) {
		wcn36xx_dbg(WCN36XX_DBG_MAC,
			    "mac bss PS set %d\n",
			    bss_conf->ps);
		if (bss_conf->ps) {
			wcn36xx_pmc_enter_bmps_state(wcn, vif);
		} else {
			wcn36xx_pmc_exit_bmps_state(wcn, vif);
		}
	}

	if (changed & BSS_CHANGED_BSSID) {
		wcn36xx_dbg(WCN36XX_DBG_MAC, "mac bss changed_bssid %pM\n",
			    bss_conf->bssid);

		if (!is_zero_ether_addr(bss_conf->bssid)) {
			vif_priv->is_joining = true;
			vif_priv->bss_index = 0xff;
			wcn36xx_smd_join(wcn, bss_conf->bssid,
					 vif->addr, WCN36XX_HW_CHANNEL(wcn));
			wcn36xx_smd_config_bss(wcn, vif, NULL,
					       bss_conf->bssid, false);
		} else {
			vif_priv->is_joining = false;
			wcn36xx_smd_delete_bss(wcn, vif);
		}
	}

	if (changed & BSS_CHANGED_SSID) {
		wcn36xx_dbg(WCN36XX_DBG_MAC,
			    "mac bss changed ssid\n");
		wcn36xx_dbg_dump(WCN36XX_DBG_MAC, "ssid ",
				 bss_conf->ssid, bss_conf->ssid_len);

		vif_priv->ssid.length = bss_conf->ssid_len;
		memcpy(&vif_priv->ssid.ssid,
		       bss_conf->ssid,
		       bss_conf->ssid_len);
	}

	if (changed & BSS_CHANGED_ASSOC) {
		vif_priv->is_joining = false;
		if (bss_conf->assoc) {
			struct ieee80211_sta *sta;
			struct wcn36xx_sta *sta_priv;

			wcn36xx_dbg(WCN36XX_DBG_MAC,
				    "mac assoc bss %pM vif %pM AID=%d\n",
				     bss_conf->bssid,
				     vif->addr,
				     bss_conf->aid);

			rcu_read_lock();
			sta = ieee80211_find_sta(vif, bss_conf->bssid);
			if (!sta) {
				wcn36xx_err("sta %pM is not found\n",
					      bss_conf->bssid);
				rcu_read_unlock();
				goto out;
			}
			sta_priv = (struct wcn36xx_sta *)sta->drv_priv;

			wcn36xx_update_allowed_rates(sta, WCN36XX_BAND(wcn));

			wcn36xx_smd_set_link_st(wcn, bss_conf->bssid,
				vif->addr,
				WCN36XX_HAL_LINK_POSTASSOC_STATE);
			wcn36xx_smd_config_bss(wcn, vif, sta,
					       bss_conf->bssid,
					       true);
			sta_priv->aid = bss_conf->aid;
			/*
			 * config_sta must be called from  because this is the
			 * place where AID is available.
			 */
			wcn36xx_smd_config_sta(wcn, vif, sta);
			rcu_read_unlock();
		} else {
			wcn36xx_dbg(WCN36XX_DBG_MAC,
				    "disassociated bss %pM vif %pM AID=%d\n",
				    bss_conf->bssid,
				    vif->addr,
				    bss_conf->aid);
			wcn36xx_smd_set_link_st(wcn,
						bss_conf->bssid,
						vif->addr,
						WCN36XX_HAL_LINK_IDLE_STATE);
		}
	}

	if (changed & BSS_CHANGED_AP_PROBE_RESP) {
		wcn36xx_dbg(WCN36XX_DBG_MAC, "mac bss changed ap probe resp\n");
		skb = ieee80211_proberesp_get(hw, vif);
		if (!skb) {
			wcn36xx_err("failed to alloc probereq skb\n");
			goto out;
		}

		wcn36xx_smd_update_proberesp_tmpl(wcn, vif, skb);
		dev_kfree_skb(skb);
	}

	if (changed & BSS_CHANGED_BEACON_ENABLED ||
	    changed & BSS_CHANGED_BEACON) {
		wcn36xx_dbg(WCN36XX_DBG_MAC,
			    "mac bss changed beacon enabled %d\n",
			    bss_conf->enable_beacon);

		if (bss_conf->enable_beacon) {
			vif_priv->dtim_period = bss_conf->dtim_period;
			vif_priv->bss_index = 0xff;
			wcn36xx_smd_config_bss(wcn, vif, NULL,
					       vif->addr, false);
			skb = ieee80211_beacon_get_tim(hw, vif, &tim_off,
						       &tim_len);
			if (!skb) {
				wcn36xx_err("failed to alloc beacon skb\n");
				goto out;
			}
			wcn36xx_smd_send_beacon(wcn, vif, skb, tim_off, 0);
			dev_kfree_skb(skb);

			if (vif->type == NL80211_IFTYPE_ADHOC ||
			    vif->type == NL80211_IFTYPE_MESH_POINT)
				link_state = WCN36XX_HAL_LINK_IBSS_STATE;
			else
				link_state = WCN36XX_HAL_LINK_AP_STATE;

			wcn36xx_smd_set_link_st(wcn, vif->addr, vif->addr,
						link_state);
		} else {
			wcn36xx_smd_set_link_st(wcn, vif->addr, vif->addr,
						WCN36XX_HAL_LINK_IDLE_STATE);
			wcn36xx_smd_delete_bss(wcn, vif);
		}
	}
out:
	return;
}

/* this is required when using IEEE80211_HW_HAS_RATE_CONTROL */
static int wcn36xx_set_rts_threshold(struct ieee80211_hw *hw, u32 value)
{
	struct wcn36xx *wcn = hw->priv;
	wcn36xx_dbg(WCN36XX_DBG_MAC, "mac set RTS threshold %d\n", value);

	wcn36xx_smd_update_cfg(wcn, WCN36XX_HAL_CFG_RTS_THRESHOLD, value);
	return 0;
}

static void wcn36xx_remove_interface(struct ieee80211_hw *hw,
				     struct ieee80211_vif *vif)
{
	struct wcn36xx *wcn = hw->priv;
	struct wcn36xx_vif *vif_priv = (struct wcn36xx_vif *)vif->drv_priv;
	wcn36xx_dbg(WCN36XX_DBG_MAC, "mac remove interface vif %p\n", vif);

	list_del(&vif_priv->list);
	wcn36xx_smd_delete_sta_self(wcn, vif->addr);
}

static int wcn36xx_add_interface(struct ieee80211_hw *hw,
				 struct ieee80211_vif *vif)
{
	struct wcn36xx *wcn = hw->priv;
	struct wcn36xx_vif *vif_priv = (struct wcn36xx_vif *)vif->drv_priv;

	wcn36xx_dbg(WCN36XX_DBG_MAC, "mac add interface vif %p type %d\n",
		    vif, vif->type);

	if (!(NL80211_IFTYPE_STATION == vif->type ||
	      NL80211_IFTYPE_AP == vif->type ||
	      NL80211_IFTYPE_ADHOC == vif->type ||
	      NL80211_IFTYPE_MESH_POINT == vif->type)) {
		wcn36xx_warn("Unsupported interface type requested: %d\n",
			     vif->type);
		return -EOPNOTSUPP;
	}

	list_add(&vif_priv->list, &wcn->vif_list);
	wcn36xx_smd_add_sta_self(wcn, vif);

	return 0;
}

static int wcn36xx_sta_add(struct ieee80211_hw *hw, struct ieee80211_vif *vif,
			   struct ieee80211_sta *sta)
{
	struct wcn36xx *wcn = hw->priv;
	struct wcn36xx_vif *vif_priv = (struct wcn36xx_vif *)vif->drv_priv;
	struct wcn36xx_sta *sta_priv = (struct wcn36xx_sta *)sta->drv_priv;
	wcn36xx_dbg(WCN36XX_DBG_MAC, "mac sta add vif %p sta %pM\n",
		    vif, sta->addr);

	spin_lock_init(&sta_priv->ampdu_lock);
	vif_priv->sta = sta_priv;
	sta_priv->vif = vif_priv;
	/*
	 * For STA mode HW will be configured on BSS_CHANGED_ASSOC because
	 * at this stage AID is not available yet.
	 */
	if (NL80211_IFTYPE_STATION != vif->type) {
		wcn36xx_update_allowed_rates(sta, WCN36XX_BAND(wcn));
		sta_priv->aid = sta->aid;
		wcn36xx_smd_config_sta(wcn, vif, sta);
	}
	return 0;
}

static int wcn36xx_sta_remove(struct ieee80211_hw *hw,
			      struct ieee80211_vif *vif,
			      struct ieee80211_sta *sta)
{
	struct wcn36xx *wcn = hw->priv;
	struct wcn36xx_vif *vif_priv = (struct wcn36xx_vif *)vif->drv_priv;
	struct wcn36xx_sta *sta_priv = (struct wcn36xx_sta *)sta->drv_priv;

	wcn36xx_dbg(WCN36XX_DBG_MAC, "mac sta remove vif %p sta %pM index %d\n",
		    vif, sta->addr, sta_priv->sta_index);

	wcn36xx_smd_delete_sta(wcn, sta_priv->sta_index);
	vif_priv->sta = NULL;
	sta_priv->vif = NULL;
	return 0;
}

#ifdef CONFIG_PM

static int wcn36xx_suspend(struct ieee80211_hw *hw, struct cfg80211_wowlan *wow)
{
	struct wcn36xx *wcn = hw->priv;

	wcn36xx_dbg(WCN36XX_DBG_MAC, "mac suspend\n");

	flush_workqueue(wcn->hal_ind_wq);
	wcn36xx_smd_set_power_params(wcn, true);
	return 0;
}

static int wcn36xx_resume(struct ieee80211_hw *hw)
{
	struct wcn36xx *wcn = hw->priv;

	wcn36xx_dbg(WCN36XX_DBG_MAC, "mac resume\n");

	flush_workqueue(wcn->hal_ind_wq);
	wcn36xx_smd_set_power_params(wcn, false);
	return 0;
}

#endif

static int wcn36xx_ampdu_action(struct ieee80211_hw *hw,
		    struct ieee80211_vif *vif,
		    enum ieee80211_ampdu_mlme_action action,
		    struct ieee80211_sta *sta, u16 tid, u16 *ssn,
		    u8 buf_size)
{
	struct wcn36xx *wcn = hw->priv;
	struct wcn36xx_sta *sta_priv = NULL;

	wcn36xx_dbg(WCN36XX_DBG_MAC, "mac ampdu action action %d tid %d\n",
		    action, tid);

	sta_priv = (struct wcn36xx_sta *)sta->drv_priv;

	switch (action) {
	case IEEE80211_AMPDU_RX_START:
		sta_priv->tid = tid;
		wcn36xx_smd_add_ba_session(wcn, sta, tid, ssn, 0,
			get_sta_index(vif, sta_priv));
		wcn36xx_smd_add_ba(wcn);
		wcn36xx_smd_trigger_ba(wcn, get_sta_index(vif, sta_priv));
		break;
	case IEEE80211_AMPDU_RX_STOP:
		wcn36xx_smd_del_ba(wcn, tid, get_sta_index(vif, sta_priv));
		break;
	case IEEE80211_AMPDU_TX_START:
		spin_lock_bh(&sta_priv->ampdu_lock);
		sta_priv->ampdu_state[tid] = WCN36XX_AMPDU_START;
		spin_unlock_bh(&sta_priv->ampdu_lock);

		ieee80211_start_tx_ba_cb_irqsafe(vif, sta->addr, tid);
		break;
	case IEEE80211_AMPDU_TX_OPERATIONAL:
		spin_lock_bh(&sta_priv->ampdu_lock);
		sta_priv->ampdu_state[tid] = WCN36XX_AMPDU_OPERATIONAL;
		spin_unlock_bh(&sta_priv->ampdu_lock);

		wcn36xx_smd_add_ba_session(wcn, sta, tid, ssn, 1,
			get_sta_index(vif, sta_priv));
		break;
	case IEEE80211_AMPDU_TX_STOP_FLUSH:
	case IEEE80211_AMPDU_TX_STOP_FLUSH_CONT:
	case IEEE80211_AMPDU_TX_STOP_CONT:
		spin_lock_bh(&sta_priv->ampdu_lock);
		sta_priv->ampdu_state[tid] = WCN36XX_AMPDU_NONE;
		spin_unlock_bh(&sta_priv->ampdu_lock);

		ieee80211_stop_tx_ba_cb_irqsafe(vif, sta->addr, tid);
		break;
	default:
		wcn36xx_err("Unknown AMPDU action\n");
	}

	return 0;
}

static const struct ieee80211_ops wcn36xx_ops = {
	.start			= wcn36xx_start,
	.stop			= wcn36xx_stop,
	.add_interface		= wcn36xx_add_interface,
	.remove_interface	= wcn36xx_remove_interface,
#ifdef CONFIG_PM
	.suspend		= wcn36xx_suspend,
	.resume			= wcn36xx_resume,
#endif
	.config			= wcn36xx_config,
	.configure_filter       = wcn36xx_configure_filter,
	.tx			= wcn36xx_tx,
	.set_key		= wcn36xx_set_key,
	.sw_scan_start		= wcn36xx_sw_scan_start,
	.sw_scan_complete	= wcn36xx_sw_scan_complete,
	.bss_info_changed	= wcn36xx_bss_info_changed,
	.set_rts_threshold	= wcn36xx_set_rts_threshold,
	.sta_add		= wcn36xx_sta_add,
	.sta_remove		= wcn36xx_sta_remove,
	.ampdu_action		= wcn36xx_ampdu_action,
};

static int wcn36xx_init_ieee80211(struct wcn36xx *wcn)
{
	int ret = 0;

	static const u32 cipher_suites[] = {
		WLAN_CIPHER_SUITE_WEP40,
		WLAN_CIPHER_SUITE_WEP104,
		WLAN_CIPHER_SUITE_TKIP,
		WLAN_CIPHER_SUITE_CCMP,
	};

	wcn->hw->flags = IEEE80211_HW_SIGNAL_DBM |
		IEEE80211_HW_HAS_RATE_CONTROL |
		IEEE80211_HW_SUPPORTS_PS |
		IEEE80211_HW_CONNECTION_MONITOR |
		IEEE80211_HW_AMPDU_AGGREGATION |
		IEEE80211_HW_TIMING_BEACON_ONLY;

	wcn->hw->wiphy->interface_modes = BIT(NL80211_IFTYPE_STATION) |
		BIT(NL80211_IFTYPE_AP) |
		BIT(NL80211_IFTYPE_ADHOC) |
		BIT(NL80211_IFTYPE_MESH_POINT);

	wcn->hw->wiphy->bands[IEEE80211_BAND_2GHZ] = &wcn_band_2ghz;
	wcn->hw->wiphy->bands[IEEE80211_BAND_5GHZ] = &wcn_band_5ghz;

	wcn->hw->wiphy->cipher_suites = cipher_suites;
	wcn->hw->wiphy->n_cipher_suites = ARRAY_SIZE(cipher_suites);

	wcn->hw->wiphy->flags |= WIPHY_FLAG_AP_PROBE_RESP_OFFLOAD;

#ifdef CONFIG_PM
	wcn->hw->wiphy->wowlan = &wowlan_support;
#endif

	wcn->hw->max_listen_interval = 200;

	wcn->hw->queues = 4;

	SET_IEEE80211_DEV(wcn->hw, wcn->dev);

	wcn->hw->sta_data_size = sizeof(struct wcn36xx_sta);
	wcn->hw->vif_data_size = sizeof(struct wcn36xx_vif);

	return ret;
}

static int wcn36xx_platform_get_resources(struct wcn36xx *wcn,
					  struct platform_device *pdev)
{
	struct resource *res;
	/* Set TX IRQ */
	res = platform_get_resource_byname(pdev, IORESOURCE_IRQ,
					   "wcnss_wlantx_irq");
	if (!res) {
		wcn36xx_err("failed to get tx_irq\n");
		return -ENOENT;
	}
	wcn->tx_irq = res->start;

	/* Set RX IRQ */
	res = platform_get_resource_byname(pdev, IORESOURCE_IRQ,
					   "wcnss_wlanrx_irq");
	if (!res) {
		wcn36xx_err("failed to get rx_irq\n");
		return -ENOENT;
	}
	wcn->rx_irq = res->start;

	/* Map the memory */
	res = platform_get_resource_byname(pdev, IORESOURCE_MEM,
						 "wcnss_mmio");
	if (!res) {
		wcn36xx_err("failed to get mmio\n");
		return -ENOENT;
	}
	wcn->mmio = ioremap(res->start, resource_size(res));
	if (!wcn->mmio) {
		wcn36xx_err("failed to map io memory\n");
		return -ENOMEM;
	}
	return 0;
}

static int wcn36xx_probe(struct platform_device *pdev)
{
	struct ieee80211_hw *hw;
	struct wcn36xx *wcn;
	int ret;
	u8 addr[ETH_ALEN];

	wcn36xx_dbg(WCN36XX_DBG_MAC, "platform probe\n");

	hw = ieee80211_alloc_hw(sizeof(struct wcn36xx), &wcn36xx_ops);
	if (!hw) {
		wcn36xx_err("failed to alloc hw\n");
		ret = -ENOMEM;
		goto out_err;
	}
	platform_set_drvdata(pdev, hw);
	wcn = hw->priv;
	wcn->hw = hw;
	wcn->dev = &pdev->dev;
	wcn->ctrl_ops = pdev->dev.platform_data;

	mutex_init(&wcn->hal_mutex);

	if (!wcn->ctrl_ops->get_hw_mac(addr)) {
		wcn36xx_info("mac address: %pM\n", addr);
		SET_IEEE80211_PERM_ADDR(wcn->hw, addr);
	}

	ret = wcn36xx_platform_get_resources(wcn, pdev);
	if (ret)
		goto out_wq;

	wcn36xx_init_ieee80211(wcn);
	ret = ieee80211_register_hw(wcn->hw);
	if (ret)
		goto out_unmap;

	return 0;

out_unmap:
	iounmap(wcn->mmio);
out_wq:
	ieee80211_free_hw(hw);
out_err:
	return ret;
}
static int wcn36xx_remove(struct platform_device *pdev)
{
	struct ieee80211_hw *hw = platform_get_drvdata(pdev);
	struct wcn36xx *wcn = hw->priv;
	wcn36xx_dbg(WCN36XX_DBG_MAC, "platform remove\n");

	release_firmware(wcn->nv);
	mutex_destroy(&wcn->hal_mutex);

	ieee80211_unregister_hw(hw);
	iounmap(wcn->mmio);
	ieee80211_free_hw(hw);

	return 0;
}
static const struct platform_device_id wcn36xx_platform_id_table[] = {
	{
		.name = "wcn36xx",
		.driver_data = 0
	},
	{}
};
MODULE_DEVICE_TABLE(platform, wcn36xx_platform_id_table);

static struct platform_driver wcn36xx_driver = {
	.probe      = wcn36xx_probe,
	.remove     = wcn36xx_remove,
	.driver         = {
		.name   = "wcn36xx",
	},
	.id_table    = wcn36xx_platform_id_table,
};

static int __init wcn36xx_init(void)
{
	platform_driver_register(&wcn36xx_driver);
	return 0;
}
module_init(wcn36xx_init);

static void __exit wcn36xx_exit(void)
{
	platform_driver_unregister(&wcn36xx_driver);
}
module_exit(wcn36xx_exit);

MODULE_LICENSE("Dual BSD/GPL");
MODULE_AUTHOR("Eugene Krasnikov k.eugene.e@gmail.com");
MODULE_FIRMWARE(WLAN_NV_FILE);
