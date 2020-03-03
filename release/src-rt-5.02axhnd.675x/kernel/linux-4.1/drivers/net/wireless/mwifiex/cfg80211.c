/*
 * Marvell Wireless LAN device driver: CFG80211
 *
 * Copyright (C) 2011-2014, Marvell International Ltd.
 *
 * This software file (the "File") is distributed by Marvell International
 * Ltd. under the terms of the GNU General Public License Version 2, June 1991
 * (the "License").  You may use, redistribute and/or modify this File in
 * accordance with the terms and conditions of the License, a copy of which
 * is available by writing to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA or on the
 * worldwide web at http://www.gnu.org/licenses/old-licenses/gpl-2.0.txt.
 *
 * THE FILE IS DISTRIBUTED AS-IS, WITHOUT WARRANTY OF ANY KIND, AND THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE
 * ARE EXPRESSLY DISCLAIMED.  The License provides additional details about
 * this warranty disclaimer.
 */

#include "cfg80211.h"
#include "main.h"

static char *reg_alpha2;
module_param(reg_alpha2, charp, 0);

static const struct ieee80211_iface_limit mwifiex_ap_sta_limits[] = {
	{
		.max = 2, .types = BIT(NL80211_IFTYPE_STATION) |
				   BIT(NL80211_IFTYPE_P2P_GO) |
				   BIT(NL80211_IFTYPE_P2P_CLIENT),
	},
	{
		.max = 1, .types = BIT(NL80211_IFTYPE_AP),
	},
};

static const struct ieee80211_iface_combination mwifiex_iface_comb_ap_sta = {
	.limits = mwifiex_ap_sta_limits,
	.num_different_channels = 1,
	.n_limits = ARRAY_SIZE(mwifiex_ap_sta_limits),
	.max_interfaces = MWIFIEX_MAX_BSS_NUM,
	.beacon_int_infra_match = true,
};

/*
 * This function maps the nl802.11 channel type into driver channel type.
 *
 * The mapping is as follows -
 *      NL80211_CHAN_NO_HT     -> IEEE80211_HT_PARAM_CHA_SEC_NONE
 *      NL80211_CHAN_HT20      -> IEEE80211_HT_PARAM_CHA_SEC_NONE
 *      NL80211_CHAN_HT40PLUS  -> IEEE80211_HT_PARAM_CHA_SEC_ABOVE
 *      NL80211_CHAN_HT40MINUS -> IEEE80211_HT_PARAM_CHA_SEC_BELOW
 *      Others                 -> IEEE80211_HT_PARAM_CHA_SEC_NONE
 */
u8 mwifiex_chan_type_to_sec_chan_offset(enum nl80211_channel_type chan_type)
{
	switch (chan_type) {
	case NL80211_CHAN_NO_HT:
	case NL80211_CHAN_HT20:
		return IEEE80211_HT_PARAM_CHA_SEC_NONE;
	case NL80211_CHAN_HT40PLUS:
		return IEEE80211_HT_PARAM_CHA_SEC_ABOVE;
	case NL80211_CHAN_HT40MINUS:
		return IEEE80211_HT_PARAM_CHA_SEC_BELOW;
	default:
		return IEEE80211_HT_PARAM_CHA_SEC_NONE;
	}
}

/*
 * This function checks whether WEP is set.
 */
static int
mwifiex_is_alg_wep(u32 cipher)
{
	switch (cipher) {
	case WLAN_CIPHER_SUITE_WEP40:
	case WLAN_CIPHER_SUITE_WEP104:
		return 1;
	default:
		break;
	}

	return 0;
}

/*
 * This function retrieves the private structure from kernel wiphy structure.
 */
static void *mwifiex_cfg80211_get_adapter(struct wiphy *wiphy)
{
	return (void *) (*(unsigned long *) wiphy_priv(wiphy));
}

/*
 * CFG802.11 operation handler to delete a network key.
 */
static int
mwifiex_cfg80211_del_key(struct wiphy *wiphy, struct net_device *netdev,
			 u8 key_index, bool pairwise, const u8 *mac_addr)
{
	struct mwifiex_private *priv = mwifiex_netdev_get_priv(netdev);
	const u8 bc_mac[] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff};
	const u8 *peer_mac = pairwise ? mac_addr : bc_mac;

	if (mwifiex_set_encode(priv, NULL, NULL, 0, key_index, peer_mac, 1)) {
		wiphy_err(wiphy, "deleting the crypto keys\n");
		return -EFAULT;
	}

	wiphy_dbg(wiphy, "info: crypto keys deleted\n");
	return 0;
}

/*
 * This function forms an skb for management frame.
 */
static int
mwifiex_form_mgmt_frame(struct sk_buff *skb, const u8 *buf, size_t len)
{
	u8 addr[ETH_ALEN] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
	u16 pkt_len;
	u32 tx_control = 0, pkt_type = PKT_TYPE_MGMT;

	pkt_len = len + ETH_ALEN;

	skb_reserve(skb, MWIFIEX_MIN_DATA_HEADER_LEN +
		    MWIFIEX_MGMT_FRAME_HEADER_SIZE + sizeof(pkt_len));
	memcpy(skb_push(skb, sizeof(pkt_len)), &pkt_len, sizeof(pkt_len));

	memcpy(skb_push(skb, sizeof(tx_control)),
	       &tx_control, sizeof(tx_control));

	memcpy(skb_push(skb, sizeof(pkt_type)), &pkt_type, sizeof(pkt_type));

	/* Add packet data and address4 */
	memcpy(skb_put(skb, sizeof(struct ieee80211_hdr_3addr)), buf,
	       sizeof(struct ieee80211_hdr_3addr));
	memcpy(skb_put(skb, ETH_ALEN), addr, ETH_ALEN);
	memcpy(skb_put(skb, len - sizeof(struct ieee80211_hdr_3addr)),
	       buf + sizeof(struct ieee80211_hdr_3addr),
	       len - sizeof(struct ieee80211_hdr_3addr));

	skb->priority = LOW_PRIO_TID;
	__net_timestamp(skb);

	return 0;
}

/*
 * CFG802.11 operation handler to transmit a management frame.
 */
static int
mwifiex_cfg80211_mgmt_tx(struct wiphy *wiphy, struct wireless_dev *wdev,
			 struct cfg80211_mgmt_tx_params *params, u64 *cookie)
{
	const u8 *buf = params->buf;
	size_t len = params->len;
	struct sk_buff *skb;
	u16 pkt_len;
	const struct ieee80211_mgmt *mgmt;
	struct mwifiex_txinfo *tx_info;
	struct mwifiex_private *priv = mwifiex_netdev_get_priv(wdev->netdev);

	if (!buf || !len) {
		wiphy_err(wiphy, "invalid buffer and length\n");
		return -EFAULT;
	}

	mgmt = (const struct ieee80211_mgmt *)buf;
	if (GET_BSS_ROLE(priv) != MWIFIEX_BSS_ROLE_STA &&
	    ieee80211_is_probe_resp(mgmt->frame_control)) {
		/* Since we support offload probe resp, we need to skip probe
		 * resp in AP or GO mode */
		wiphy_dbg(wiphy,
			  "info: skip to send probe resp in AP or GO mode\n");
		return 0;
	}

	pkt_len = len + ETH_ALEN;
	skb = dev_alloc_skb(MWIFIEX_MIN_DATA_HEADER_LEN +
			    MWIFIEX_MGMT_FRAME_HEADER_SIZE +
			    pkt_len + sizeof(pkt_len));

	if (!skb) {
		wiphy_err(wiphy, "allocate skb failed for management frame\n");
		return -ENOMEM;
	}

	tx_info = MWIFIEX_SKB_TXCB(skb);
	memset(tx_info, 0, sizeof(*tx_info));
	tx_info->bss_num = priv->bss_num;
	tx_info->bss_type = priv->bss_type;
	tx_info->pkt_len = pkt_len;

	mwifiex_form_mgmt_frame(skb, buf, len);
	*cookie = prandom_u32() | 1;

	if (ieee80211_is_action(mgmt->frame_control))
		skb = mwifiex_clone_skb_for_tx_status(priv,
						      skb,
				MWIFIEX_BUF_FLAG_ACTION_TX_STATUS, cookie);
	else
		cfg80211_mgmt_tx_status(wdev, *cookie, buf, len, true,
					GFP_ATOMIC);

	mwifiex_queue_tx_pkt(priv, skb);

	wiphy_dbg(wiphy, "info: management frame transmitted\n");
	return 0;
}

/*
 * CFG802.11 operation handler to register a mgmt frame.
 */
static void
mwifiex_cfg80211_mgmt_frame_register(struct wiphy *wiphy,
				     struct wireless_dev *wdev,
				     u16 frame_type, bool reg)
{
	struct mwifiex_private *priv = mwifiex_netdev_get_priv(wdev->netdev);
	u32 mask;

	if (reg)
		mask = priv->mgmt_frame_mask | BIT(frame_type >> 4);
	else
		mask = priv->mgmt_frame_mask & ~BIT(frame_type >> 4);

	if (mask != priv->mgmt_frame_mask) {
		priv->mgmt_frame_mask = mask;
		mwifiex_send_cmd(priv, HostCmd_CMD_MGMT_FRAME_REG,
				 HostCmd_ACT_GEN_SET, 0,
				 &priv->mgmt_frame_mask, false);
		wiphy_dbg(wiphy, "info: mgmt frame registered\n");
	}
}

/*
 * CFG802.11 operation handler to remain on channel.
 */
static int
mwifiex_cfg80211_remain_on_channel(struct wiphy *wiphy,
				   struct wireless_dev *wdev,
				   struct ieee80211_channel *chan,
				   unsigned int duration, u64 *cookie)
{
	struct mwifiex_private *priv = mwifiex_netdev_get_priv(wdev->netdev);
	int ret;

	if (!chan || !cookie) {
		wiphy_err(wiphy, "Invalid parameter for ROC\n");
		return -EINVAL;
	}

	if (priv->roc_cfg.cookie) {
		wiphy_dbg(wiphy, "info: ongoing ROC, cookie = 0x%llx\n",
			  priv->roc_cfg.cookie);
		return -EBUSY;
	}

	ret = mwifiex_remain_on_chan_cfg(priv, HostCmd_ACT_GEN_SET, chan,
					 duration);

	if (!ret) {
		*cookie = prandom_u32() | 1;
		priv->roc_cfg.cookie = *cookie;
		priv->roc_cfg.chan = *chan;

		cfg80211_ready_on_channel(wdev, *cookie, chan,
					  duration, GFP_ATOMIC);

		wiphy_dbg(wiphy, "info: ROC, cookie = 0x%llx\n", *cookie);
	}

	return ret;
}

/*
 * CFG802.11 operation handler to cancel remain on channel.
 */
static int
mwifiex_cfg80211_cancel_remain_on_channel(struct wiphy *wiphy,
					  struct wireless_dev *wdev, u64 cookie)
{
	struct mwifiex_private *priv = mwifiex_netdev_get_priv(wdev->netdev);
	int ret;

	if (cookie != priv->roc_cfg.cookie)
		return -ENOENT;

	ret = mwifiex_remain_on_chan_cfg(priv, HostCmd_ACT_GEN_REMOVE,
					 &priv->roc_cfg.chan, 0);

	if (!ret) {
		cfg80211_remain_on_channel_expired(wdev, cookie,
						   &priv->roc_cfg.chan,
						   GFP_ATOMIC);

		memset(&priv->roc_cfg, 0, sizeof(struct mwifiex_roc_cfg));

		wiphy_dbg(wiphy, "info: cancel ROC, cookie = 0x%llx\n", cookie);
	}

	return ret;
}

/*
 * CFG802.11 operation handler to set Tx power.
 */
static int
mwifiex_cfg80211_set_tx_power(struct wiphy *wiphy,
			      struct wireless_dev *wdev,
			      enum nl80211_tx_power_setting type,
			      int mbm)
{
	struct mwifiex_adapter *adapter = mwifiex_cfg80211_get_adapter(wiphy);
	struct mwifiex_private *priv;
	struct mwifiex_power_cfg power_cfg;
	int dbm = MBM_TO_DBM(mbm);

	if (type == NL80211_TX_POWER_FIXED) {
		power_cfg.is_power_auto = 0;
		power_cfg.power_level = dbm;
	} else {
		power_cfg.is_power_auto = 1;
	}

	priv = mwifiex_get_priv(adapter, MWIFIEX_BSS_ROLE_ANY);

	return mwifiex_set_tx_power(priv, &power_cfg);
}

/*
 * CFG802.11 operation handler to set Power Save option.
 *
 * The timeout value, if provided, is currently ignored.
 */
static int
mwifiex_cfg80211_set_power_mgmt(struct wiphy *wiphy,
				struct net_device *dev,
				bool enabled, int timeout)
{
	struct mwifiex_private *priv = mwifiex_netdev_get_priv(dev);
	u32 ps_mode;

	if (timeout)
		wiphy_dbg(wiphy,
			  "info: ignore timeout value for IEEE Power Save\n");

	ps_mode = enabled;

	return mwifiex_drv_set_power(priv, &ps_mode);
}

/*
 * CFG802.11 operation handler to set the default network key.
 */
static int
mwifiex_cfg80211_set_default_key(struct wiphy *wiphy, struct net_device *netdev,
				 u8 key_index, bool unicast,
				 bool multicast)
{
	struct mwifiex_private *priv = mwifiex_netdev_get_priv(netdev);

	/* Return if WEP key not configured */
	if (!priv->sec_info.wep_enabled)
		return 0;

	if (priv->bss_type == MWIFIEX_BSS_TYPE_UAP) {
		priv->wep_key_curr_index = key_index;
	} else if (mwifiex_set_encode(priv, NULL, NULL, 0, key_index,
				      NULL, 0)) {
		wiphy_err(wiphy, "set default Tx key index\n");
		return -EFAULT;
	}

	return 0;
}

/*
 * CFG802.11 operation handler to add a network key.
 */
static int
mwifiex_cfg80211_add_key(struct wiphy *wiphy, struct net_device *netdev,
			 u8 key_index, bool pairwise, const u8 *mac_addr,
			 struct key_params *params)
{
	struct mwifiex_private *priv = mwifiex_netdev_get_priv(netdev);
	struct mwifiex_wep_key *wep_key;
	const u8 bc_mac[] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff};
	const u8 *peer_mac = pairwise ? mac_addr : bc_mac;

	if (GET_BSS_ROLE(priv) == MWIFIEX_BSS_ROLE_UAP &&
	    (params->cipher == WLAN_CIPHER_SUITE_WEP40 ||
	     params->cipher == WLAN_CIPHER_SUITE_WEP104)) {
		if (params->key && params->key_len) {
			wep_key = &priv->wep_key[key_index];
			memset(wep_key, 0, sizeof(struct mwifiex_wep_key));
			memcpy(wep_key->key_material, params->key,
			       params->key_len);
			wep_key->key_index = key_index;
			wep_key->key_length = params->key_len;
			priv->sec_info.wep_enabled = 1;
		}
		return 0;
	}

	if (mwifiex_set_encode(priv, params, params->key, params->key_len,
			       key_index, peer_mac, 0)) {
		wiphy_err(wiphy, "crypto keys added\n");
		return -EFAULT;
	}

	return 0;
}

/*
 * This function sends domain information to the firmware.
 *
 * The following information are passed to the firmware -
 *      - Country codes
 *      - Sub bands (first channel, number of channels, maximum Tx power)
 */
static int mwifiex_send_domain_info_cmd_fw(struct wiphy *wiphy)
{
	u8 no_of_triplet = 0;
	struct ieee80211_country_ie_triplet *t;
	u8 no_of_parsed_chan = 0;
	u8 first_chan = 0, next_chan = 0, max_pwr = 0;
	u8 i, flag = 0;
	enum ieee80211_band band;
	struct ieee80211_supported_band *sband;
	struct ieee80211_channel *ch;
	struct mwifiex_adapter *adapter = mwifiex_cfg80211_get_adapter(wiphy);
	struct mwifiex_private *priv;
	struct mwifiex_802_11d_domain_reg *domain_info = &adapter->domain_reg;

	/* Set country code */
	domain_info->country_code[0] = adapter->country_code[0];
	domain_info->country_code[1] = adapter->country_code[1];
	domain_info->country_code[2] = ' ';

	band = mwifiex_band_to_radio_type(adapter->config_bands);
	if (!wiphy->bands[band]) {
		wiphy_err(wiphy, "11D: setting domain info in FW\n");
		return -1;
	}

	sband = wiphy->bands[band];

	for (i = 0; i < sband->n_channels ; i++) {
		ch = &sband->channels[i];
		if (ch->flags & IEEE80211_CHAN_DISABLED)
			continue;

		if (!flag) {
			flag = 1;
			first_chan = (u32) ch->hw_value;
			next_chan = first_chan;
			max_pwr = ch->max_power;
			no_of_parsed_chan = 1;
			continue;
		}

		if (ch->hw_value == next_chan + 1 &&
		    ch->max_power == max_pwr) {
			next_chan++;
			no_of_parsed_chan++;
		} else {
			t = &domain_info->triplet[no_of_triplet];
			t->chans.first_channel = first_chan;
			t->chans.num_channels = no_of_parsed_chan;
			t->chans.max_power = max_pwr;
			no_of_triplet++;
			first_chan = (u32) ch->hw_value;
			next_chan = first_chan;
			max_pwr = ch->max_power;
			no_of_parsed_chan = 1;
		}
	}

	if (flag) {
		t = &domain_info->triplet[no_of_triplet];
		t->chans.first_channel = first_chan;
		t->chans.num_channels = no_of_parsed_chan;
		t->chans.max_power = max_pwr;
		no_of_triplet++;
	}

	domain_info->no_of_triplet = no_of_triplet;

	priv = mwifiex_get_priv(adapter, MWIFIEX_BSS_ROLE_ANY);

	if (mwifiex_send_cmd(priv, HostCmd_CMD_802_11D_DOMAIN_INFO,
			     HostCmd_ACT_GEN_SET, 0, NULL, false)) {
		wiphy_err(wiphy, "11D: setting domain info in FW\n");
		return -1;
	}

	return 0;
}

/*
 * CFG802.11 regulatory domain callback function.
 *
 * This function is called when the regulatory domain is changed due to the
 * following reasons -
 *      - Set by driver
 *      - Set by system core
 *      - Set by user
 *      - Set bt Country IE
 */
static void mwifiex_reg_notifier(struct wiphy *wiphy,
				 struct regulatory_request *request)
{
	struct mwifiex_adapter *adapter = mwifiex_cfg80211_get_adapter(wiphy);
	struct mwifiex_private *priv = mwifiex_get_priv(adapter,
							MWIFIEX_BSS_ROLE_ANY);

	wiphy_dbg(wiphy, "info: cfg80211 regulatory domain callback for %c%c\n",
		  request->alpha2[0], request->alpha2[1]);

	switch (request->initiator) {
	case NL80211_REGDOM_SET_BY_DRIVER:
	case NL80211_REGDOM_SET_BY_CORE:
	case NL80211_REGDOM_SET_BY_USER:
	case NL80211_REGDOM_SET_BY_COUNTRY_IE:
		break;
	default:
		wiphy_err(wiphy, "unknown regdom initiator: %d\n",
			  request->initiator);
		return;
	}

	/* Don't send world or same regdom info to firmware */
	if (strncmp(request->alpha2, "00", 2) &&
	    strncmp(request->alpha2, adapter->country_code,
		    sizeof(request->alpha2))) {
		memcpy(adapter->country_code, request->alpha2,
		       sizeof(request->alpha2));
		mwifiex_send_domain_info_cmd_fw(wiphy);
		mwifiex_dnld_txpwr_table(priv);
	}
}

/*
 * This function sets the fragmentation threshold.
 *
 * The fragmentation threshold value must lie between MWIFIEX_FRAG_MIN_VALUE
 * and MWIFIEX_FRAG_MAX_VALUE.
 */
static int
mwifiex_set_frag(struct mwifiex_private *priv, u32 frag_thr)
{
	if (frag_thr < MWIFIEX_FRAG_MIN_VALUE ||
	    frag_thr > MWIFIEX_FRAG_MAX_VALUE)
		frag_thr = MWIFIEX_FRAG_MAX_VALUE;

	return mwifiex_send_cmd(priv, HostCmd_CMD_802_11_SNMP_MIB,
				HostCmd_ACT_GEN_SET, FRAG_THRESH_I,
				&frag_thr, true);
}

/*
 * This function sets the RTS threshold.

 * The rts value must lie between MWIFIEX_RTS_MIN_VALUE
 * and MWIFIEX_RTS_MAX_VALUE.
 */
static int
mwifiex_set_rts(struct mwifiex_private *priv, u32 rts_thr)
{
	if (rts_thr < MWIFIEX_RTS_MIN_VALUE || rts_thr > MWIFIEX_RTS_MAX_VALUE)
		rts_thr = MWIFIEX_RTS_MAX_VALUE;

	return mwifiex_send_cmd(priv, HostCmd_CMD_802_11_SNMP_MIB,
				HostCmd_ACT_GEN_SET, RTS_THRESH_I,
				&rts_thr, true);
}

/*
 * CFG802.11 operation handler to set wiphy parameters.
 *
 * This function can be used to set the RTS threshold and the
 * Fragmentation threshold of the driver.
 */
static int
mwifiex_cfg80211_set_wiphy_params(struct wiphy *wiphy, u32 changed)
{
	struct mwifiex_adapter *adapter = mwifiex_cfg80211_get_adapter(wiphy);
	struct mwifiex_private *priv;
	struct mwifiex_uap_bss_param *bss_cfg;
	int ret;

	priv = mwifiex_get_priv(adapter, MWIFIEX_BSS_ROLE_ANY);

	switch (priv->bss_role) {
	case MWIFIEX_BSS_ROLE_UAP:
		if (priv->bss_started) {
			dev_err(adapter->dev,
				"cannot change wiphy params when bss started");
			return -EINVAL;
		}

		bss_cfg = kzalloc(sizeof(*bss_cfg), GFP_KERNEL);
		if (!bss_cfg)
			return -ENOMEM;

		mwifiex_set_sys_config_invalid_data(bss_cfg);

		if (changed & WIPHY_PARAM_RTS_THRESHOLD)
			bss_cfg->rts_threshold = wiphy->rts_threshold;
		if (changed & WIPHY_PARAM_FRAG_THRESHOLD)
			bss_cfg->frag_threshold = wiphy->frag_threshold;
		if (changed & WIPHY_PARAM_RETRY_LONG)
			bss_cfg->retry_limit = wiphy->retry_long;

		ret = mwifiex_send_cmd(priv, HostCmd_CMD_UAP_SYS_CONFIG,
				       HostCmd_ACT_GEN_SET,
				       UAP_BSS_PARAMS_I, bss_cfg,
				       false);

		kfree(bss_cfg);
		if (ret) {
			wiphy_err(wiphy, "Failed to set wiphy phy params\n");
			return ret;
		}
		break;

		case MWIFIEX_BSS_ROLE_STA:
		if (priv->media_connected) {
			dev_err(adapter->dev,
				"cannot change wiphy params when connected");
			return -EINVAL;
		}
		if (changed & WIPHY_PARAM_RTS_THRESHOLD) {
			ret = mwifiex_set_rts(priv,
					      wiphy->rts_threshold);
			if (ret)
				return ret;
		}
		if (changed & WIPHY_PARAM_FRAG_THRESHOLD) {
			ret = mwifiex_set_frag(priv,
					       wiphy->frag_threshold);
			if (ret)
				return ret;
		}
		break;
	}

	return 0;
}

static int
mwifiex_cfg80211_deinit_p2p(struct mwifiex_private *priv)
{
	u16 mode = P2P_MODE_DISABLE;

	if (mwifiex_send_cmd(priv, HostCmd_CMD_P2P_MODE_CFG,
			     HostCmd_ACT_GEN_SET, 0, &mode, true))
		return -1;

	return 0;
}

/*
 * This function initializes the functionalities for P2P client.
 * The P2P client initialization sequence is:
 * disable -> device -> client
 */
static int
mwifiex_cfg80211_init_p2p_client(struct mwifiex_private *priv)
{
	u16 mode;

	if (mwifiex_cfg80211_deinit_p2p(priv))
		return -1;

	mode = P2P_MODE_DEVICE;
	if (mwifiex_send_cmd(priv, HostCmd_CMD_P2P_MODE_CFG,
			     HostCmd_ACT_GEN_SET, 0, &mode, true))
		return -1;

	mode = P2P_MODE_CLIENT;
	if (mwifiex_send_cmd(priv, HostCmd_CMD_P2P_MODE_CFG,
			     HostCmd_ACT_GEN_SET, 0, &mode, true))
		return -1;

	return 0;
}

/*
 * This function initializes the functionalities for P2P GO.
 * The P2P GO initialization sequence is:
 * disable -> device -> GO
 */
static int
mwifiex_cfg80211_init_p2p_go(struct mwifiex_private *priv)
{
	u16 mode;

	if (mwifiex_cfg80211_deinit_p2p(priv))
		return -1;

	mode = P2P_MODE_DEVICE;
	if (mwifiex_send_cmd(priv, HostCmd_CMD_P2P_MODE_CFG,
			     HostCmd_ACT_GEN_SET, 0, &mode, true))
		return -1;

	mode = P2P_MODE_GO;
	if (mwifiex_send_cmd(priv, HostCmd_CMD_P2P_MODE_CFG,
			     HostCmd_ACT_GEN_SET, 0, &mode, true))
		return -1;

	return 0;
}

static int mwifiex_deinit_priv_params(struct mwifiex_private *priv)
{
	struct mwifiex_adapter *adapter = priv->adapter;
	unsigned long flags;

	priv->mgmt_frame_mask = 0;
	if (mwifiex_send_cmd(priv, HostCmd_CMD_MGMT_FRAME_REG,
			     HostCmd_ACT_GEN_SET, 0,
			     &priv->mgmt_frame_mask, false)) {
		dev_warn(priv->adapter->dev,
			 "could not unregister mgmt frame rx\n");
		return -1;
	}

	mwifiex_deauthenticate(priv, NULL);

	spin_lock_irqsave(&adapter->main_proc_lock, flags);
	adapter->main_locked = true;
	if (adapter->mwifiex_processing) {
		spin_unlock_irqrestore(&adapter->main_proc_lock, flags);
		flush_workqueue(adapter->workqueue);
	} else {
		spin_unlock_irqrestore(&adapter->main_proc_lock, flags);
	}

	spin_lock_irqsave(&adapter->rx_proc_lock, flags);
	adapter->rx_locked = true;
	if (adapter->rx_processing) {
		spin_unlock_irqrestore(&adapter->rx_proc_lock, flags);
		flush_workqueue(adapter->rx_workqueue);
	} else {
	spin_unlock_irqrestore(&adapter->rx_proc_lock, flags);
	}

	mwifiex_free_priv(priv);
	priv->wdev.iftype = NL80211_IFTYPE_UNSPECIFIED;
	priv->bss_mode = NL80211_IFTYPE_UNSPECIFIED;
	priv->sec_info.authentication_mode = NL80211_AUTHTYPE_OPEN_SYSTEM;

	return 0;
}

static int
mwifiex_init_new_priv_params(struct mwifiex_private *priv,
			     struct net_device *dev,
			     enum nl80211_iftype type)
{
	struct mwifiex_adapter *adapter = priv->adapter;
	unsigned long flags;

	mwifiex_init_priv(priv);

	priv->bss_mode = type;
	priv->wdev.iftype = type;

	mwifiex_init_priv_params(priv, priv->netdev);
	priv->bss_started = 0;

	switch (type) {
	case NL80211_IFTYPE_STATION:
	case NL80211_IFTYPE_ADHOC:
		priv->bss_role =  MWIFIEX_BSS_ROLE_STA;
		priv->bss_type = MWIFIEX_BSS_TYPE_STA;
		break;
	case NL80211_IFTYPE_P2P_CLIENT:
	case NL80211_IFTYPE_P2P_GO:
		priv->bss_role =  MWIFIEX_BSS_ROLE_STA;
		priv->bss_type = MWIFIEX_BSS_TYPE_P2P;
		break;
	case NL80211_IFTYPE_AP:
		priv->bss_type = MWIFIEX_BSS_TYPE_UAP;
		priv->bss_role = MWIFIEX_BSS_ROLE_UAP;
		break;
	default:
		dev_err(priv->adapter->dev,
			"%s: changing to %d not supported\n",
			dev->name, type);
		return -EOPNOTSUPP;
	}

	spin_lock_irqsave(&adapter->main_proc_lock, flags);
	adapter->main_locked = false;
	spin_unlock_irqrestore(&adapter->main_proc_lock, flags);

	spin_lock_irqsave(&adapter->rx_proc_lock, flags);
	adapter->rx_locked = false;
	spin_unlock_irqrestore(&adapter->rx_proc_lock, flags);

	return 0;
}

static int
mwifiex_change_vif_to_p2p(struct net_device *dev,
			  enum nl80211_iftype curr_iftype,
			  enum nl80211_iftype type, u32 *flags,
			  struct vif_params *params)
{
	struct mwifiex_private *priv;
	struct mwifiex_adapter *adapter;

	priv = mwifiex_netdev_get_priv(dev);

	if (!priv)
		return -1;

	adapter = priv->adapter;

	if (adapter->curr_iface_comb.p2p_intf ==
	    adapter->iface_limit.p2p_intf) {
		dev_err(adapter->dev,
			"cannot create multiple P2P ifaces\n");
		return -1;
	}

	dev_dbg(priv->adapter->dev, "%s: changing role to p2p\n", dev->name);

	if (mwifiex_deinit_priv_params(priv))
		return -1;
	if (mwifiex_init_new_priv_params(priv, dev, type))
		return -1;

	switch (type) {
	case NL80211_IFTYPE_P2P_CLIENT:
		if (mwifiex_cfg80211_init_p2p_client(priv))
			return -EFAULT;
		break;
	case NL80211_IFTYPE_P2P_GO:
		if (mwifiex_cfg80211_init_p2p_go(priv))
			return -EFAULT;
		break;
	default:
		dev_err(priv->adapter->dev,
			"%s: changing to %d not supported\n",
			dev->name, type);
		return -EOPNOTSUPP;
	}

	if (mwifiex_send_cmd(priv, HostCmd_CMD_SET_BSS_MODE,
			     HostCmd_ACT_GEN_SET, 0, NULL, true))
		return -1;

	if (mwifiex_sta_init_cmd(priv, false, false))
		return -1;

	switch (curr_iftype) {
	case NL80211_IFTYPE_STATION:
	case NL80211_IFTYPE_ADHOC:
		adapter->curr_iface_comb.sta_intf--;
		break;
	case NL80211_IFTYPE_AP:
		adapter->curr_iface_comb.uap_intf--;
		break;
	default:
		break;
	}

	adapter->curr_iface_comb.p2p_intf++;
	dev->ieee80211_ptr->iftype = type;

	return 0;
}

static int
mwifiex_change_vif_to_sta_adhoc(struct net_device *dev,
				enum nl80211_iftype curr_iftype,
				enum nl80211_iftype type, u32 *flags,
				struct vif_params *params)
{
	struct mwifiex_private *priv;
	struct mwifiex_adapter *adapter;

	priv = mwifiex_netdev_get_priv(dev);

	if (!priv)
		return -1;

	adapter = priv->adapter;

	if ((curr_iftype != NL80211_IFTYPE_P2P_CLIENT &&
	     curr_iftype != NL80211_IFTYPE_P2P_GO) &&
	    (adapter->curr_iface_comb.sta_intf ==
	     adapter->iface_limit.sta_intf)) {
		dev_err(adapter->dev,
			"cannot create multiple station/adhoc ifaces\n");
		return -1;
	}

	if (type == NL80211_IFTYPE_STATION)
		dev_notice(adapter->dev,
			   "%s: changing role to station\n", dev->name);
	else
		dev_notice(adapter->dev,
			   "%s: changing role to adhoc\n", dev->name);

	if (mwifiex_deinit_priv_params(priv))
		return -1;
	if (mwifiex_init_new_priv_params(priv, dev, type))
		return -1;
	if (mwifiex_send_cmd(priv, HostCmd_CMD_SET_BSS_MODE,
			     HostCmd_ACT_GEN_SET, 0, NULL, true))
		return -1;
	if (mwifiex_sta_init_cmd(priv, false, false))
		return -1;

	switch (curr_iftype) {
	case NL80211_IFTYPE_P2P_CLIENT:
	case NL80211_IFTYPE_P2P_GO:
		adapter->curr_iface_comb.p2p_intf--;
		break;
	case NL80211_IFTYPE_AP:
		adapter->curr_iface_comb.uap_intf--;
		break;
	default:
		break;
	}

	adapter->curr_iface_comb.sta_intf++;
	dev->ieee80211_ptr->iftype = type;
	return 0;
}

static int
mwifiex_change_vif_to_ap(struct net_device *dev,
			 enum nl80211_iftype curr_iftype,
			 enum nl80211_iftype type, u32 *flags,
			 struct vif_params *params)
{
	struct mwifiex_private *priv;
	struct mwifiex_adapter *adapter;

	priv = mwifiex_netdev_get_priv(dev);

	if (!priv)
		return -1;

	adapter = priv->adapter;

	if (adapter->curr_iface_comb.uap_intf ==
	    adapter->iface_limit.uap_intf) {
		dev_err(adapter->dev,
			"cannot create multiple AP ifaces\n");
		return -1;
	}

	dev_notice(adapter->dev, "%s: changing role to AP\n", dev->name);

	if (mwifiex_deinit_priv_params(priv))
		return -1;
	if (mwifiex_init_new_priv_params(priv, dev, type))
		return -1;
	if (mwifiex_send_cmd(priv, HostCmd_CMD_SET_BSS_MODE,
			     HostCmd_ACT_GEN_SET, 0, NULL, true))
		return -1;
	if (mwifiex_sta_init_cmd(priv, false, false))
		return -1;

	switch (curr_iftype) {
	case NL80211_IFTYPE_P2P_CLIENT:
	case NL80211_IFTYPE_P2P_GO:
		adapter->curr_iface_comb.p2p_intf--;
		break;
	case NL80211_IFTYPE_STATION:
	case NL80211_IFTYPE_ADHOC:
		adapter->curr_iface_comb.sta_intf--;
		break;
	default:
		break;
	}

	adapter->curr_iface_comb.uap_intf++;
	dev->ieee80211_ptr->iftype = type;
	return 0;
}
/*
 * CFG802.11 operation handler to change interface type.
 */
static int
mwifiex_cfg80211_change_virtual_intf(struct wiphy *wiphy,
				     struct net_device *dev,
				     enum nl80211_iftype type, u32 *flags,
				     struct vif_params *params)
{
	struct mwifiex_private *priv = mwifiex_netdev_get_priv(dev);
	enum nl80211_iftype curr_iftype = dev->ieee80211_ptr->iftype;

	switch (curr_iftype) {
	case NL80211_IFTYPE_ADHOC:
		switch (type) {
		case NL80211_IFTYPE_STATION:
			priv->bss_mode = type;
			priv->sec_info.authentication_mode =
						   NL80211_AUTHTYPE_OPEN_SYSTEM;
			dev->ieee80211_ptr->iftype = type;
			mwifiex_deauthenticate(priv, NULL);
			return mwifiex_send_cmd(priv, HostCmd_CMD_SET_BSS_MODE,
						HostCmd_ACT_GEN_SET, 0, NULL,
						true);
		case NL80211_IFTYPE_P2P_CLIENT:
		case NL80211_IFTYPE_P2P_GO:
			return mwifiex_change_vif_to_p2p(dev, curr_iftype,
							 type, flags, params);
		case NL80211_IFTYPE_AP:
			return mwifiex_change_vif_to_ap(dev, curr_iftype, type,
							flags, params);
		case NL80211_IFTYPE_UNSPECIFIED:
			wiphy_warn(wiphy, "%s: kept type as IBSS\n", dev->name);
		case NL80211_IFTYPE_ADHOC:	/* This shouldn't happen */
			return 0;
		default:
			wiphy_err(wiphy, "%s: changing to %d not supported\n",
				  dev->name, type);
			return -EOPNOTSUPP;
		}
		break;
	case NL80211_IFTYPE_STATION:
		switch (type) {
		case NL80211_IFTYPE_ADHOC:
			priv->bss_mode = type;
			priv->sec_info.authentication_mode =
						   NL80211_AUTHTYPE_OPEN_SYSTEM;
			dev->ieee80211_ptr->iftype = type;
			mwifiex_deauthenticate(priv, NULL);
			return mwifiex_send_cmd(priv, HostCmd_CMD_SET_BSS_MODE,
						HostCmd_ACT_GEN_SET, 0, NULL,
						true);
		case NL80211_IFTYPE_P2P_CLIENT:
		case NL80211_IFTYPE_P2P_GO:
			return mwifiex_change_vif_to_p2p(dev, curr_iftype,
							 type, flags, params);
		case NL80211_IFTYPE_AP:
			return mwifiex_change_vif_to_ap(dev, curr_iftype, type,
							flags, params);
		case NL80211_IFTYPE_UNSPECIFIED:
			wiphy_warn(wiphy, "%s: kept type as STA\n", dev->name);
		case NL80211_IFTYPE_STATION:	/* This shouldn't happen */
			return 0;
		default:
			wiphy_err(wiphy, "%s: changing to %d not supported\n",
				  dev->name, type);
			return -EOPNOTSUPP;
		}
		break;
	case NL80211_IFTYPE_AP:
		switch (type) {
		case NL80211_IFTYPE_ADHOC:
		case NL80211_IFTYPE_STATION:
			return mwifiex_change_vif_to_sta_adhoc(dev, curr_iftype,
							       type, flags,
							       params);
			break;
		case NL80211_IFTYPE_P2P_CLIENT:
		case NL80211_IFTYPE_P2P_GO:
			return mwifiex_change_vif_to_p2p(dev, curr_iftype,
							 type, flags, params);
		case NL80211_IFTYPE_UNSPECIFIED:
			wiphy_warn(wiphy, "%s: kept type as AP\n", dev->name);
		case NL80211_IFTYPE_AP:		/* This shouldn't happen */
			return 0;
		default:
			wiphy_err(wiphy, "%s: changing to %d not supported\n",
				  dev->name, type);
			return -EOPNOTSUPP;
		}
		break;
	case NL80211_IFTYPE_P2P_CLIENT:
	case NL80211_IFTYPE_P2P_GO:
		switch (type) {
		case NL80211_IFTYPE_STATION:
			if (mwifiex_cfg80211_init_p2p_client(priv))
				return -EFAULT;
			dev->ieee80211_ptr->iftype = type;
			break;
		case NL80211_IFTYPE_ADHOC:
			if (mwifiex_cfg80211_deinit_p2p(priv))
				return -EFAULT;
			return mwifiex_change_vif_to_sta_adhoc(dev, curr_iftype,
							       type, flags,
							       params);
			break;
		case NL80211_IFTYPE_AP:
			if (mwifiex_cfg80211_deinit_p2p(priv))
				return -EFAULT;
			return mwifiex_change_vif_to_ap(dev, curr_iftype, type,
							flags, params);
		case NL80211_IFTYPE_UNSPECIFIED:
			wiphy_warn(wiphy, "%s: kept type as P2P\n", dev->name);
		case NL80211_IFTYPE_P2P_CLIENT:
		case NL80211_IFTYPE_P2P_GO:
			return 0;
		default:
			wiphy_err(wiphy, "%s: changing to %d not supported\n",
				  dev->name, type);
			return -EOPNOTSUPP;
		}
		break;
	default:
		wiphy_err(wiphy, "%s: unknown iftype: %d\n",
			  dev->name, dev->ieee80211_ptr->iftype);
		return -EOPNOTSUPP;
	}


	return 0;
}

static void
mwifiex_parse_htinfo(struct mwifiex_private *priv, u8 tx_htinfo,
		     struct rate_info *rate)
{
	struct mwifiex_adapter *adapter = priv->adapter;

	if (adapter->is_hw_11ac_capable) {
		/* bit[1-0]: 00=LG 01=HT 10=VHT */
		if (tx_htinfo & BIT(0)) {
			/* HT */
			rate->mcs = priv->tx_rate;
			rate->flags |= RATE_INFO_FLAGS_MCS;
		}
		if (tx_htinfo & BIT(1)) {
			/* VHT */
			rate->mcs = priv->tx_rate & 0x0F;
			rate->flags |= RATE_INFO_FLAGS_VHT_MCS;
		}

		if (tx_htinfo & (BIT(1) | BIT(0))) {
			/* HT or VHT */
			switch (tx_htinfo & (BIT(3) | BIT(2))) {
			case 0:
				rate->bw = RATE_INFO_BW_20;
				break;
			case (BIT(2)):
				rate->bw = RATE_INFO_BW_40;
				break;
			case (BIT(3)):
				rate->bw = RATE_INFO_BW_80;
				break;
			case (BIT(3) | BIT(2)):
				rate->bw = RATE_INFO_BW_160;
				break;
			}

			if (tx_htinfo & BIT(4))
				rate->flags |= RATE_INFO_FLAGS_SHORT_GI;

			if ((priv->tx_rate >> 4) == 1)
				rate->nss = 2;
			else
				rate->nss = 1;
		}
	} else {
		/*
		 * Bit 0 in tx_htinfo indicates that current Tx rate
		 * is 11n rate. Valid MCS index values for us are 0 to 15.
		 */
		if ((tx_htinfo & BIT(0)) && (priv->tx_rate < 16)) {
			rate->mcs = priv->tx_rate;
			rate->flags |= RATE_INFO_FLAGS_MCS;
			rate->bw = RATE_INFO_BW_20;
			if (tx_htinfo & BIT(1))
				rate->bw = RATE_INFO_BW_40;
			if (tx_htinfo & BIT(2))
				rate->flags |= RATE_INFO_FLAGS_SHORT_GI;
		}
	}
}

/*
 * This function dumps the station information on a buffer.
 *
 * The following information are shown -
 *      - Total bytes transmitted
 *      - Total bytes received
 *      - Total packets transmitted
 *      - Total packets received
 *      - Signal quality level
 *      - Transmission rate
 */
static int
mwifiex_dump_station_info(struct mwifiex_private *priv,
			  struct station_info *sinfo)
{
	u32 rate;

	sinfo->filled = BIT(NL80211_STA_INFO_RX_BYTES) | BIT(NL80211_STA_INFO_TX_BYTES) |
			BIT(NL80211_STA_INFO_RX_PACKETS) | BIT(NL80211_STA_INFO_TX_PACKETS) |
			BIT(NL80211_STA_INFO_TX_BITRATE) |
			BIT(NL80211_STA_INFO_SIGNAL) | BIT(NL80211_STA_INFO_SIGNAL_AVG);

	/* Get signal information from the firmware */
	if (mwifiex_send_cmd(priv, HostCmd_CMD_RSSI_INFO,
			     HostCmd_ACT_GEN_GET, 0, NULL, true)) {
		dev_err(priv->adapter->dev, "failed to get signal information\n");
		return -EFAULT;
	}

	if (mwifiex_drv_get_data_rate(priv, &rate)) {
		dev_err(priv->adapter->dev, "getting data rate\n");
		return -EFAULT;
	}

	/* Get DTIM period information from firmware */
	mwifiex_send_cmd(priv, HostCmd_CMD_802_11_SNMP_MIB,
			 HostCmd_ACT_GEN_GET, DTIM_PERIOD_I,
			 &priv->dtim_period, true);

	mwifiex_parse_htinfo(priv, priv->tx_htinfo, &sinfo->txrate);

	sinfo->signal_avg = priv->bcn_rssi_avg;
	sinfo->rx_bytes = priv->stats.rx_bytes;
	sinfo->tx_bytes = priv->stats.tx_bytes;
	sinfo->rx_packets = priv->stats.rx_packets;
	sinfo->tx_packets = priv->stats.tx_packets;
	sinfo->signal = priv->bcn_rssi_avg;
	/* bit rate is in 500 kb/s units. Convert it to 100kb/s units */
	sinfo->txrate.legacy = rate * 5;

	if (priv->bss_mode == NL80211_IFTYPE_STATION) {
		sinfo->filled |= BIT(NL80211_STA_INFO_BSS_PARAM);
		sinfo->bss_param.flags = 0;
		if (priv->curr_bss_params.bss_descriptor.cap_info_bitmap &
						WLAN_CAPABILITY_SHORT_PREAMBLE)
			sinfo->bss_param.flags |=
					BSS_PARAM_FLAGS_SHORT_PREAMBLE;
		if (priv->curr_bss_params.bss_descriptor.cap_info_bitmap &
						WLAN_CAPABILITY_SHORT_SLOT_TIME)
			sinfo->bss_param.flags |=
					BSS_PARAM_FLAGS_SHORT_SLOT_TIME;
		sinfo->bss_param.dtim_period = priv->dtim_period;
		sinfo->bss_param.beacon_interval =
			priv->curr_bss_params.bss_descriptor.beacon_period;
	}

	return 0;
}

/*
 * CFG802.11 operation handler to get station information.
 *
 * This function only works in connected mode, and dumps the
 * requested station information, if available.
 */
static int
mwifiex_cfg80211_get_station(struct wiphy *wiphy, struct net_device *dev,
			     const u8 *mac, struct station_info *sinfo)
{
	struct mwifiex_private *priv = mwifiex_netdev_get_priv(dev);

	if (!priv->media_connected)
		return -ENOENT;
	if (memcmp(mac, priv->cfg_bssid, ETH_ALEN))
		return -ENOENT;

	return mwifiex_dump_station_info(priv, sinfo);
}

/*
 * CFG802.11 operation handler to dump station information.
 */
static int
mwifiex_cfg80211_dump_station(struct wiphy *wiphy, struct net_device *dev,
			      int idx, u8 *mac, struct station_info *sinfo)
{
	struct mwifiex_private *priv = mwifiex_netdev_get_priv(dev);

	if (!priv->media_connected || idx)
		return -ENOENT;

	memcpy(mac, priv->cfg_bssid, ETH_ALEN);

	return mwifiex_dump_station_info(priv, sinfo);
}

static int
mwifiex_cfg80211_dump_survey(struct wiphy *wiphy, struct net_device *dev,
			     int idx, struct survey_info *survey)
{
	struct mwifiex_private *priv = mwifiex_netdev_get_priv(dev);
	struct mwifiex_chan_stats *pchan_stats = priv->adapter->chan_stats;
	enum ieee80211_band band;

	dev_dbg(priv->adapter->dev, "dump_survey idx=%d\n", idx);

	memset(survey, 0, sizeof(struct survey_info));

	if ((GET_BSS_ROLE(priv) == MWIFIEX_BSS_ROLE_STA) &&
	    priv->media_connected && idx == 0) {
			u8 curr_bss_band = priv->curr_bss_params.band;
			u32 chan = priv->curr_bss_params.bss_descriptor.channel;

			band = mwifiex_band_to_radio_type(curr_bss_band);
			survey->channel = ieee80211_get_channel(wiphy,
				ieee80211_channel_to_frequency(chan, band));

			if (priv->bcn_nf_last) {
				survey->filled = SURVEY_INFO_NOISE_DBM;
				survey->noise = priv->bcn_nf_last;
			}
			return 0;
	}

	if (idx >= priv->adapter->num_in_chan_stats)
		return -ENOENT;

	if (!pchan_stats[idx].cca_scan_dur)
		return 0;

	band = pchan_stats[idx].bandcfg;
	survey->channel = ieee80211_get_channel(wiphy,
	    ieee80211_channel_to_frequency(pchan_stats[idx].chan_num, band));
	survey->filled = SURVEY_INFO_NOISE_DBM |
			 SURVEY_INFO_TIME |
			 SURVEY_INFO_TIME_BUSY;
	survey->noise = pchan_stats[idx].noise;
	survey->time = pchan_stats[idx].cca_scan_dur;
	survey->time_busy = pchan_stats[idx].cca_busy_dur;

	return 0;
}

/* Supported rates to be advertised to the cfg80211 */
static struct ieee80211_rate mwifiex_rates[] = {
	{.bitrate = 10, .hw_value = 2, },
	{.bitrate = 20, .hw_value = 4, },
	{.bitrate = 55, .hw_value = 11, },
	{.bitrate = 110, .hw_value = 22, },
	{.bitrate = 60, .hw_value = 12, },
	{.bitrate = 90, .hw_value = 18, },
	{.bitrate = 120, .hw_value = 24, },
	{.bitrate = 180, .hw_value = 36, },
	{.bitrate = 240, .hw_value = 48, },
	{.bitrate = 360, .hw_value = 72, },
	{.bitrate = 480, .hw_value = 96, },
	{.bitrate = 540, .hw_value = 108, },
};

/* Channel definitions to be advertised to cfg80211 */
static struct ieee80211_channel mwifiex_channels_2ghz[] = {
	{.center_freq = 2412, .hw_value = 1, },
	{.center_freq = 2417, .hw_value = 2, },
	{.center_freq = 2422, .hw_value = 3, },
	{.center_freq = 2427, .hw_value = 4, },
	{.center_freq = 2432, .hw_value = 5, },
	{.center_freq = 2437, .hw_value = 6, },
	{.center_freq = 2442, .hw_value = 7, },
	{.center_freq = 2447, .hw_value = 8, },
	{.center_freq = 2452, .hw_value = 9, },
	{.center_freq = 2457, .hw_value = 10, },
	{.center_freq = 2462, .hw_value = 11, },
	{.center_freq = 2467, .hw_value = 12, },
	{.center_freq = 2472, .hw_value = 13, },
	{.center_freq = 2484, .hw_value = 14, },
};

static struct ieee80211_supported_band mwifiex_band_2ghz = {
	.channels = mwifiex_channels_2ghz,
	.n_channels = ARRAY_SIZE(mwifiex_channels_2ghz),
	.bitrates = mwifiex_rates,
	.n_bitrates = ARRAY_SIZE(mwifiex_rates),
};

static struct ieee80211_channel mwifiex_channels_5ghz[] = {
	{.center_freq = 5040, .hw_value = 8, },
	{.center_freq = 5060, .hw_value = 12, },
	{.center_freq = 5080, .hw_value = 16, },
	{.center_freq = 5170, .hw_value = 34, },
	{.center_freq = 5190, .hw_value = 38, },
	{.center_freq = 5210, .hw_value = 42, },
	{.center_freq = 5230, .hw_value = 46, },
	{.center_freq = 5180, .hw_value = 36, },
	{.center_freq = 5200, .hw_value = 40, },
	{.center_freq = 5220, .hw_value = 44, },
	{.center_freq = 5240, .hw_value = 48, },
	{.center_freq = 5260, .hw_value = 52, },
	{.center_freq = 5280, .hw_value = 56, },
	{.center_freq = 5300, .hw_value = 60, },
	{.center_freq = 5320, .hw_value = 64, },
	{.center_freq = 5500, .hw_value = 100, },
	{.center_freq = 5520, .hw_value = 104, },
	{.center_freq = 5540, .hw_value = 108, },
	{.center_freq = 5560, .hw_value = 112, },
	{.center_freq = 5580, .hw_value = 116, },
	{.center_freq = 5600, .hw_value = 120, },
	{.center_freq = 5620, .hw_value = 124, },
	{.center_freq = 5640, .hw_value = 128, },
	{.center_freq = 5660, .hw_value = 132, },
	{.center_freq = 5680, .hw_value = 136, },
	{.center_freq = 5700, .hw_value = 140, },
	{.center_freq = 5745, .hw_value = 149, },
	{.center_freq = 5765, .hw_value = 153, },
	{.center_freq = 5785, .hw_value = 157, },
	{.center_freq = 5805, .hw_value = 161, },
	{.center_freq = 5825, .hw_value = 165, },
};

static struct ieee80211_supported_band mwifiex_band_5ghz = {
	.channels = mwifiex_channels_5ghz,
	.n_channels = ARRAY_SIZE(mwifiex_channels_5ghz),
	.bitrates = mwifiex_rates + 4,
	.n_bitrates = ARRAY_SIZE(mwifiex_rates) - 4,
};


/* Supported crypto cipher suits to be advertised to cfg80211 */
static const u32 mwifiex_cipher_suites[] = {
	WLAN_CIPHER_SUITE_WEP40,
	WLAN_CIPHER_SUITE_WEP104,
	WLAN_CIPHER_SUITE_TKIP,
	WLAN_CIPHER_SUITE_CCMP,
	WLAN_CIPHER_SUITE_AES_CMAC,
};

/* Supported mgmt frame types to be advertised to cfg80211 */
static const struct ieee80211_txrx_stypes
mwifiex_mgmt_stypes[NUM_NL80211_IFTYPES] = {
	[NL80211_IFTYPE_STATION] = {
		.tx = BIT(IEEE80211_STYPE_ACTION >> 4) |
		      BIT(IEEE80211_STYPE_PROBE_RESP >> 4),
		.rx = BIT(IEEE80211_STYPE_ACTION >> 4) |
		      BIT(IEEE80211_STYPE_PROBE_REQ >> 4),
	},
	[NL80211_IFTYPE_AP] = {
		.tx = BIT(IEEE80211_STYPE_ACTION >> 4) |
		      BIT(IEEE80211_STYPE_PROBE_RESP >> 4),
		.rx = BIT(IEEE80211_STYPE_ACTION >> 4) |
		      BIT(IEEE80211_STYPE_PROBE_REQ >> 4),
	},
	[NL80211_IFTYPE_P2P_CLIENT] = {
		.tx = BIT(IEEE80211_STYPE_ACTION >> 4) |
		      BIT(IEEE80211_STYPE_PROBE_RESP >> 4),
		.rx = BIT(IEEE80211_STYPE_ACTION >> 4) |
		      BIT(IEEE80211_STYPE_PROBE_REQ >> 4),
	},
	[NL80211_IFTYPE_P2P_GO] = {
		.tx = BIT(IEEE80211_STYPE_ACTION >> 4) |
		      BIT(IEEE80211_STYPE_PROBE_RESP >> 4),
		.rx = BIT(IEEE80211_STYPE_ACTION >> 4) |
		      BIT(IEEE80211_STYPE_PROBE_REQ >> 4),
	},
};

/*
 * CFG802.11 operation handler for setting bit rates.
 *
 * Function configures data rates to firmware using bitrate mask
 * provided by cfg80211.
 */
static int mwifiex_cfg80211_set_bitrate_mask(struct wiphy *wiphy,
				struct net_device *dev,
				const u8 *peer,
				const struct cfg80211_bitrate_mask *mask)
{
	struct mwifiex_private *priv = mwifiex_netdev_get_priv(dev);
	u16 bitmap_rates[MAX_BITMAP_RATES_SIZE];
	enum ieee80211_band band;
	struct mwifiex_adapter *adapter = priv->adapter;

	if (!priv->media_connected) {
		dev_err(adapter->dev,
			"Can not set Tx data rate in disconnected state\n");
		return -EINVAL;
	}

	band = mwifiex_band_to_radio_type(priv->curr_bss_params.band);

	memset(bitmap_rates, 0, sizeof(bitmap_rates));

	/* Fill HR/DSSS rates. */
	if (band == IEEE80211_BAND_2GHZ)
		bitmap_rates[0] = mask->control[band].legacy & 0x000f;

	/* Fill OFDM rates */
	if (band == IEEE80211_BAND_2GHZ)
		bitmap_rates[1] = (mask->control[band].legacy & 0x0ff0) >> 4;
	else
		bitmap_rates[1] = mask->control[band].legacy;

	/* Fill HT MCS rates */
	bitmap_rates[2] = mask->control[band].ht_mcs[0];
	if (adapter->hw_dev_mcs_support == HT_STREAM_2X2)
		bitmap_rates[2] |= mask->control[band].ht_mcs[1] << 8;

       /* Fill VHT MCS rates */
	if (adapter->fw_api_ver == MWIFIEX_FW_V15) {
		bitmap_rates[10] = mask->control[band].vht_mcs[0];
		if (adapter->hw_dev_mcs_support == HT_STREAM_2X2)
			bitmap_rates[11] = mask->control[band].vht_mcs[1];
	}

	return mwifiex_send_cmd(priv, HostCmd_CMD_TX_RATE_CFG,
				HostCmd_ACT_GEN_SET, 0, bitmap_rates, true);
}

/*
 * CFG802.11 operation handler for connection quality monitoring.
 *
 * This function subscribes/unsubscribes HIGH_RSSI and LOW_RSSI
 * events to FW.
 */
static int mwifiex_cfg80211_set_cqm_rssi_config(struct wiphy *wiphy,
						struct net_device *dev,
						s32 rssi_thold, u32 rssi_hyst)
{
	struct mwifiex_private *priv = mwifiex_netdev_get_priv(dev);
	struct mwifiex_ds_misc_subsc_evt subsc_evt;

	priv->cqm_rssi_thold = rssi_thold;
	priv->cqm_rssi_hyst = rssi_hyst;

	memset(&subsc_evt, 0x00, sizeof(struct mwifiex_ds_misc_subsc_evt));
	subsc_evt.events = BITMASK_BCN_RSSI_LOW | BITMASK_BCN_RSSI_HIGH;

	/* Subscribe/unsubscribe low and high rssi events */
	if (rssi_thold && rssi_hyst) {
		subsc_evt.action = HostCmd_ACT_BITWISE_SET;
		subsc_evt.bcn_l_rssi_cfg.abs_value = abs(rssi_thold);
		subsc_evt.bcn_h_rssi_cfg.abs_value = abs(rssi_thold);
		subsc_evt.bcn_l_rssi_cfg.evt_freq = 1;
		subsc_evt.bcn_h_rssi_cfg.evt_freq = 1;
		return mwifiex_send_cmd(priv,
					HostCmd_CMD_802_11_SUBSCRIBE_EVENT,
					0, 0, &subsc_evt, true);
	} else {
		subsc_evt.action = HostCmd_ACT_BITWISE_CLR;
		return mwifiex_send_cmd(priv,
					HostCmd_CMD_802_11_SUBSCRIBE_EVENT,
					0, 0, &subsc_evt, true);
	}

	return 0;
}

/* cfg80211 operation handler for change_beacon.
 * Function retrieves and sets modified management IEs to FW.
 */
static int mwifiex_cfg80211_change_beacon(struct wiphy *wiphy,
					  struct net_device *dev,
					  struct cfg80211_beacon_data *data)
{
	struct mwifiex_private *priv = mwifiex_netdev_get_priv(dev);

	if (GET_BSS_ROLE(priv) != MWIFIEX_BSS_ROLE_UAP) {
		wiphy_err(wiphy, "%s: bss_type mismatched\n", __func__);
		return -EINVAL;
	}

	if (!priv->bss_started) {
		wiphy_err(wiphy, "%s: bss not started\n", __func__);
		return -EINVAL;
	}

	if (mwifiex_set_mgmt_ies(priv, data)) {
		wiphy_err(wiphy, "%s: setting mgmt ies failed\n", __func__);
		return -EFAULT;
	}

	return 0;
}

/* cfg80211 operation handler for del_station.
 * Function deauthenticates station which value is provided in mac parameter.
 * If mac is NULL/broadcast, all stations in associated station list are
 * deauthenticated. If bss is not started or there are no stations in
 * associated stations list, no action is taken.
 */
static int
mwifiex_cfg80211_del_station(struct wiphy *wiphy, struct net_device *dev,
			     struct station_del_parameters *params)
{
	struct mwifiex_private *priv = mwifiex_netdev_get_priv(dev);
	struct mwifiex_sta_node *sta_node;
	u8 deauth_mac[ETH_ALEN];
	unsigned long flags;

	if (list_empty(&priv->sta_list) || !priv->bss_started)
		return 0;

	if (!params->mac || is_broadcast_ether_addr(params->mac))
		return 0;

	wiphy_dbg(wiphy, "%s: mac address %pM\n", __func__, params->mac);

	eth_zero_addr(deauth_mac);

	spin_lock_irqsave(&priv->sta_list_spinlock, flags);
	sta_node = mwifiex_get_sta_entry(priv, params->mac);
	if (sta_node)
		ether_addr_copy(deauth_mac, params->mac);
	spin_unlock_irqrestore(&priv->sta_list_spinlock, flags);

	if (is_valid_ether_addr(deauth_mac)) {
		if (mwifiex_send_cmd(priv, HostCmd_CMD_UAP_STA_DEAUTH,
				     HostCmd_ACT_GEN_SET, 0,
				     deauth_mac, true))
			return -1;
	}

	return 0;
}

static int
mwifiex_cfg80211_set_antenna(struct wiphy *wiphy, u32 tx_ant, u32 rx_ant)
{
	struct mwifiex_adapter *adapter = mwifiex_cfg80211_get_adapter(wiphy);
	struct mwifiex_private *priv = mwifiex_get_priv(adapter,
							MWIFIEX_BSS_ROLE_ANY);
	struct mwifiex_ds_ant_cfg ant_cfg;

	if (!tx_ant || !rx_ant)
		return -EOPNOTSUPP;

	if (adapter->hw_dev_mcs_support != HT_STREAM_2X2) {
		/* Not a MIMO chip. User should provide specific antenna number
		 * for Tx/Rx path or enable all antennas for diversity
		 */
		if (tx_ant != rx_ant)
			return -EOPNOTSUPP;

		if ((tx_ant & (tx_ant - 1)) &&
		    (tx_ant != BIT(adapter->number_of_antenna) - 1))
			return -EOPNOTSUPP;

		if ((tx_ant == BIT(adapter->number_of_antenna) - 1) &&
		    (priv->adapter->number_of_antenna > 1)) {
			tx_ant = RF_ANTENNA_AUTO;
			rx_ant = RF_ANTENNA_AUTO;
		}
	} else {
		struct ieee80211_sta_ht_cap *ht_info;
		int rx_mcs_supp;
		enum ieee80211_band band;

		if ((tx_ant == 0x1 && rx_ant == 0x1)) {
			adapter->user_dev_mcs_support = HT_STREAM_1X1;
			if (adapter->is_hw_11ac_capable)
				adapter->usr_dot_11ac_mcs_support =
						MWIFIEX_11AC_MCS_MAP_1X1;
		} else {
			adapter->user_dev_mcs_support = HT_STREAM_2X2;
			if (adapter->is_hw_11ac_capable)
				adapter->usr_dot_11ac_mcs_support =
						MWIFIEX_11AC_MCS_MAP_2X2;
		}

		for (band = 0; band < IEEE80211_NUM_BANDS; band++) {
			if (!adapter->wiphy->bands[band])
				continue;

			ht_info = &adapter->wiphy->bands[band]->ht_cap;
			rx_mcs_supp =
				GET_RXMCSSUPP(adapter->user_dev_mcs_support);
			memset(&ht_info->mcs, 0, adapter->number_of_antenna);
			memset(&ht_info->mcs, 0xff, rx_mcs_supp);
		}
	}

	ant_cfg.tx_ant = tx_ant;
	ant_cfg.rx_ant = rx_ant;

	return mwifiex_send_cmd(priv, HostCmd_CMD_RF_ANTENNA,
				HostCmd_ACT_GEN_SET, 0, &ant_cfg, true);
}

/* cfg80211 operation handler for stop ap.
 * Function stops BSS running at uAP interface.
 */
static int mwifiex_cfg80211_stop_ap(struct wiphy *wiphy, struct net_device *dev)
{
	struct mwifiex_private *priv = mwifiex_netdev_get_priv(dev);

	mwifiex_abort_cac(priv);

	if (mwifiex_del_mgmt_ies(priv))
		wiphy_err(wiphy, "Failed to delete mgmt IEs!\n");

	priv->ap_11n_enabled = 0;
	memset(&priv->bss_cfg, 0, sizeof(priv->bss_cfg));

	if (mwifiex_send_cmd(priv, HostCmd_CMD_UAP_BSS_STOP,
			     HostCmd_ACT_GEN_SET, 0, NULL, true)) {
		wiphy_err(wiphy, "Failed to stop the BSS\n");
		return -1;
	}

	return 0;
}

/* cfg80211 operation handler for start_ap.
 * Function sets beacon period, DTIM period, SSID and security into
 * AP config structure.
 * AP is configured with these settings and BSS is started.
 */
static int mwifiex_cfg80211_start_ap(struct wiphy *wiphy,
				     struct net_device *dev,
				     struct cfg80211_ap_settings *params)
{
	struct mwifiex_uap_bss_param *bss_cfg;
	struct mwifiex_private *priv = mwifiex_netdev_get_priv(dev);

	if (GET_BSS_ROLE(priv) != MWIFIEX_BSS_ROLE_UAP)
		return -1;

	bss_cfg = kzalloc(sizeof(struct mwifiex_uap_bss_param), GFP_KERNEL);
	if (!bss_cfg)
		return -ENOMEM;

	mwifiex_set_sys_config_invalid_data(bss_cfg);

	if (params->beacon_interval)
		bss_cfg->beacon_period = params->beacon_interval;
	if (params->dtim_period)
		bss_cfg->dtim_period = params->dtim_period;

	if (params->ssid && params->ssid_len) {
		memcpy(bss_cfg->ssid.ssid, params->ssid, params->ssid_len);
		bss_cfg->ssid.ssid_len = params->ssid_len;
	}
	if (params->inactivity_timeout > 0) {
		/* sta_ao_timer/ps_sta_ao_timer is in unit of 100ms */
		bss_cfg->sta_ao_timer = 10 * params->inactivity_timeout;
		bss_cfg->ps_sta_ao_timer = 10 * params->inactivity_timeout;
	}

	switch (params->hidden_ssid) {
	case NL80211_HIDDEN_SSID_NOT_IN_USE:
		bss_cfg->bcast_ssid_ctl = 1;
		break;
	case NL80211_HIDDEN_SSID_ZERO_LEN:
		bss_cfg->bcast_ssid_ctl = 0;
		break;
	case NL80211_HIDDEN_SSID_ZERO_CONTENTS:
		/* firmware doesn't support this type of hidden SSID */
	default:
		kfree(bss_cfg);
		return -EINVAL;
	}

	mwifiex_uap_set_channel(bss_cfg, params->chandef);
	mwifiex_set_uap_rates(bss_cfg, params);

	if (mwifiex_set_secure_params(priv, bss_cfg, params)) {
		kfree(bss_cfg);
		wiphy_err(wiphy, "Failed to parse secuirty parameters!\n");
		return -1;
	}

	mwifiex_set_ht_params(priv, bss_cfg, params);

	if (priv->adapter->is_hw_11ac_capable) {
		mwifiex_set_vht_params(priv, bss_cfg, params);
		mwifiex_set_vht_width(priv, params->chandef.width,
				      priv->ap_11ac_enabled);
	}

	if (priv->ap_11ac_enabled)
		mwifiex_set_11ac_ba_params(priv);
	else
		mwifiex_set_ba_params(priv);

	mwifiex_set_wmm_params(priv, bss_cfg, params);

	if (mwifiex_is_11h_active(priv) &&
	    !cfg80211_chandef_dfs_required(wiphy, &params->chandef,
					   priv->bss_mode)) {
		dev_dbg(priv->adapter->dev, "Disable 11h extensions in FW\n");
		if (mwifiex_11h_activate(priv, false)) {
			dev_err(priv->adapter->dev,
				"Failed to disable 11h extensions!!");
			return -1;
		}
		priv->state_11h.is_11h_active = true;
	}

	if (mwifiex_config_start_uap(priv, bss_cfg)) {
		wiphy_err(wiphy, "Failed to start AP\n");
		kfree(bss_cfg);
		return -1;
	}

	if (mwifiex_set_mgmt_ies(priv, &params->beacon))
		return -1;

	memcpy(&priv->bss_cfg, bss_cfg, sizeof(priv->bss_cfg));
	kfree(bss_cfg);
	return 0;
}

/*
 * CFG802.11 operation handler for disconnection request.
 *
 * This function does not work when there is already a disconnection
 * procedure going on.
 */
static int
mwifiex_cfg80211_disconnect(struct wiphy *wiphy, struct net_device *dev,
			    u16 reason_code)
{
	struct mwifiex_private *priv = mwifiex_netdev_get_priv(dev);

	if (mwifiex_deauthenticate(priv, NULL))
		return -EFAULT;

	wiphy_dbg(wiphy, "info: successfully disconnected from %pM:"
		" reason code %d\n", priv->cfg_bssid, reason_code);

	eth_zero_addr(priv->cfg_bssid);
	priv->hs2_enabled = false;

	return 0;
}

/*
 * This function informs the CFG802.11 subsystem of a new IBSS.
 *
 * The following information are sent to the CFG802.11 subsystem
 * to register the new IBSS. If we do not register the new IBSS,
 * a kernel panic will result.
 *      - SSID
 *      - SSID length
 *      - BSSID
 *      - Channel
 */
static int mwifiex_cfg80211_inform_ibss_bss(struct mwifiex_private *priv)
{
	struct ieee80211_channel *chan;
	struct mwifiex_bss_info bss_info;
	struct cfg80211_bss *bss;
	int ie_len;
	u8 ie_buf[IEEE80211_MAX_SSID_LEN + sizeof(struct ieee_types_header)];
	enum ieee80211_band band;

	if (mwifiex_get_bss_info(priv, &bss_info))
		return -1;

	ie_buf[0] = WLAN_EID_SSID;
	ie_buf[1] = bss_info.ssid.ssid_len;

	memcpy(&ie_buf[sizeof(struct ieee_types_header)],
	       &bss_info.ssid.ssid, bss_info.ssid.ssid_len);
	ie_len = ie_buf[1] + sizeof(struct ieee_types_header);

	band = mwifiex_band_to_radio_type(priv->curr_bss_params.band);
	chan = __ieee80211_get_channel(priv->wdev.wiphy,
			ieee80211_channel_to_frequency(bss_info.bss_chan,
						       band));

	bss = cfg80211_inform_bss(priv->wdev.wiphy, chan,
				  CFG80211_BSS_FTYPE_UNKNOWN,
				  bss_info.bssid, 0, WLAN_CAPABILITY_IBSS,
				  0, ie_buf, ie_len, 0, GFP_KERNEL);
	cfg80211_put_bss(priv->wdev.wiphy, bss);
	memcpy(priv->cfg_bssid, bss_info.bssid, ETH_ALEN);

	return 0;
}

/*
 * This function connects with a BSS.
 *
 * This function handles both Infra and Ad-Hoc modes. It also performs
 * validity checking on the provided parameters, disconnects from the
 * current BSS (if any), sets up the association/scan parameters,
 * including security settings, and performs specific SSID scan before
 * trying to connect.
 *
 * For Infra mode, the function returns failure if the specified SSID
 * is not found in scan table. However, for Ad-Hoc mode, it can create
 * the IBSS if it does not exist. On successful completion in either case,
 * the function notifies the CFG802.11 subsystem of the new BSS connection.
 */
static int
mwifiex_cfg80211_assoc(struct mwifiex_private *priv, size_t ssid_len,
		       const u8 *ssid, const u8 *bssid, int mode,
		       struct ieee80211_channel *channel,
		       struct cfg80211_connect_params *sme, bool privacy)
{
	struct cfg80211_ssid req_ssid;
	int ret, auth_type = 0;
	struct cfg80211_bss *bss = NULL;
	u8 is_scanning_required = 0;

	memset(&req_ssid, 0, sizeof(struct cfg80211_ssid));

	req_ssid.ssid_len = ssid_len;
	if (ssid_len > IEEE80211_MAX_SSID_LEN) {
		dev_err(priv->adapter->dev, "invalid SSID - aborting\n");
		return -EINVAL;
	}

	memcpy(req_ssid.ssid, ssid, ssid_len);
	if (!req_ssid.ssid_len || req_ssid.ssid[0] < 0x20) {
		dev_err(priv->adapter->dev, "invalid SSID - aborting\n");
		return -EINVAL;
	}

	/* As this is new association, clear locally stored
	 * keys and security related flags */
	priv->sec_info.wpa_enabled = false;
	priv->sec_info.wpa2_enabled = false;
	priv->wep_key_curr_index = 0;
	priv->sec_info.encryption_mode = 0;
	priv->sec_info.is_authtype_auto = 0;
	ret = mwifiex_set_encode(priv, NULL, NULL, 0, 0, NULL, 1);

	if (mode == NL80211_IFTYPE_ADHOC) {
		/* "privacy" is set only for ad-hoc mode */
		if (privacy) {
			/*
			 * Keep WLAN_CIPHER_SUITE_WEP104 for now so that
			 * the firmware can find a matching network from the
			 * scan. The cfg80211 does not give us the encryption
			 * mode at this stage so just setting it to WEP here.
			 */
			priv->sec_info.encryption_mode =
					WLAN_CIPHER_SUITE_WEP104;
			priv->sec_info.authentication_mode =
					NL80211_AUTHTYPE_OPEN_SYSTEM;
		}

		goto done;
	}

	/* Now handle infra mode. "sme" is valid for infra mode only */
	if (sme->auth_type == NL80211_AUTHTYPE_AUTOMATIC) {
		auth_type = NL80211_AUTHTYPE_OPEN_SYSTEM;
		priv->sec_info.is_authtype_auto = 1;
	} else {
		auth_type = sme->auth_type;
	}

	if (sme->crypto.n_ciphers_pairwise) {
		priv->sec_info.encryption_mode =
						sme->crypto.ciphers_pairwise[0];
		priv->sec_info.authentication_mode = auth_type;
	}

	if (sme->crypto.cipher_group) {
		priv->sec_info.encryption_mode = sme->crypto.cipher_group;
		priv->sec_info.authentication_mode = auth_type;
	}
	if (sme->ie)
		ret = mwifiex_set_gen_ie(priv, sme->ie, sme->ie_len);

	if (sme->key) {
		if (mwifiex_is_alg_wep(priv->sec_info.encryption_mode)) {
			dev_dbg(priv->adapter->dev,
				"info: setting wep encryption"
				" with key len %d\n", sme->key_len);
			priv->wep_key_curr_index = sme->key_idx;
			ret = mwifiex_set_encode(priv, NULL, sme->key,
						 sme->key_len, sme->key_idx,
						 NULL, 0);
		}
	}
done:
	/*
	 * Scan entries are valid for some time (15 sec). So we can save one
	 * active scan time if we just try cfg80211_get_bss first. If it fails
	 * then request scan and cfg80211_get_bss() again for final output.
	 */
	while (1) {
		if (is_scanning_required) {
			/* Do specific SSID scanning */
			if (mwifiex_request_scan(priv, &req_ssid)) {
				dev_err(priv->adapter->dev, "scan error\n");
				return -EFAULT;
			}
		}

		/* Find the BSS we want using available scan results */
		if (mode == NL80211_IFTYPE_ADHOC)
			bss = cfg80211_get_bss(priv->wdev.wiphy, channel,
					       bssid, ssid, ssid_len,
					       IEEE80211_BSS_TYPE_IBSS,
					       IEEE80211_PRIVACY_ANY);
		else
			bss = cfg80211_get_bss(priv->wdev.wiphy, channel,
					       bssid, ssid, ssid_len,
					       IEEE80211_BSS_TYPE_ESS,
					       IEEE80211_PRIVACY_ANY);

		if (!bss) {
			if (is_scanning_required) {
				dev_warn(priv->adapter->dev,
					 "assoc: requested bss not found in scan results\n");
				break;
			}
			is_scanning_required = 1;
		} else {
			dev_dbg(priv->adapter->dev,
				"info: trying to associate to '%s' bssid %pM\n",
				(char *) req_ssid.ssid, bss->bssid);
			memcpy(&priv->cfg_bssid, bss->bssid, ETH_ALEN);
			break;
		}
	}

	ret = mwifiex_bss_start(priv, bss, &req_ssid);
	if (ret)
		return ret;

	if (mode == NL80211_IFTYPE_ADHOC) {
		/* Inform the BSS information to kernel, otherwise
		 * kernel will give a panic after successful assoc */
		if (mwifiex_cfg80211_inform_ibss_bss(priv))
			return -EFAULT;
	}

	return ret;
}

/*
 * CFG802.11 operation handler for association request.
 *
 * This function does not work when the current mode is set to Ad-Hoc, or
 * when there is already an association procedure going on. The given BSS
 * information is used to associate.
 */
static int
mwifiex_cfg80211_connect(struct wiphy *wiphy, struct net_device *dev,
			 struct cfg80211_connect_params *sme)
{
	struct mwifiex_private *priv = mwifiex_netdev_get_priv(dev);
	struct mwifiex_adapter *adapter = priv->adapter;
	int ret;

	if (GET_BSS_ROLE(priv) != MWIFIEX_BSS_ROLE_STA) {
		wiphy_err(wiphy,
			  "%s: reject infra assoc request in non-STA role\n",
			  dev->name);
		return -EINVAL;
	}

	if (priv->wdev.current_bss) {
		wiphy_warn(wiphy, "%s: already connected\n", dev->name);
		return -EALREADY;
	}

	if (adapter->surprise_removed || adapter->is_cmd_timedout) {
		wiphy_err(wiphy,
			  "%s: Ignore connection. Card removed or FW in bad state\n",
			  dev->name);
		return -EFAULT;
	}

	wiphy_dbg(wiphy, "info: Trying to associate to %s and bssid %pM\n",
		  (char *) sme->ssid, sme->bssid);

	ret = mwifiex_cfg80211_assoc(priv, sme->ssid_len, sme->ssid, sme->bssid,
				     priv->bss_mode, sme->channel, sme, 0);
	if (!ret) {
		cfg80211_connect_result(priv->netdev, priv->cfg_bssid, NULL, 0,
					NULL, 0, WLAN_STATUS_SUCCESS,
					GFP_KERNEL);
		dev_dbg(priv->adapter->dev,
			"info: associated to bssid %pM successfully\n",
			priv->cfg_bssid);
		if (ISSUPP_TDLS_ENABLED(priv->adapter->fw_cap_info) &&
		    priv->adapter->auto_tdls &&
		    priv->bss_type == MWIFIEX_BSS_TYPE_STA)
			mwifiex_setup_auto_tdls_timer(priv);
	} else {
		dev_dbg(priv->adapter->dev,
			"info: association to bssid %pM failed\n",
			priv->cfg_bssid);
		eth_zero_addr(priv->cfg_bssid);

		if (ret > 0)
			cfg80211_connect_result(priv->netdev, priv->cfg_bssid,
						NULL, 0, NULL, 0, ret,
						GFP_KERNEL);
		else
			cfg80211_connect_result(priv->netdev, priv->cfg_bssid,
						NULL, 0, NULL, 0,
						WLAN_STATUS_UNSPECIFIED_FAILURE,
						GFP_KERNEL);
	}

	return 0;
}

/*
 * This function sets following parameters for ibss network.
 *  -  channel
 *  -  start band
 *  -  11n flag
 *  -  secondary channel offset
 */
static int mwifiex_set_ibss_params(struct mwifiex_private *priv,
				   struct cfg80211_ibss_params *params)
{
	struct wiphy *wiphy = priv->wdev.wiphy;
	struct mwifiex_adapter *adapter = priv->adapter;
	int index = 0, i;
	u8 config_bands = 0;

	if (params->chandef.chan->band == IEEE80211_BAND_2GHZ) {
		if (!params->basic_rates) {
			config_bands = BAND_B | BAND_G;
		} else {
			for (i = 0; i < mwifiex_band_2ghz.n_bitrates; i++) {
				/*
				 * Rates below 6 Mbps in the table are CCK
				 * rates; 802.11b and from 6 they are OFDM;
				 * 802.11G
				 */
				if (mwifiex_rates[i].bitrate == 60) {
					index = 1 << i;
					break;
				}
			}

			if (params->basic_rates < index) {
				config_bands = BAND_B;
			} else {
				config_bands = BAND_G;
				if (params->basic_rates % index)
					config_bands |= BAND_B;
			}
		}

		if (cfg80211_get_chandef_type(&params->chandef) !=
						NL80211_CHAN_NO_HT)
			config_bands |= BAND_G | BAND_GN;
	} else {
		if (cfg80211_get_chandef_type(&params->chandef) ==
						NL80211_CHAN_NO_HT)
			config_bands = BAND_A;
		else
			config_bands = BAND_AN | BAND_A;
	}

	if (!((config_bands | adapter->fw_bands) & ~adapter->fw_bands)) {
		adapter->config_bands = config_bands;
		adapter->adhoc_start_band = config_bands;

		if ((config_bands & BAND_GN) || (config_bands & BAND_AN))
			adapter->adhoc_11n_enabled = true;
		else
			adapter->adhoc_11n_enabled = false;
	}

	adapter->sec_chan_offset =
		mwifiex_chan_type_to_sec_chan_offset(
			cfg80211_get_chandef_type(&params->chandef));
	priv->adhoc_channel = ieee80211_frequency_to_channel(
				params->chandef.chan->center_freq);

	wiphy_dbg(wiphy, "info: set ibss band %d, chan %d, chan offset %d\n",
		  config_bands, priv->adhoc_channel, adapter->sec_chan_offset);

	return 0;
}

/*
 * CFG802.11 operation handler to join an IBSS.
 *
 * This function does not work in any mode other than Ad-Hoc, or if
 * a join operation is already in progress.
 */
static int
mwifiex_cfg80211_join_ibss(struct wiphy *wiphy, struct net_device *dev,
			   struct cfg80211_ibss_params *params)
{
	struct mwifiex_private *priv = mwifiex_netdev_get_priv(dev);
	int ret = 0;

	if (priv->bss_mode != NL80211_IFTYPE_ADHOC) {
		wiphy_err(wiphy, "request to join ibss received "
				"when station is not in ibss mode\n");
		goto done;
	}

	wiphy_dbg(wiphy, "info: trying to join to %s and bssid %pM\n",
		  (char *) params->ssid, params->bssid);

	mwifiex_set_ibss_params(priv, params);

	ret = mwifiex_cfg80211_assoc(priv, params->ssid_len, params->ssid,
				     params->bssid, priv->bss_mode,
				     params->chandef.chan, NULL,
				     params->privacy);
done:
	if (!ret) {
		cfg80211_ibss_joined(priv->netdev, priv->cfg_bssid,
				     params->chandef.chan, GFP_KERNEL);
		dev_dbg(priv->adapter->dev,
			"info: joined/created adhoc network with bssid"
			" %pM successfully\n", priv->cfg_bssid);
	} else {
		dev_dbg(priv->adapter->dev,
			"info: failed creating/joining adhoc network\n");
	}

	return ret;
}

/*
 * CFG802.11 operation handler to leave an IBSS.
 *
 * This function does not work if a leave operation is
 * already in progress.
 */
static int
mwifiex_cfg80211_leave_ibss(struct wiphy *wiphy, struct net_device *dev)
{
	struct mwifiex_private *priv = mwifiex_netdev_get_priv(dev);

	wiphy_dbg(wiphy, "info: disconnecting from essid %pM\n",
		  priv->cfg_bssid);
	if (mwifiex_deauthenticate(priv, NULL))
		return -EFAULT;

	eth_zero_addr(priv->cfg_bssid);

	return 0;
}

/*
 * CFG802.11 operation handler for scan request.
 *
 * This function issues a scan request to the firmware based upon
 * the user specified scan configuration. On successfull completion,
 * it also informs the results.
 */
static int
mwifiex_cfg80211_scan(struct wiphy *wiphy,
		      struct cfg80211_scan_request *request)
{
	struct net_device *dev = request->wdev->netdev;
	struct mwifiex_private *priv = mwifiex_netdev_get_priv(dev);
	int i, offset, ret;
	struct ieee80211_channel *chan;
	struct ieee_types_header *ie;
	struct mwifiex_user_scan_cfg *user_scan_cfg;

	wiphy_dbg(wiphy, "info: received scan request on %s\n", dev->name);

	/* Block scan request if scan operation or scan cleanup when interface
	 * is disabled is in process
	 */
	if (priv->scan_request || priv->scan_aborting) {
		dev_err(priv->adapter->dev, "cmd: Scan already in process..\n");
		return -EBUSY;
	}

	user_scan_cfg = kzalloc(sizeof(*user_scan_cfg), GFP_KERNEL);
	if (!user_scan_cfg)
		return -ENOMEM;

	priv->scan_request = request;

	user_scan_cfg->num_ssids = request->n_ssids;
	user_scan_cfg->ssid_list = request->ssids;

	if (request->ie && request->ie_len) {
		offset = 0;
		for (i = 0; i < MWIFIEX_MAX_VSIE_NUM; i++) {
			if (priv->vs_ie[i].mask != MWIFIEX_VSIE_MASK_CLEAR)
				continue;
			priv->vs_ie[i].mask = MWIFIEX_VSIE_MASK_SCAN;
			ie = (struct ieee_types_header *)(request->ie + offset);
			memcpy(&priv->vs_ie[i].ie, ie, sizeof(*ie) + ie->len);
			offset += sizeof(*ie) + ie->len;

			if (offset >= request->ie_len)
				break;
		}
	}

	for (i = 0; i < min_t(u32, request->n_channels,
			      MWIFIEX_USER_SCAN_CHAN_MAX); i++) {
		chan = request->channels[i];
		user_scan_cfg->chan_list[i].chan_number = chan->hw_value;
		user_scan_cfg->chan_list[i].radio_type = chan->band;

		if ((chan->flags & IEEE80211_CHAN_NO_IR) || !request->n_ssids)
			user_scan_cfg->chan_list[i].scan_type =
						MWIFIEX_SCAN_TYPE_PASSIVE;
		else
			user_scan_cfg->chan_list[i].scan_type =
						MWIFIEX_SCAN_TYPE_ACTIVE;

		user_scan_cfg->chan_list[i].scan_time = 0;
	}

	if (priv->adapter->scan_chan_gap_enabled &&
	    mwifiex_is_any_intf_active(priv))
		user_scan_cfg->scan_chan_gap =
					      priv->adapter->scan_chan_gap_time;

	ret = mwifiex_scan_networks(priv, user_scan_cfg);
	kfree(user_scan_cfg);
	if (ret) {
		dev_err(priv->adapter->dev, "scan failed: %d\n", ret);
		priv->scan_aborting = false;
		priv->scan_request = NULL;
		return ret;
	}

	if (request->ie && request->ie_len) {
		for (i = 0; i < MWIFIEX_MAX_VSIE_NUM; i++) {
			if (priv->vs_ie[i].mask == MWIFIEX_VSIE_MASK_SCAN) {
				priv->vs_ie[i].mask = MWIFIEX_VSIE_MASK_CLEAR;
				memset(&priv->vs_ie[i].ie, 0,
				       MWIFIEX_MAX_VSIE_LEN);
			}
		}
	}
	return 0;
}

static void mwifiex_setup_vht_caps(struct ieee80211_sta_vht_cap *vht_info,
				   struct mwifiex_private *priv)
{
	struct mwifiex_adapter *adapter = priv->adapter;

	vht_info->vht_supported = true;

	vht_info->cap = adapter->hw_dot_11ac_dev_cap;
	/* Update MCS support for VHT */
	vht_info->vht_mcs.rx_mcs_map = cpu_to_le16(
				adapter->hw_dot_11ac_mcs_support & 0xFFFF);
	vht_info->vht_mcs.rx_highest = 0;
	vht_info->vht_mcs.tx_mcs_map = cpu_to_le16(
				adapter->hw_dot_11ac_mcs_support >> 16);
	vht_info->vht_mcs.tx_highest = 0;
}

/*
 * This function sets up the CFG802.11 specific HT capability fields
 * with default values.
 *
 * The following default values are set -
 *      - HT Supported = True
 *      - Maximum AMPDU length factor = IEEE80211_HT_MAX_AMPDU_64K
 *      - Minimum AMPDU spacing = IEEE80211_HT_MPDU_DENSITY_NONE
 *      - HT Capabilities supported by firmware
 *      - MCS information, Rx mask = 0xff
 *      - MCD information, Tx parameters = IEEE80211_HT_MCS_TX_DEFINED (0x01)
 */
static void
mwifiex_setup_ht_caps(struct ieee80211_sta_ht_cap *ht_info,
		      struct mwifiex_private *priv)
{
	int rx_mcs_supp;
	struct ieee80211_mcs_info mcs_set;
	u8 *mcs = (u8 *)&mcs_set;
	struct mwifiex_adapter *adapter = priv->adapter;

	ht_info->ht_supported = true;
	ht_info->ampdu_factor = IEEE80211_HT_MAX_AMPDU_64K;
	ht_info->ampdu_density = IEEE80211_HT_MPDU_DENSITY_NONE;

	memset(&ht_info->mcs, 0, sizeof(ht_info->mcs));

	/* Fill HT capability information */
	if (ISSUPP_CHANWIDTH40(adapter->hw_dot_11n_dev_cap))
		ht_info->cap |= IEEE80211_HT_CAP_SUP_WIDTH_20_40;
	else
		ht_info->cap &= ~IEEE80211_HT_CAP_SUP_WIDTH_20_40;

	if (ISSUPP_SHORTGI20(adapter->hw_dot_11n_dev_cap))
		ht_info->cap |= IEEE80211_HT_CAP_SGI_20;
	else
		ht_info->cap &= ~IEEE80211_HT_CAP_SGI_20;

	if (ISSUPP_SHORTGI40(adapter->hw_dot_11n_dev_cap))
		ht_info->cap |= IEEE80211_HT_CAP_SGI_40;
	else
		ht_info->cap &= ~IEEE80211_HT_CAP_SGI_40;

	if (adapter->user_dev_mcs_support == HT_STREAM_2X2)
		ht_info->cap |= 3 << IEEE80211_HT_CAP_RX_STBC_SHIFT;
	else
		ht_info->cap |= 1 << IEEE80211_HT_CAP_RX_STBC_SHIFT;

	if (ISSUPP_TXSTBC(adapter->hw_dot_11n_dev_cap))
		ht_info->cap |= IEEE80211_HT_CAP_TX_STBC;
	else
		ht_info->cap &= ~IEEE80211_HT_CAP_TX_STBC;

	if (ISSUPP_GREENFIELD(adapter->hw_dot_11n_dev_cap))
		ht_info->cap |= IEEE80211_HT_CAP_GRN_FLD;
	else
		ht_info->cap &= ~IEEE80211_HT_CAP_GRN_FLD;

	if (ISENABLED_40MHZ_INTOLERANT(adapter->hw_dot_11n_dev_cap))
		ht_info->cap |= IEEE80211_HT_CAP_40MHZ_INTOLERANT;
	else
		ht_info->cap &= ~IEEE80211_HT_CAP_40MHZ_INTOLERANT;

	if (ISSUPP_RXLDPC(adapter->hw_dot_11n_dev_cap))
		ht_info->cap |= IEEE80211_HT_CAP_LDPC_CODING;
	else
		ht_info->cap &= ~IEEE80211_HT_CAP_LDPC_CODING;

	ht_info->cap &= ~IEEE80211_HT_CAP_MAX_AMSDU;
	ht_info->cap |= IEEE80211_HT_CAP_SM_PS;

	rx_mcs_supp = GET_RXMCSSUPP(adapter->user_dev_mcs_support);
	/* Set MCS for 1x1/2x2 */
	memset(mcs, 0xff, rx_mcs_supp);
	/* Clear all the other values */
	memset(&mcs[rx_mcs_supp], 0,
	       sizeof(struct ieee80211_mcs_info) - rx_mcs_supp);
	if (priv->bss_mode == NL80211_IFTYPE_STATION ||
	    ISSUPP_CHANWIDTH40(adapter->hw_dot_11n_dev_cap))
		/* Set MCS32 for infra mode or ad-hoc mode with 40MHz support */
		SETHT_MCS32(mcs_set.rx_mask);

	memcpy((u8 *) &ht_info->mcs, mcs, sizeof(struct ieee80211_mcs_info));

	ht_info->mcs.tx_params = IEEE80211_HT_MCS_TX_DEFINED;
}

/*
 *  create a new virtual interface with the given name and name assign type
 */
struct wireless_dev *mwifiex_add_virtual_intf(struct wiphy *wiphy,
					      const char *name,
					      unsigned char name_assign_type,
					      enum nl80211_iftype type,
					      u32 *flags,
					      struct vif_params *params)
{
	struct mwifiex_adapter *adapter = mwifiex_cfg80211_get_adapter(wiphy);
	struct mwifiex_private *priv;
	struct net_device *dev;
	void *mdev_priv;

	if (!adapter)
		return ERR_PTR(-EFAULT);

	switch (type) {
	case NL80211_IFTYPE_UNSPECIFIED:
	case NL80211_IFTYPE_STATION:
	case NL80211_IFTYPE_ADHOC:
		if (adapter->curr_iface_comb.sta_intf ==
		    adapter->iface_limit.sta_intf) {
			wiphy_err(wiphy,
				  "cannot create multiple sta/adhoc ifaces\n");
			return ERR_PTR(-EINVAL);
		}

		priv = mwifiex_get_unused_priv(adapter);
		if (!priv) {
			wiphy_err(wiphy,
				  "could not get free private struct\n");
			return ERR_PTR(-EFAULT);
		}

		priv->wdev.wiphy = wiphy;
		priv->wdev.iftype = NL80211_IFTYPE_STATION;

		if (type == NL80211_IFTYPE_UNSPECIFIED)
			priv->bss_mode = NL80211_IFTYPE_STATION;
		else
			priv->bss_mode = type;

		priv->bss_type = MWIFIEX_BSS_TYPE_STA;
		priv->frame_type = MWIFIEX_DATA_FRAME_TYPE_ETH_II;
		priv->bss_priority = 0;
		priv->bss_role = MWIFIEX_BSS_ROLE_STA;
		priv->bss_num = 0;

		break;
	case NL80211_IFTYPE_AP:
		if (adapter->curr_iface_comb.uap_intf ==
		    adapter->iface_limit.uap_intf) {
			wiphy_err(wiphy,
				  "cannot create multiple AP ifaces\n");
			return ERR_PTR(-EINVAL);
		}

		priv = mwifiex_get_unused_priv(adapter);
		if (!priv) {
			wiphy_err(wiphy,
				  "could not get free private struct\n");
			return ERR_PTR(-EFAULT);
		}

		priv->wdev.wiphy = wiphy;
		priv->wdev.iftype = NL80211_IFTYPE_AP;

		priv->bss_type = MWIFIEX_BSS_TYPE_UAP;
		priv->frame_type = MWIFIEX_DATA_FRAME_TYPE_ETH_II;
		priv->bss_priority = 0;
		priv->bss_role = MWIFIEX_BSS_ROLE_UAP;
		priv->bss_started = 0;
		priv->bss_num = 0;
		priv->bss_mode = type;

		break;
	case NL80211_IFTYPE_P2P_CLIENT:
		if (adapter->curr_iface_comb.p2p_intf ==
		    adapter->iface_limit.p2p_intf) {
			wiphy_err(wiphy,
				  "cannot create multiple P2P ifaces\n");
			return ERR_PTR(-EINVAL);
		}

		priv = mwifiex_get_unused_priv(adapter);
		if (!priv) {
			wiphy_err(wiphy,
				  "could not get free private struct\n");
			return ERR_PTR(-EFAULT);
		}

		priv->wdev.wiphy = wiphy;
		/* At start-up, wpa_supplicant tries to change the interface
		 * to NL80211_IFTYPE_STATION if it is not managed mode.
		 */
		priv->wdev.iftype = NL80211_IFTYPE_P2P_CLIENT;
		priv->bss_mode = NL80211_IFTYPE_P2P_CLIENT;

		/* Setting bss_type to P2P tells firmware that this interface
		 * is receiving P2P peers found during find phase and doing
		 * action frame handshake.
		 */
		priv->bss_type = MWIFIEX_BSS_TYPE_P2P;

		priv->frame_type = MWIFIEX_DATA_FRAME_TYPE_ETH_II;
		priv->bss_priority = MWIFIEX_BSS_ROLE_STA;
		priv->bss_role = MWIFIEX_BSS_ROLE_STA;
		priv->bss_started = 0;
		priv->bss_num = 0;

		if (mwifiex_cfg80211_init_p2p_client(priv)) {
			memset(&priv->wdev, 0, sizeof(priv->wdev));
			priv->wdev.iftype = NL80211_IFTYPE_UNSPECIFIED;
			return ERR_PTR(-EFAULT);
		}

		break;
	default:
		wiphy_err(wiphy, "type not supported\n");
		return ERR_PTR(-EINVAL);
	}

	dev = alloc_netdev_mqs(sizeof(struct mwifiex_private *), name,
			       name_assign_type, ether_setup,
			       IEEE80211_NUM_ACS, 1);
	if (!dev) {
		wiphy_err(wiphy, "no memory available for netdevice\n");
		memset(&priv->wdev, 0, sizeof(priv->wdev));
		priv->wdev.iftype = NL80211_IFTYPE_UNSPECIFIED;
		priv->bss_mode = NL80211_IFTYPE_UNSPECIFIED;
		return ERR_PTR(-ENOMEM);
	}

	mwifiex_init_priv_params(priv, dev);
	priv->netdev = dev;

	mwifiex_setup_ht_caps(&wiphy->bands[IEEE80211_BAND_2GHZ]->ht_cap, priv);
	if (adapter->is_hw_11ac_capable)
		mwifiex_setup_vht_caps(
			&wiphy->bands[IEEE80211_BAND_2GHZ]->vht_cap, priv);

	if (adapter->config_bands & BAND_A)
		mwifiex_setup_ht_caps(
			&wiphy->bands[IEEE80211_BAND_5GHZ]->ht_cap, priv);

	if ((adapter->config_bands & BAND_A) && adapter->is_hw_11ac_capable)
		mwifiex_setup_vht_caps(
			&wiphy->bands[IEEE80211_BAND_5GHZ]->vht_cap, priv);

	dev_net_set(dev, wiphy_net(wiphy));
	dev->ieee80211_ptr = &priv->wdev;
	dev->ieee80211_ptr->iftype = priv->bss_mode;
	memcpy(dev->dev_addr, wiphy->perm_addr, ETH_ALEN);
	SET_NETDEV_DEV(dev, wiphy_dev(wiphy));

	dev->flags |= IFF_BROADCAST | IFF_MULTICAST;
	dev->watchdog_timeo = MWIFIEX_DEFAULT_WATCHDOG_TIMEOUT;
	dev->hard_header_len += MWIFIEX_MIN_DATA_HEADER_LEN;
	dev->ethtool_ops = &mwifiex_ethtool_ops;

	mdev_priv = netdev_priv(dev);
	*((unsigned long *) mdev_priv) = (unsigned long) priv;

	SET_NETDEV_DEV(dev, adapter->dev);

	/* Register network device */
	if (register_netdevice(dev)) {
		wiphy_err(wiphy, "cannot register virtual network device\n");
		free_netdev(dev);
		priv->bss_mode = NL80211_IFTYPE_UNSPECIFIED;
		priv->netdev = NULL;
		memset(&priv->wdev, 0, sizeof(priv->wdev));
		priv->wdev.iftype = NL80211_IFTYPE_UNSPECIFIED;
		return ERR_PTR(-EFAULT);
	}

	priv->dfs_cac_workqueue = alloc_workqueue("MWIFIEX_DFS_CAC%s",
						  WQ_HIGHPRI |
						  WQ_MEM_RECLAIM |
						  WQ_UNBOUND, 1, name);
	if (!priv->dfs_cac_workqueue) {
		wiphy_err(wiphy, "cannot register virtual network device\n");
		free_netdev(dev);
		priv->bss_mode = NL80211_IFTYPE_UNSPECIFIED;
		priv->netdev = NULL;
		memset(&priv->wdev, 0, sizeof(priv->wdev));
		priv->wdev.iftype = NL80211_IFTYPE_UNSPECIFIED;
		return ERR_PTR(-ENOMEM);
	}

	INIT_DELAYED_WORK(&priv->dfs_cac_work, mwifiex_dfs_cac_work_queue);

	priv->dfs_chan_sw_workqueue = alloc_workqueue("MWIFIEX_DFS_CHSW%s",
						      WQ_HIGHPRI | WQ_UNBOUND |
						      WQ_MEM_RECLAIM, 1, name);
	if (!priv->dfs_chan_sw_workqueue) {
		wiphy_err(wiphy, "cannot register virtual network device\n");
		free_netdev(dev);
		priv->bss_mode = NL80211_IFTYPE_UNSPECIFIED;
		priv->netdev = NULL;
		memset(&priv->wdev, 0, sizeof(priv->wdev));
		priv->wdev.iftype = NL80211_IFTYPE_UNSPECIFIED;
		return ERR_PTR(-ENOMEM);
	}

	INIT_DELAYED_WORK(&priv->dfs_chan_sw_work,
			  mwifiex_dfs_chan_sw_work_queue);

	sema_init(&priv->async_sem, 1);

	dev_dbg(adapter->dev, "info: %s: Marvell 802.11 Adapter\n", dev->name);

#ifdef CONFIG_DEBUG_FS
	mwifiex_dev_debugfs_init(priv);
#endif

	switch (type) {
	case NL80211_IFTYPE_UNSPECIFIED:
	case NL80211_IFTYPE_STATION:
	case NL80211_IFTYPE_ADHOC:
		adapter->curr_iface_comb.sta_intf++;
		break;
	case NL80211_IFTYPE_AP:
		adapter->curr_iface_comb.uap_intf++;
		break;
	case NL80211_IFTYPE_P2P_CLIENT:
		adapter->curr_iface_comb.p2p_intf++;
		break;
	default:
		wiphy_err(wiphy, "type not supported\n");
		return ERR_PTR(-EINVAL);
	}

	return &priv->wdev;
}
EXPORT_SYMBOL_GPL(mwifiex_add_virtual_intf);

/*
 * del_virtual_intf: remove the virtual interface determined by dev
 */
int mwifiex_del_virtual_intf(struct wiphy *wiphy, struct wireless_dev *wdev)
{
	struct mwifiex_private *priv = mwifiex_netdev_get_priv(wdev->netdev);
	struct mwifiex_adapter *adapter = priv->adapter;

#ifdef CONFIG_DEBUG_FS
	mwifiex_dev_debugfs_remove(priv);
#endif

	mwifiex_stop_net_dev_queue(priv->netdev, adapter);

	if (netif_carrier_ok(priv->netdev))
		netif_carrier_off(priv->netdev);

	if (wdev->netdev->reg_state == NETREG_REGISTERED)
		unregister_netdevice(wdev->netdev);

	if (priv->dfs_cac_workqueue) {
		flush_workqueue(priv->dfs_cac_workqueue);
		destroy_workqueue(priv->dfs_cac_workqueue);
		priv->dfs_cac_workqueue = NULL;
	}

	if (priv->dfs_chan_sw_workqueue) {
		flush_workqueue(priv->dfs_chan_sw_workqueue);
		destroy_workqueue(priv->dfs_chan_sw_workqueue);
		priv->dfs_chan_sw_workqueue = NULL;
	}
	/* Clear the priv in adapter */
	priv->netdev->ieee80211_ptr = NULL;
	priv->netdev = NULL;
	priv->wdev.iftype = NL80211_IFTYPE_UNSPECIFIED;

	priv->media_connected = false;

	switch (priv->bss_mode) {
	case NL80211_IFTYPE_UNSPECIFIED:
	case NL80211_IFTYPE_STATION:
	case NL80211_IFTYPE_ADHOC:
		adapter->curr_iface_comb.sta_intf++;
		break;
	case NL80211_IFTYPE_AP:
		adapter->curr_iface_comb.uap_intf++;
		break;
	case NL80211_IFTYPE_P2P_CLIENT:
	case NL80211_IFTYPE_P2P_GO:
		adapter->curr_iface_comb.p2p_intf++;
		break;
	default:
		dev_err(adapter->dev, "del_virtual_intf: type not supported\n");
		break;
	}

	priv->bss_mode = NL80211_IFTYPE_UNSPECIFIED;

	if (GET_BSS_ROLE(priv) == MWIFIEX_BSS_ROLE_STA ||
	    GET_BSS_ROLE(priv) == MWIFIEX_BSS_ROLE_UAP)
		kfree(priv->hist_data);

	return 0;
}
EXPORT_SYMBOL_GPL(mwifiex_del_virtual_intf);

static bool
mwifiex_is_pattern_supported(struct cfg80211_pkt_pattern *pat, s8 *byte_seq,
			     u8 max_byte_seq)
{
	int j, k, valid_byte_cnt = 0;
	bool dont_care_byte = false;

	for (j = 0; j < DIV_ROUND_UP(pat->pattern_len, 8); j++) {
		for (k = 0; k < 8; k++) {
			if (pat->mask[j] & 1 << k) {
				memcpy(byte_seq + valid_byte_cnt,
				       &pat->pattern[j * 8 + k], 1);
				valid_byte_cnt++;
				if (dont_care_byte)
					return false;
			} else {
				if (valid_byte_cnt)
					dont_care_byte = true;
			}

			if (valid_byte_cnt > max_byte_seq)
				return false;
		}
	}

	byte_seq[max_byte_seq] = valid_byte_cnt;

	return true;
}

#ifdef CONFIG_PM
static void mwifiex_set_auto_arp_mef_entry(struct mwifiex_private *priv,
					   struct mwifiex_mef_entry *mef_entry)
{
	int i, filt_num = 0, num_ipv4 = 0;
	struct in_device *in_dev;
	struct in_ifaddr *ifa;
	__be32 ips[MWIFIEX_MAX_SUPPORTED_IPADDR];
	struct mwifiex_adapter *adapter = priv->adapter;

	mef_entry->mode = MEF_MODE_HOST_SLEEP;
	mef_entry->action = MEF_ACTION_AUTO_ARP;

	/* Enable ARP offload feature */
	memset(ips, 0, sizeof(ips));
	for (i = 0; i < MWIFIEX_MAX_BSS_NUM; i++) {
		if (adapter->priv[i]->netdev) {
			in_dev = __in_dev_get_rtnl(adapter->priv[i]->netdev);
			if (!in_dev)
				continue;
			ifa = in_dev->ifa_list;
			if (!ifa || !ifa->ifa_local)
				continue;
			ips[i] = ifa->ifa_local;
			num_ipv4++;
		}
	}

	for (i = 0; i < num_ipv4; i++) {
		if (!ips[i])
			continue;
		mef_entry->filter[filt_num].repeat = 1;
		memcpy(mef_entry->filter[filt_num].byte_seq,
		       (u8 *)&ips[i], sizeof(ips[i]));
		mef_entry->filter[filt_num].
			byte_seq[MWIFIEX_MEF_MAX_BYTESEQ] =
			sizeof(ips[i]);
		mef_entry->filter[filt_num].offset = 46;
		mef_entry->filter[filt_num].filt_type = TYPE_EQ;
		if (filt_num) {
			mef_entry->filter[filt_num].filt_action =
				TYPE_OR;
		}
		filt_num++;
	}

	mef_entry->filter[filt_num].repeat = 1;
	mef_entry->filter[filt_num].byte_seq[0] = 0x08;
	mef_entry->filter[filt_num].byte_seq[1] = 0x06;
	mef_entry->filter[filt_num].byte_seq[MWIFIEX_MEF_MAX_BYTESEQ] = 2;
	mef_entry->filter[filt_num].offset = 20;
	mef_entry->filter[filt_num].filt_type = TYPE_EQ;
	mef_entry->filter[filt_num].filt_action = TYPE_AND;
}

static int mwifiex_set_wowlan_mef_entry(struct mwifiex_private *priv,
					struct mwifiex_ds_mef_cfg *mef_cfg,
					struct mwifiex_mef_entry *mef_entry,
					struct cfg80211_wowlan *wowlan)
{
	int i, filt_num = 0, ret = 0;
	bool first_pat = true;
	u8 byte_seq[MWIFIEX_MEF_MAX_BYTESEQ + 1];
	const u8 ipv4_mc_mac[] = {0x33, 0x33};
	const u8 ipv6_mc_mac[] = {0x01, 0x00, 0x5e};

	mef_entry->mode = MEF_MODE_HOST_SLEEP;
	mef_entry->action = MEF_ACTION_ALLOW_AND_WAKEUP_HOST;

	for (i = 0; i < wowlan->n_patterns; i++) {
		memset(byte_seq, 0, sizeof(byte_seq));
		if (!mwifiex_is_pattern_supported(&wowlan->patterns[i],
					byte_seq,
					MWIFIEX_MEF_MAX_BYTESEQ)) {
			dev_err(priv->adapter->dev, "Pattern not supported\n");
			kfree(mef_entry);
			return -EOPNOTSUPP;
		}

		if (!wowlan->patterns[i].pkt_offset) {
			if (!(byte_seq[0] & 0x01) &&
			    (byte_seq[MWIFIEX_MEF_MAX_BYTESEQ] == 1)) {
				mef_cfg->criteria |= MWIFIEX_CRITERIA_UNICAST;
				continue;
			} else if (is_broadcast_ether_addr(byte_seq)) {
				mef_cfg->criteria |= MWIFIEX_CRITERIA_BROADCAST;
				continue;
			} else if ((!memcmp(byte_seq, ipv4_mc_mac, 2) &&
				    (byte_seq[MWIFIEX_MEF_MAX_BYTESEQ] == 2)) ||
				   (!memcmp(byte_seq, ipv6_mc_mac, 3) &&
				    (byte_seq[MWIFIEX_MEF_MAX_BYTESEQ] == 3))) {
				mef_cfg->criteria |= MWIFIEX_CRITERIA_MULTICAST;
				continue;
			}
		}
		mef_entry->filter[filt_num].repeat = 1;
		mef_entry->filter[filt_num].offset =
			wowlan->patterns[i].pkt_offset;
		memcpy(mef_entry->filter[filt_num].byte_seq, byte_seq,
				sizeof(byte_seq));
		mef_entry->filter[filt_num].filt_type = TYPE_EQ;

		if (first_pat)
			first_pat = false;
		else
			mef_entry->filter[filt_num].filt_action = TYPE_AND;

		filt_num++;
	}

	if (wowlan->magic_pkt) {
		mef_cfg->criteria |= MWIFIEX_CRITERIA_UNICAST;
		mef_entry->filter[filt_num].repeat = 16;
		memcpy(mef_entry->filter[filt_num].byte_seq, priv->curr_addr,
				ETH_ALEN);
		mef_entry->filter[filt_num].byte_seq[MWIFIEX_MEF_MAX_BYTESEQ] =
			ETH_ALEN;
		mef_entry->filter[filt_num].offset = 28;
		mef_entry->filter[filt_num].filt_type = TYPE_EQ;
		if (filt_num)
			mef_entry->filter[filt_num].filt_action = TYPE_OR;

		filt_num++;
		mef_entry->filter[filt_num].repeat = 16;
		memcpy(mef_entry->filter[filt_num].byte_seq, priv->curr_addr,
				ETH_ALEN);
		mef_entry->filter[filt_num].byte_seq[MWIFIEX_MEF_MAX_BYTESEQ] =
			ETH_ALEN;
		mef_entry->filter[filt_num].offset = 56;
		mef_entry->filter[filt_num].filt_type = TYPE_EQ;
		mef_entry->filter[filt_num].filt_action = TYPE_OR;
	}
	return ret;
}

static int mwifiex_set_mef_filter(struct mwifiex_private *priv,
				  struct cfg80211_wowlan *wowlan)
{
	int ret = 0, num_entries = 1;
	struct mwifiex_ds_mef_cfg mef_cfg;
	struct mwifiex_mef_entry *mef_entry;

	if (wowlan->n_patterns || wowlan->magic_pkt)
		num_entries++;

	mef_entry = kcalloc(num_entries, sizeof(*mef_entry), GFP_KERNEL);
	if (!mef_entry)
		return -ENOMEM;

	memset(&mef_cfg, 0, sizeof(mef_cfg));
	mef_cfg.criteria |= MWIFIEX_CRITERIA_BROADCAST |
		MWIFIEX_CRITERIA_UNICAST;
	mef_cfg.num_entries = num_entries;
	mef_cfg.mef_entry = mef_entry;

	mwifiex_set_auto_arp_mef_entry(priv, &mef_entry[0]);

	if (wowlan->n_patterns || wowlan->magic_pkt)
		ret = mwifiex_set_wowlan_mef_entry(priv, &mef_cfg,
						   &mef_entry[1], wowlan);

	if (!mef_cfg.criteria)
		mef_cfg.criteria = MWIFIEX_CRITERIA_BROADCAST |
			MWIFIEX_CRITERIA_UNICAST |
			MWIFIEX_CRITERIA_MULTICAST;

	ret = mwifiex_send_cmd(priv, HostCmd_CMD_MEF_CFG,
			HostCmd_ACT_GEN_SET, 0,
			&mef_cfg, true);
	kfree(mef_entry);
	return ret;
}

static int mwifiex_cfg80211_suspend(struct wiphy *wiphy,
				    struct cfg80211_wowlan *wowlan)
{
	struct mwifiex_adapter *adapter = mwifiex_cfg80211_get_adapter(wiphy);
	struct mwifiex_ds_hs_cfg hs_cfg;
	int i, ret = 0;
	struct mwifiex_private *priv;

	for (i = 0; i < adapter->priv_num; i++) {
		priv = adapter->priv[i];
		mwifiex_abort_cac(priv);
	}

	mwifiex_cancel_all_pending_cmd(adapter);

	if (!wowlan) {
		dev_warn(adapter->dev, "None of the WOWLAN triggers enabled\n");
		return 0;
	}

	priv = mwifiex_get_priv(adapter, MWIFIEX_BSS_ROLE_STA);

	if (!priv->media_connected) {
		dev_warn(adapter->dev,
			 "Can not configure WOWLAN in disconnected state\n");
		return 0;
	}

	ret = mwifiex_set_mef_filter(priv, wowlan);
	if (ret) {
		dev_err(adapter->dev, "Failed to set MEF filter\n");
		return ret;
	}

	if (wowlan->disconnect) {
		memset(&hs_cfg, 0, sizeof(hs_cfg));
		hs_cfg.is_invoke_hostcmd = false;
		hs_cfg.conditions = HS_CFG_COND_MAC_EVENT;
		hs_cfg.gpio = HS_CFG_GPIO_DEF;
		hs_cfg.gap = HS_CFG_GAP_DEF;
		ret = mwifiex_set_hs_params(priv, HostCmd_ACT_GEN_SET,
					    MWIFIEX_SYNC_CMD, &hs_cfg);
		if (ret) {
			dev_err(adapter->dev, "Failed to set HS params\n");
			return ret;
		}
	}

	return ret;
}

static int mwifiex_cfg80211_resume(struct wiphy *wiphy)
{
	return 0;
}

static void mwifiex_cfg80211_set_wakeup(struct wiphy *wiphy,
				       bool enabled)
{
	struct mwifiex_adapter *adapter = mwifiex_cfg80211_get_adapter(wiphy);

	device_set_wakeup_enable(adapter->dev, enabled);
}
#endif

static int mwifiex_get_coalesce_pkt_type(u8 *byte_seq)
{
	const u8 ipv4_mc_mac[] = {0x33, 0x33};
	const u8 ipv6_mc_mac[] = {0x01, 0x00, 0x5e};
	const u8 bc_mac[] = {0xff, 0xff, 0xff, 0xff};

	if ((byte_seq[0] & 0x01) &&
	    (byte_seq[MWIFIEX_COALESCE_MAX_BYTESEQ] == 1))
		return PACKET_TYPE_UNICAST;
	else if (!memcmp(byte_seq, bc_mac, 4))
		return PACKET_TYPE_BROADCAST;
	else if ((!memcmp(byte_seq, ipv4_mc_mac, 2) &&
		  byte_seq[MWIFIEX_COALESCE_MAX_BYTESEQ] == 2) ||
		 (!memcmp(byte_seq, ipv6_mc_mac, 3) &&
		  byte_seq[MWIFIEX_COALESCE_MAX_BYTESEQ] == 3))
		return PACKET_TYPE_MULTICAST;

	return 0;
}

static int
mwifiex_fill_coalesce_rule_info(struct mwifiex_private *priv,
				struct cfg80211_coalesce_rules *crule,
				struct mwifiex_coalesce_rule *mrule)
{
	u8 byte_seq[MWIFIEX_COALESCE_MAX_BYTESEQ + 1];
	struct filt_field_param *param;
	int i;

	mrule->max_coalescing_delay = crule->delay;

	param = mrule->params;

	for (i = 0; i < crule->n_patterns; i++) {
		memset(byte_seq, 0, sizeof(byte_seq));
		if (!mwifiex_is_pattern_supported(&crule->patterns[i],
						  byte_seq,
						MWIFIEX_COALESCE_MAX_BYTESEQ)) {
			dev_err(priv->adapter->dev, "Pattern not supported\n");
			return -EOPNOTSUPP;
		}

		if (!crule->patterns[i].pkt_offset) {
			u8 pkt_type;

			pkt_type = mwifiex_get_coalesce_pkt_type(byte_seq);
			if (pkt_type && mrule->pkt_type) {
				dev_err(priv->adapter->dev,
					"Multiple packet types not allowed\n");
				return -EOPNOTSUPP;
			} else if (pkt_type) {
				mrule->pkt_type = pkt_type;
				continue;
			}
		}

		if (crule->condition == NL80211_COALESCE_CONDITION_MATCH)
			param->operation = RECV_FILTER_MATCH_TYPE_EQ;
		else
			param->operation = RECV_FILTER_MATCH_TYPE_NE;

		param->operand_len = byte_seq[MWIFIEX_COALESCE_MAX_BYTESEQ];
		memcpy(param->operand_byte_stream, byte_seq,
		       param->operand_len);
		param->offset = crule->patterns[i].pkt_offset;
		param++;

		mrule->num_of_fields++;
	}

	if (!mrule->pkt_type) {
		dev_err(priv->adapter->dev,
			"Packet type can not be determined\n");
		return -EOPNOTSUPP;
	}

	return 0;
}

static int mwifiex_cfg80211_set_coalesce(struct wiphy *wiphy,
					 struct cfg80211_coalesce *coalesce)
{
	struct mwifiex_adapter *adapter = mwifiex_cfg80211_get_adapter(wiphy);
	int i, ret;
	struct mwifiex_ds_coalesce_cfg coalesce_cfg;
	struct mwifiex_private *priv =
			mwifiex_get_priv(adapter, MWIFIEX_BSS_ROLE_STA);

	memset(&coalesce_cfg, 0, sizeof(coalesce_cfg));
	if (!coalesce) {
		dev_dbg(adapter->dev,
			"Disable coalesce and reset all previous rules\n");
		return mwifiex_send_cmd(priv, HostCmd_CMD_COALESCE_CFG,
					HostCmd_ACT_GEN_SET, 0,
					&coalesce_cfg, true);
	}

	coalesce_cfg.num_of_rules = coalesce->n_rules;
	for (i = 0; i < coalesce->n_rules; i++) {
		ret = mwifiex_fill_coalesce_rule_info(priv, &coalesce->rules[i],
						      &coalesce_cfg.rule[i]);
		if (ret) {
			dev_err(priv->adapter->dev,
				"Recheck the patterns provided for rule %d\n",
				i + 1);
			return ret;
		}
	}

	return mwifiex_send_cmd(priv, HostCmd_CMD_COALESCE_CFG,
				HostCmd_ACT_GEN_SET, 0, &coalesce_cfg, true);
}

/* cfg80211 ops handler for tdls_mgmt.
 * Function prepares TDLS action frame packets and forwards them to FW
 */
static int
mwifiex_cfg80211_tdls_mgmt(struct wiphy *wiphy, struct net_device *dev,
			   const u8 *peer, u8 action_code, u8 dialog_token,
			   u16 status_code, u32 peer_capability,
			   bool initiator, const u8 *extra_ies,
			   size_t extra_ies_len)
{
	struct mwifiex_private *priv = mwifiex_netdev_get_priv(dev);
	int ret;

	if (!(wiphy->flags & WIPHY_FLAG_SUPPORTS_TDLS))
		return -ENOTSUPP;

	/* make sure we are in station mode and connected */
	if (!(priv->bss_type == MWIFIEX_BSS_TYPE_STA && priv->media_connected))
		return -ENOTSUPP;

	switch (action_code) {
	case WLAN_TDLS_SETUP_REQUEST:
		dev_dbg(priv->adapter->dev,
			"Send TDLS Setup Request to %pM status_code=%d\n", peer,
			 status_code);
		mwifiex_add_auto_tdls_peer(priv, peer);
		ret = mwifiex_send_tdls_data_frame(priv, peer, action_code,
						   dialog_token, status_code,
						   extra_ies, extra_ies_len);
		break;
	case WLAN_TDLS_SETUP_RESPONSE:
		mwifiex_add_auto_tdls_peer(priv, peer);
		dev_dbg(priv->adapter->dev,
			"Send TDLS Setup Response to %pM status_code=%d\n",
			peer, status_code);
		ret = mwifiex_send_tdls_data_frame(priv, peer, action_code,
						   dialog_token, status_code,
						   extra_ies, extra_ies_len);
		break;
	case WLAN_TDLS_SETUP_CONFIRM:
		dev_dbg(priv->adapter->dev,
			"Send TDLS Confirm to %pM status_code=%d\n", peer,
			status_code);
		ret = mwifiex_send_tdls_data_frame(priv, peer, action_code,
						   dialog_token, status_code,
						   extra_ies, extra_ies_len);
		break;
	case WLAN_TDLS_TEARDOWN:
		dev_dbg(priv->adapter->dev, "Send TDLS Tear down to %pM\n",
			peer);
		ret = mwifiex_send_tdls_data_frame(priv, peer, action_code,
						   dialog_token, status_code,
						   extra_ies, extra_ies_len);
		break;
	case WLAN_TDLS_DISCOVERY_REQUEST:
		dev_dbg(priv->adapter->dev,
			"Send TDLS Discovery Request to %pM\n", peer);
		ret = mwifiex_send_tdls_data_frame(priv, peer, action_code,
						   dialog_token, status_code,
						   extra_ies, extra_ies_len);
		break;
	case WLAN_PUB_ACTION_TDLS_DISCOVER_RES:
		dev_dbg(priv->adapter->dev,
			"Send TDLS Discovery Response to %pM\n", peer);
		ret = mwifiex_send_tdls_action_frame(priv, peer, action_code,
						   dialog_token, status_code,
						   extra_ies, extra_ies_len);
		break;
	default:
		dev_warn(priv->adapter->dev,
			 "Unknown TDLS mgmt/action frame %pM\n", peer);
		ret = -EINVAL;
		break;
	}

	return ret;
}

static int
mwifiex_cfg80211_tdls_oper(struct wiphy *wiphy, struct net_device *dev,
			   const u8 *peer, enum nl80211_tdls_operation action)
{
	struct mwifiex_private *priv = mwifiex_netdev_get_priv(dev);

	if (!(wiphy->flags & WIPHY_FLAG_SUPPORTS_TDLS) ||
	    !(wiphy->flags & WIPHY_FLAG_TDLS_EXTERNAL_SETUP))
		return -ENOTSUPP;

	/* make sure we are in station mode and connected */
	if (!(priv->bss_type == MWIFIEX_BSS_TYPE_STA && priv->media_connected))
		return -ENOTSUPP;

	dev_dbg(priv->adapter->dev,
		"TDLS peer=%pM, oper=%d\n", peer, action);

	switch (action) {
	case NL80211_TDLS_ENABLE_LINK:
		action = MWIFIEX_TDLS_ENABLE_LINK;
		break;
	case NL80211_TDLS_DISABLE_LINK:
		action = MWIFIEX_TDLS_DISABLE_LINK;
		break;
	case NL80211_TDLS_TEARDOWN:
		/* shouldn't happen!*/
		dev_warn(priv->adapter->dev,
			 "tdls_oper: teardown from driver not supported\n");
		return -EINVAL;
	case NL80211_TDLS_SETUP:
		/* shouldn't happen!*/
		dev_warn(priv->adapter->dev,
			 "tdls_oper: setup from driver not supported\n");
		return -EINVAL;
	case NL80211_TDLS_DISCOVERY_REQ:
		/* shouldn't happen!*/
		dev_warn(priv->adapter->dev,
			 "tdls_oper: discovery from driver not supported\n");
		return -EINVAL;
	default:
		dev_err(priv->adapter->dev,
			"tdls_oper: operation not supported\n");
		return -ENOTSUPP;
	}

	return mwifiex_tdls_oper(priv, peer, action);
}

static int
mwifiex_cfg80211_add_station(struct wiphy *wiphy, struct net_device *dev,
			     const u8 *mac, struct station_parameters *params)
{
	struct mwifiex_private *priv = mwifiex_netdev_get_priv(dev);

	if (!(params->sta_flags_set & BIT(NL80211_STA_FLAG_TDLS_PEER)))
		return -ENOTSUPP;

	/* make sure we are in station mode and connected */
	if ((priv->bss_type != MWIFIEX_BSS_TYPE_STA) || !priv->media_connected)
		return -ENOTSUPP;

	return mwifiex_tdls_oper(priv, mac, MWIFIEX_TDLS_CREATE_LINK);
}

static int
mwifiex_cfg80211_channel_switch(struct wiphy *wiphy, struct net_device *dev,
				struct cfg80211_csa_settings *params)
{
	struct ieee_types_header *chsw_ie;
	struct ieee80211_channel_sw_ie *channel_sw;
	int chsw_msec;
	struct mwifiex_private *priv = mwifiex_netdev_get_priv(dev);

	if (priv->adapter->scan_processing) {
		dev_err(priv->adapter->dev,
			"radar detection: scan in process...\n");
		return -EBUSY;
	}

	if (priv->wdev.cac_started)
		return -EBUSY;

	if (cfg80211_chandef_identical(&params->chandef,
				       &priv->dfs_chandef))
		return -EINVAL;

	chsw_ie = (void *)cfg80211_find_ie(WLAN_EID_CHANNEL_SWITCH,
					   params->beacon_csa.tail,
					   params->beacon_csa.tail_len);
	if (!chsw_ie) {
		dev_err(priv->adapter->dev,
			"Could not parse channel switch announcement IE\n");
		return -EINVAL;
	}

	channel_sw = (void *)(chsw_ie + 1);
	if (channel_sw->mode) {
		if (netif_carrier_ok(priv->netdev))
			netif_carrier_off(priv->netdev);
		mwifiex_stop_net_dev_queue(priv->netdev, priv->adapter);
	}

	if (mwifiex_del_mgmt_ies(priv))
		wiphy_err(wiphy, "Failed to delete mgmt IEs!\n");

	if (mwifiex_set_mgmt_ies(priv, &params->beacon_csa)) {
		wiphy_err(wiphy, "%s: setting mgmt ies failed\n", __func__);
		return -EFAULT;
	}

	memcpy(&priv->dfs_chandef, &params->chandef, sizeof(priv->dfs_chandef));
	memcpy(&priv->beacon_after, &params->beacon_after,
	       sizeof(priv->beacon_after));

	chsw_msec = max(channel_sw->count * priv->bss_cfg.beacon_period, 100);
	queue_delayed_work(priv->dfs_chan_sw_workqueue, &priv->dfs_chan_sw_work,
			   msecs_to_jiffies(chsw_msec));
	return 0;
}

static int
mwifiex_cfg80211_start_radar_detection(struct wiphy *wiphy,
				       struct net_device *dev,
				       struct cfg80211_chan_def *chandef,
				       u32 cac_time_ms)
{
	struct mwifiex_private *priv = mwifiex_netdev_get_priv(dev);
	struct mwifiex_radar_params radar_params;

	if (priv->adapter->scan_processing) {
		dev_err(priv->adapter->dev,
			"radar detection: scan already in process...\n");
		return -EBUSY;
	}

	if (!mwifiex_is_11h_active(priv)) {
		dev_dbg(priv->adapter->dev, "Enable 11h extensions in FW\n");
		if (mwifiex_11h_activate(priv, true)) {
			dev_err(priv->adapter->dev,
				"Failed to activate 11h extensions!!");
			return -1;
		}
		priv->state_11h.is_11h_active = true;
	}

	memset(&radar_params, 0, sizeof(struct mwifiex_radar_params));
	radar_params.chandef = chandef;
	radar_params.cac_time_ms = cac_time_ms;

	memcpy(&priv->dfs_chandef, chandef, sizeof(priv->dfs_chandef));

	if (mwifiex_send_cmd(priv, HostCmd_CMD_CHAN_REPORT_REQUEST,
			     HostCmd_ACT_GEN_SET, 0, &radar_params, true))
		return -1;

	queue_delayed_work(priv->dfs_cac_workqueue, &priv->dfs_cac_work,
			   msecs_to_jiffies(cac_time_ms));
	return 0;
}

static int
mwifiex_cfg80211_change_station(struct wiphy *wiphy, struct net_device *dev,
				const u8 *mac,
				struct station_parameters *params)
{
	int ret;
	struct mwifiex_private *priv = mwifiex_netdev_get_priv(dev);

	/* we support change_station handler only for TDLS peers*/
	if (!(params->sta_flags_set & BIT(NL80211_STA_FLAG_TDLS_PEER)))
		return -ENOTSUPP;

	/* make sure we are in station mode and connected */
	if ((priv->bss_type != MWIFIEX_BSS_TYPE_STA) || !priv->media_connected)
		return -ENOTSUPP;

	priv->sta_params = params;

	ret = mwifiex_tdls_oper(priv, mac, MWIFIEX_TDLS_CONFIG_LINK);
	priv->sta_params = NULL;

	return ret;
}

/* station cfg80211 operations */
static struct cfg80211_ops mwifiex_cfg80211_ops = {
	.add_virtual_intf = mwifiex_add_virtual_intf,
	.del_virtual_intf = mwifiex_del_virtual_intf,
	.change_virtual_intf = mwifiex_cfg80211_change_virtual_intf,
	.scan = mwifiex_cfg80211_scan,
	.connect = mwifiex_cfg80211_connect,
	.disconnect = mwifiex_cfg80211_disconnect,
	.get_station = mwifiex_cfg80211_get_station,
	.dump_station = mwifiex_cfg80211_dump_station,
	.dump_survey = mwifiex_cfg80211_dump_survey,
	.set_wiphy_params = mwifiex_cfg80211_set_wiphy_params,
	.join_ibss = mwifiex_cfg80211_join_ibss,
	.leave_ibss = mwifiex_cfg80211_leave_ibss,
	.add_key = mwifiex_cfg80211_add_key,
	.del_key = mwifiex_cfg80211_del_key,
	.mgmt_tx = mwifiex_cfg80211_mgmt_tx,
	.mgmt_frame_register = mwifiex_cfg80211_mgmt_frame_register,
	.remain_on_channel = mwifiex_cfg80211_remain_on_channel,
	.cancel_remain_on_channel = mwifiex_cfg80211_cancel_remain_on_channel,
	.set_default_key = mwifiex_cfg80211_set_default_key,
	.set_power_mgmt = mwifiex_cfg80211_set_power_mgmt,
	.set_tx_power = mwifiex_cfg80211_set_tx_power,
	.set_bitrate_mask = mwifiex_cfg80211_set_bitrate_mask,
	.start_ap = mwifiex_cfg80211_start_ap,
	.stop_ap = mwifiex_cfg80211_stop_ap,
	.change_beacon = mwifiex_cfg80211_change_beacon,
	.set_cqm_rssi_config = mwifiex_cfg80211_set_cqm_rssi_config,
	.set_antenna = mwifiex_cfg80211_set_antenna,
	.del_station = mwifiex_cfg80211_del_station,
#ifdef CONFIG_PM
	.suspend = mwifiex_cfg80211_suspend,
	.resume = mwifiex_cfg80211_resume,
	.set_wakeup = mwifiex_cfg80211_set_wakeup,
#endif
	.set_coalesce = mwifiex_cfg80211_set_coalesce,
	.tdls_mgmt = mwifiex_cfg80211_tdls_mgmt,
	.tdls_oper = mwifiex_cfg80211_tdls_oper,
	.add_station = mwifiex_cfg80211_add_station,
	.change_station = mwifiex_cfg80211_change_station,
	.start_radar_detection = mwifiex_cfg80211_start_radar_detection,
	.channel_switch = mwifiex_cfg80211_channel_switch,
};

#ifdef CONFIG_PM
static const struct wiphy_wowlan_support mwifiex_wowlan_support = {
	.flags = WIPHY_WOWLAN_MAGIC_PKT | WIPHY_WOWLAN_DISCONNECT,
	.n_patterns = MWIFIEX_MEF_MAX_FILTERS,
	.pattern_min_len = 1,
	.pattern_max_len = MWIFIEX_MAX_PATTERN_LEN,
	.max_pkt_offset = MWIFIEX_MAX_OFFSET_LEN,
};
#endif

static bool mwifiex_is_valid_alpha2(const char *alpha2)
{
	if (!alpha2 || strlen(alpha2) != 2)
		return false;

	if (isalpha(alpha2[0]) && isalpha(alpha2[1]))
		return true;

	return false;
}

static const struct wiphy_coalesce_support mwifiex_coalesce_support = {
	.n_rules = MWIFIEX_COALESCE_MAX_RULES,
	.max_delay = MWIFIEX_MAX_COALESCING_DELAY,
	.n_patterns = MWIFIEX_COALESCE_MAX_FILTERS,
	.pattern_min_len = 1,
	.pattern_max_len = MWIFIEX_MAX_PATTERN_LEN,
	.max_pkt_offset = MWIFIEX_MAX_OFFSET_LEN,
};

int mwifiex_init_channel_scan_gap(struct mwifiex_adapter *adapter)
{
	u32 n_channels_bg, n_channels_a = 0;

	n_channels_bg = mwifiex_band_2ghz.n_channels;

	if (adapter->config_bands & BAND_A)
		n_channels_a = mwifiex_band_5ghz.n_channels;

	adapter->num_in_chan_stats = max_t(u32, n_channels_bg, n_channels_a);
	adapter->chan_stats = vmalloc(sizeof(*adapter->chan_stats) *
				      adapter->num_in_chan_stats);

	if (!adapter->chan_stats)
		return -ENOMEM;

	return 0;
}

/*
 * This function registers the device with CFG802.11 subsystem.
 *
 * The function creates the wireless device/wiphy, populates it with
 * default parameters and handler function pointers, and finally
 * registers the device.
 */

int mwifiex_register_cfg80211(struct mwifiex_adapter *adapter)
{
	int ret;
	void *wdev_priv;
	struct wiphy *wiphy;
	struct mwifiex_private *priv = adapter->priv[MWIFIEX_BSS_TYPE_STA];
	u8 *country_code;
	u32 thr, retry;

	/* create a new wiphy for use with cfg80211 */
	wiphy = wiphy_new(&mwifiex_cfg80211_ops,
			  sizeof(struct mwifiex_adapter *));
	if (!wiphy) {
		dev_err(adapter->dev, "%s: creating new wiphy\n", __func__);
		return -ENOMEM;
	}
	wiphy->max_scan_ssids = MWIFIEX_MAX_SSID_LIST_LENGTH;
	wiphy->max_scan_ie_len = MWIFIEX_MAX_VSIE_LEN;
	wiphy->mgmt_stypes = mwifiex_mgmt_stypes;
	wiphy->max_remain_on_channel_duration = 5000;
	wiphy->interface_modes = BIT(NL80211_IFTYPE_STATION) |
				 BIT(NL80211_IFTYPE_ADHOC) |
				 BIT(NL80211_IFTYPE_P2P_CLIENT) |
				 BIT(NL80211_IFTYPE_P2P_GO) |
				 BIT(NL80211_IFTYPE_AP);

	wiphy->bands[IEEE80211_BAND_2GHZ] = &mwifiex_band_2ghz;
	if (adapter->config_bands & BAND_A)
		wiphy->bands[IEEE80211_BAND_5GHZ] = &mwifiex_band_5ghz;
	else
		wiphy->bands[IEEE80211_BAND_5GHZ] = NULL;

	wiphy->iface_combinations = &mwifiex_iface_comb_ap_sta;
	wiphy->n_iface_combinations = 1;

	/* Initialize cipher suits */
	wiphy->cipher_suites = mwifiex_cipher_suites;
	wiphy->n_cipher_suites = ARRAY_SIZE(mwifiex_cipher_suites);

	ether_addr_copy(wiphy->perm_addr, adapter->perm_addr);
	wiphy->signal_type = CFG80211_SIGNAL_TYPE_MBM;
	wiphy->flags |= WIPHY_FLAG_HAVE_AP_SME |
			WIPHY_FLAG_AP_PROBE_RESP_OFFLOAD |
			WIPHY_FLAG_AP_UAPSD |
			WIPHY_FLAG_HAS_REMAIN_ON_CHANNEL |
			WIPHY_FLAG_HAS_CHANNEL_SWITCH;

	if (ISSUPP_TDLS_ENABLED(adapter->fw_cap_info))
		wiphy->flags |= WIPHY_FLAG_SUPPORTS_TDLS |
				WIPHY_FLAG_TDLS_EXTERNAL_SETUP;

#ifdef CONFIG_PM
	wiphy->wowlan = &mwifiex_wowlan_support;
#endif

	wiphy->coalesce = &mwifiex_coalesce_support;

	wiphy->probe_resp_offload = NL80211_PROBE_RESP_OFFLOAD_SUPPORT_WPS |
				    NL80211_PROBE_RESP_OFFLOAD_SUPPORT_WPS2 |
				    NL80211_PROBE_RESP_OFFLOAD_SUPPORT_P2P;

	wiphy->available_antennas_tx = BIT(adapter->number_of_antenna) - 1;
	wiphy->available_antennas_rx = BIT(adapter->number_of_antenna) - 1;

	wiphy->features |= NL80211_FEATURE_HT_IBSS |
			   NL80211_FEATURE_INACTIVITY_TIMER |
			   NL80211_FEATURE_NEED_OBSS_SCAN;

	if (adapter->fw_api_ver == MWIFIEX_FW_V15)
		wiphy->features |= NL80211_FEATURE_SK_TX_STATUS;

	/* Reserve space for mwifiex specific private data for BSS */
	wiphy->bss_priv_size = sizeof(struct mwifiex_bss_priv);

	wiphy->reg_notifier = mwifiex_reg_notifier;

	/* Set struct mwifiex_adapter pointer in wiphy_priv */
	wdev_priv = wiphy_priv(wiphy);
	*(unsigned long *)wdev_priv = (unsigned long)adapter;

	set_wiphy_dev(wiphy, priv->adapter->dev);

	ret = wiphy_register(wiphy);
	if (ret < 0) {
		dev_err(adapter->dev,
			"%s: wiphy_register failed: %d\n", __func__, ret);
		wiphy_free(wiphy);
		return ret;
	}

	if (reg_alpha2 && mwifiex_is_valid_alpha2(reg_alpha2)) {
		wiphy_info(wiphy, "driver hint alpha2: %2.2s\n", reg_alpha2);
		regulatory_hint(wiphy, reg_alpha2);
	} else {
		country_code = mwifiex_11d_code_2_region(adapter->region_code);
		if (country_code)
			wiphy_info(wiphy, "ignoring F/W country code %2.2s\n",
				   country_code);
	}

	mwifiex_send_cmd(priv, HostCmd_CMD_802_11_SNMP_MIB,
			 HostCmd_ACT_GEN_GET, FRAG_THRESH_I, &thr, true);
	wiphy->frag_threshold = thr;
	mwifiex_send_cmd(priv, HostCmd_CMD_802_11_SNMP_MIB,
			 HostCmd_ACT_GEN_GET, RTS_THRESH_I, &thr, true);
	wiphy->rts_threshold = thr;
	mwifiex_send_cmd(priv, HostCmd_CMD_802_11_SNMP_MIB,
			 HostCmd_ACT_GEN_GET, SHORT_RETRY_LIM_I, &retry, true);
	wiphy->retry_short = (u8) retry;
	mwifiex_send_cmd(priv, HostCmd_CMD_802_11_SNMP_MIB,
			 HostCmd_ACT_GEN_GET, LONG_RETRY_LIM_I, &retry, true);
	wiphy->retry_long = (u8) retry;

	adapter->wiphy = wiphy;
	return ret;
}
