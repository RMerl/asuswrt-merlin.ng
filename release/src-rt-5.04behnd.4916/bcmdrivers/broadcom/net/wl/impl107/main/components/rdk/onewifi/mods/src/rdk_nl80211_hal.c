/*
 * The Linux nl80211 WIFI HAL device driver
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
 * <<Broadcom-WL-IPTag/Open:>>
 *
 * $Id: $
 */
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>

#include <linux/netdevice.h>
#include <linux/nl80211.h>
#include <net/netlink.h>
#include <net/cfg80211.h>

#include <wlioctl.h>
#include <bcmutils.h>
#include <bcmwifi_rspec.h>
#include <bcmwifi_channels.h>
#include "rdk_nl80211_hal.h"
#include <wldev_common.h>
#include <wl_cfg80211.h>
#include <wl_cfgvendor_common.h>

#define WL_BRCM_STA     0x00000001

#ifdef BCM_WLMLO
#include "wlhal_lib_mld.h"
#endif /* BCM_WLMLO */

#include "rdk_nl80211_priv.h"

u32 wl_dbg_level = WL_DBG_ERR;
module_param(wl_dbg_level, int, 0);

static const struct nla_policy
wl_cfgvendor_get_station_policy[RDK_VENDOR_ATTR_MAX + 1] = {
	[RDK_VENDOR_ATTR_MAC] = NLA_POLICY_EXACT_LEN(ETH_ALEN),
};

static const struct nla_policy
wl_cfgvendor_get_station_list_policy[RDK_VENDOR_ATTR_MAX + 1] = {
};

static const struct nla_policy
wl_cfgvendor_get_phy_temp_policy[RDK_VENDOR_ATTR_MAX + 1] = {
};

static const struct nla_policy
wl_cfgvendor_get_survey_policy[RDK_VENDOR_ATTR_MAX + 1] = {
};

static const struct nla_policy
wl_cfgvendor_set_mgt_frame_pwr_policy[RDK_VENDOR_ATTR_MAX + 1] = {
	[RDK_VENDOR_ATTR_MGT_FRAME_PWR_LEVEL] = { .type = NLA_S32 },
};

#ifdef BCM_WLMLO
static const struct nla_policy
wl_cfgvendor_set_mld_policy[RDK_VENDOR_ATTR_MAX + 1] = {
	[RDK_VENDOR_ATTR_MLD_ENABLE] = { .type = NLA_U8 },
	[RDK_VENDOR_ATTR_MLD_UNIT] = { .type = NLA_U8 },
	[RDK_VENDOR_ATTR_MLD_LINK_ID] = { .type = NLA_U8 },
	[RDK_VENDOR_ATTR_MLD_MAC] = NLA_POLICY_EXACT_LEN(ETH_ALEN),
	[RDK_VENDOR_ATTR_MLD_CONFIG_APPLY] = { .type = NLA_U8 },
};
#endif /* BCM_WLMLO */

static const struct nla_policy
wl_cfgvendor_add_del_csi_station_policy[RDK_VENDOR_ATTR_MAX + 1] = {
	[RDK_VENDOR_ATTR_CSI_ENABLE] = { .type = NLA_U8 },
	[RDK_VENDOR_ATTR_MAC] = NLA_POLICY_EXACT_LEN(ETH_ALEN),
};

static const struct nla_policy
wl_cfgvendor_get_csi_station_policy[RDK_VENDOR_ATTR_MAX + 1] = {
};

static const struct nla_policy
wl_cfgvendor_get_csi_policy[RDK_VENDOR_ATTR_MAX + 1] = {
};

static const struct nla_policy
wl_cfgvendor_set_csi_policy[RDK_VENDOR_ATTR_MAX + 1] = {
	[RDK_VENDOR_ATTR_CSI_ENABLE] = { .type = NLA_U8 },
};

static const struct nla_policy
wl_cfgvendor_msglevel_policy[RDK_VENDOR_ATTR_MAX + 1] = {
	[RDK_VENDOR_ATTR_MSGLEVEL] = { .type = NLA_U32 },
};

static int rdk_csi_event_idx = 0;
u32 rdk_msglevel = RDK_CSI_LOG_ERROR;

static int
wl_cfgvendor_get_band_ioctl(struct wiphy *wiphy, struct wireless_dev *wdev, uint *band)
{
	int ret;
	struct net_device *ndev;
	struct bcm_cfg80211 *cfg = wiphy_priv(wiphy);

	ndev = wdev_to_wlc_ndev(wdev, cfg);
	if (!ndev) {
		WL_ERR(("Failed to get ndev\n"));
		return BCME_ERROR;
	}

	ret = wldev_ioctl_get(ndev, WLC_GET_BAND, &band, sizeof(band));
	if (ret < 0) {
		WL_ERR(("Get band ioctl failed, error: %d\n", ret));
		return ret;
	}

	return ret;
}

static int
wl_cfgvendor_is_ofdm_rate(int rate)
{
	int i, rateset_ofdm[] = {6, 9, 12, 18, 24, 36, 48, 54};

	for (i = 0; i < sizeof(rateset_ofdm) / sizeof(rateset_ofdm[0]); i++) {
		if (rateset_ofdm[i] != rate)
			return 0;
	}

	return 1;
}

static int
wl_cfgvendor_get_80211_standard(struct wiphy *wiphy, struct wireless_dev *wdev,
	sta_info_t *sta_info, int *standard)
{
	int i, ret;
	uint band;

	ret = wl_cfgvendor_get_band_ioctl(wiphy, wdev, &band);
	if (ret < 0) {
		WL_ERR(("Get band failed, error: %d\n", ret));
		return ret;
	}

	*standard = (sta_info->flags & WL_STA_EHT_CAP) ? RDK_VENDOR_NL80211_STANDARD_BE :
		(sta_info->flags & WL_STA_HE_CAP) ? RDK_VENDOR_NL80211_STANDARD_AX :
		(sta_info->flags & WL_STA_VHT_CAP) ? RDK_VENDOR_NL80211_STANDARD_AC :
		(sta_info->flags & WL_STA_N_CAP) ? RDK_VENDOR_NL80211_STANDARD_N :
		(band == WLC_BAND_2G) ? RDK_VENDOR_NL80211_STANDARD_G :
		RDK_VENDOR_NL80211_STANDARD_A;

	if (*standard != RDK_VENDOR_NL80211_STANDARD_G)
		return 0;

	for (i = 0; i < sta_info->rateset.count; i++) {
		if (!wl_cfgvendor_is_ofdm_rate(sta_info->rateset.rates[i] / 2)) {
			*standard = RDK_VENDOR_NL80211_STANDARD_B;
			return 0;
		}
	}

	return 0;
}

static int
wl_cfgvendor_get_phy_temp_ioctl(struct wiphy *wiphy, struct wireless_dev *wdev,
	int32 *phy_temp)
{
	int ret;
	struct net_device *ndev;
	struct bcm_cfg80211 *cfg = wiphy_priv(wiphy);

	ndev = wdev_to_wlc_ndev(wdev, cfg);
	if (!ndev) {
		WL_ERR(("Failed to get ndev\n"));
		return BCME_ERROR;
	}

	ret = wldev_iovar_getint(ndev, "phy_tempsense", phy_temp);
	if (ret < 0) {
		WL_ERR(("phy_tempsense ioctl failed, error: %d\n", ret));
		return ret;
	}

	return ret;
}

static int
wl_cfgvendor_get_phy_temp_reply(struct wiphy *wiphy, struct wireless_dev *wdev,
	int32 phy_temp)
{
	int ret;
	uint32 mem_needed;
	struct sk_buff *skb;

	mem_needed = VENDOR_REPLY_OVERHEAD + ATTRIBUTE_U32_LEN;

	skb = cfg80211_vendor_cmd_alloc_reply_skb(wiphy, mem_needed);
	if (!skb) {
		WL_ERR(("Failed to allocate skb\n"));
		return -ENOMEM;
	}

	ret = nla_put_s32(skb, RDK_VENDOR_ATTR_WIPHY_TEMP, phy_temp);
	if (ret < 0) {
		WL_ERR(("Failed to add phy temp attribute, error: %d\n", ret));
		goto error;
	}

	ret = cfg80211_vendor_cmd_reply(skb);
	if (ret < 0) {
		WL_ERR(("Failed to send reply, error: %d\n", ret));
	}
	return ret;

error:
	kfree_skb(skb);
	return ret;
}

static int
wl_cfgvendor_get_phy_temp_handler(struct wiphy *wiphy, struct wireless_dev *wdev,
	const void *data, int len)
{
	int32 ret, phy_temp;

	ret = wl_cfgvendor_get_phy_temp_ioctl(wiphy, wdev, &phy_temp);
	if (ret < 0) {
		WL_ERR(("Failed to get phy temperature, error: %d\n", ret));
		return ret;
	}

	ret = wl_cfgvendor_get_phy_temp_reply(wiphy, wdev, phy_temp);
	if (ret < 0) {
		WL_ERR(("Failed to send phy temperature, error: %d\n", ret));
		return ret;
	}

	return 0;
}

static inline int
wl_cfgvendor_get_station_rssi(sta_info_t *sta_info, int32 *rssi)
{
	int i;

	*rssi = -255;
	for (i = 0; i < WL_STA_ANT_MAX; i++) {
		if (sta_info->rssi[i] != 0 && sta_info->rssi[i] > *rssi) {
			*rssi = sta_info->rssi[i];
		}
	}

	return 0;
}

static int
wl_cfgvendor_get_station_bw(sta_info_t *sta_info, u8 *bw)
{
	switch (sta_info->link_bw) {
	case BW_20MHZ: *bw = RDK_VENDOR_NL80211_CHAN_WIDTH_20; break;
	case BW_40MHZ: *bw = RDK_VENDOR_NL80211_CHAN_WIDTH_20; break;
	case BW_80MHZ: *bw = RDK_VENDOR_NL80211_CHAN_WIDTH_80; break;
	case BW_160MHZ:	*bw = RDK_VENDOR_NL80211_CHAN_WIDTH_160; break;
	case BW_320MHZ: *bw = RDK_VENDOR_NL80211_CHAN_WIDTH_320; break;
	default: *bw = 0; break;
	}

	return 0;
}

static int
wl_cfgvendor_get_assoc_num(struct wireless_dev *wdev, struct ether_addr *mac, uint64 *assoc_count)
{
	struct file *fp;
	char *buf, *line;
	int ret, fidx, fsub_idx, idx, sub_idx;
	char fmac[ETHER_ADDR_STR_LEN], mac_str[ETHER_ADDR_STR_LEN];
	unsigned long long fassoc_count = 0;

	ret = sscanf(wdev->netdev->name, "wl%d.%d", &idx, &sub_idx);
	if (ret != 2) {
		WL_ERR(("Failed to parse dev name, error: %d\n", ret));
		return -1;
	}

	fp = filp_open(STA_ASSOC_COUNT_FILE, O_RDONLY, 0);
	if (IS_ERR(fp)) {
		WL_ERR(("Failed to open %s file, error: %ld\n", STA_ASSOC_COUNT_FILE, PTR_ERR(fp)));
		return PTR_ERR(fp);
	}

	buf = kzalloc(STA_ASSOC_COUNT_BUF, GFP_KERNEL);
	if (!buf) {
		WL_ERR(("Failed to allocate buffer\n"));
		ret = -ENOMEM;
		goto exit;
	}

	ret = kernel_read(fp, buf, STA_ASSOC_COUNT_BUF, &fp->f_pos);
	if (ret <= 0) {
		WL_ERR(("Failed to read %s file, error: %d\n", STA_ASSOC_COUNT_FILE, ret));
		goto exit;
	}

	if (ret == STA_ASSOC_COUNT_BUF) {
		WL_ERR(("Failed to read whole file %s\n", STA_ASSOC_COUNT_FILE));
		ret = -1;
		goto exit;
	}

	bcm_ether_ntoa(mac, mac_str);
	*assoc_count = 0;
	line = buf;
	while (line) {
		ret = sscanf(line, "%d %d %"__stringify(ETHER_ADDR_STR_LEN)"s %llu\n", &fidx,
			&fsub_idx, fmac, &fassoc_count);
		if (ret != 4) {
			break;
		}

		if (idx == fidx && sub_idx == fsub_idx &&
			!strncasecmp(fmac, mac_str, sizeof(fmac))) {
			*assoc_count = fassoc_count;
			break;
		}

		line = strchr(line, '\n');
		if (line)
			line += 1;
	}

	ret = 0;
exit:
	kfree(buf);
	filp_close(fp, NULL);
	return ret;
}

static int
wl_cfgvendor_get_spatial_stream_num(sta_info_t *sta_info, u8 *nss)
{
	int i;
	unsigned int tx_mcs, rx_mcs;
	u16 he_txmcsmap, he_rxmcsmap, mcs_count;

	mcs_count = 0;

	if ((sta_info->flags & WL_STA_HE_CAP) &&
		(sta_info->rateset_adv.he_mcs[0] != 0xffff)) {
		he_txmcsmap = dtoh16(sta_info->rateset_adv.he_mcs[0]);
		he_rxmcsmap = dtoh16(sta_info->rateset_adv.he_mcs[1]);
		for (i = 1; i <= HE_CAP_MCS_MAP_NSS_MAX; i++) {
			tx_mcs = HE_CAP_MAX_MCS_NSS_GET_MCS(i, he_txmcsmap);
			rx_mcs = HE_CAP_MAX_MCS_NSS_GET_MCS(i, he_rxmcsmap);
			if (tx_mcs != HE_CAP_MAX_MCS_NONE || rx_mcs != HE_CAP_MAX_MCS_NONE) {
				mcs_count++;
			}
		}
	} else if (sta_info->flags & WL_STA_VHT_CAP) {
		for (i = 0; i < VHT_CAP_MCS_MAP_NSS_MAX; i++) {
			if (sta_info->rateset_adv.vht_mcs[i]) {
				mcs_count++;
			} else {
				break;
			}
		}
	} else {
		for (i = 0; i < (MCSSET_LEN * 8); i++) {
			if (isset(&sta_info->rateset_adv.mcs[0], i))
				mcs_count++;
		}
		if (mcs_count != 0)
			mcs_count = mcs_count / 8;
	}

	if (mcs_count == 0)
		mcs_count = 8;

	*nss = mcs_count;

	return 0;
}

static int
wl_cfgvendor_get_nrate_ioctl(struct wiphy *wiphy, struct wireless_dev *wdev,
	unsigned int *nrate)
{
	int ret;
	struct net_device *ndev;

	struct bcm_cfg80211 *cfg = wiphy_priv(wiphy);

	ndev = wdev_to_wlc_ndev(wdev, cfg);
	if (!ndev) {
		WL_ERR(("Failed to get ndev\n"));
		return BCME_ERROR;
	}
	ret = wldev_iovar_getint(ndev, "nrate", (s32 *)nrate);
	if (ret < 0) {
		WL_ERR(("nrate ioctl failed, error: %d\n", ret));
	}
	return ret;
}

typedef struct rspec_gi_map {
	int rspec;
	int gi;
} rspec_gi_map_t;

typedef enum guard_interval {
	guard_interval_400 =	0x01,
	guard_interval_800 =	0x02,
	guard_interval_1600 =	0x04,
	guard_interval_3200 =	0x08,
	guard_interval_auto =	0x10
} guard_interval_t;

static rspec_gi_map_t rspec_gi_map[] = {
	/* hegi=0, sgi=0,  wldmgi=0x10 all */
	{WL_RSPEC_GI_AUTO,		guard_interval_auto},
	/* hegi=0, sgi=1, wldmgi=0x01 none he */
	{WL_RSPEC_SGI,			guard_interval_400},
	/* hegi=0, sgi=0, wldmgi=0x02 HE_1x_LTF */
	{WL_RSPEC_HE_1x_LTF_GI_0_8us,	guard_interval_800},
	/* hegi=1, sgi=0, wldmgi=0x02 HE_2x_LTF */
	{WL_RSPEC_HE_2x_LTF_GI_0_8us,	guard_interval_800},
	/* hegi=2, sgi=0, wldmgi=0x04 HE_3x_LTF */
	{WL_RSPEC_HE_2x_LTF_GI_1_6us,	guard_interval_1600},
	/* hegi=3, sgi=0, wldmgi=0x08 HE_4x_LTF */
	{WL_RSPEC_HE_4x_LTF_GI_3_2us,	guard_interval_3200}
};

static int
wl_cfgvendor_get_guard_interval(struct wiphy *wiphy, struct wireless_dev *wdev, unsigned int *gi)
{
	int ret, i, rspec_gi = 0;
	unsigned int rspec, encode, is_he_gi, offset = 0;

	ret = wl_cfgvendor_get_nrate_ioctl(wiphy, wdev, &rspec);
	if (ret < 0) {
		WL_ERR(("Failed to get rspec, error: %d\n", ret));
		return ret;
	}

	encode = rspec & WL_RSPEC_ENCODING_MASK;
	is_he_gi = (encode == WL_RSPEC_ENCODE_HE) ? 1 : 0;
	if (is_he_gi) {
		rspec_gi |= RSPEC_HE_LTF_GI(rspec);
		offset = HEGI_OFFSET;
	} else if (RSPEC_ISSGI(rspec)) {
		rspec_gi |= WL_RSPEC_SGI;
	}

	for (i = offset; i < ARRAY_SIZE(rspec_gi_map); i++) {
		if (rspec_gi_map[i].rspec == rspec_gi) {
			*gi = rspec_gi_map[i].gi;
			break;
		}
	}

	if (i == ARRAY_SIZE(rspec_gi_map)) {
		WL_ERR(("Invalid guard interval: 0x%x\n", rspec_gi));
		return -1;
	}

	if (!(rspec & (WL_RSPEC_OVERRIDE_RATE | WL_RSPEC_OVERRIDE_MODE))) {
		*gi = guard_interval_auto;
	}

	return 0;
}

typedef struct wifi_params_mcs_rate {
	unsigned int	oper_standard;	/* Operating standard - n/ac/ax */
	unsigned int	bw;		/* bandwidth 20/40/80/160 */
	int		gi;		/* guard interval 400nsec/Auto = 1 800nsec = -1 */
	unsigned int	nss;		/* NSS */
	unsigned int	rate;		/* Max uplink/downlink rate */
} wifi_params_mcs_rate_t;

/* Values in the table below are from
 * N: 802.11 spec Section 19.5 Parameters for HT MCSs Pages 2427 - 2431
 * AC: 802.11 spec Section 21.5 Parameters for VHT-MCSs Pages: 2608 - 2624
 * AC max rates accounting for mcs 11 based on
 * nss:4 - 20Mhz LGI - 433, 20Mhz SGI - 481, 40Mhz LGI - 900, 40Mhz SGI - 1000,
 * 80Mhz LGI - 1950, 80Mhz SGI - 21667. Calculate nssx_rate = nss4_rate*x/4
 * AX: Draft IEEE P802.11ax/D6.0 Section 27.5 Parameters for HE-MCSs Pages: 702 - 721
 * BE: Draft IEEE P802.11be/D5.0 Section 36.5 Parameters for EHT-MCSs Pages: 923 - 933
 */
wifi_params_mcs_rate_t mcs_rate_tbl[] = {
	/* Standard,	BW,	GI,	NSS,	Rate */
	{RDK_VENDOR_NL80211_STANDARD_AC,	RDK_VENDOR_NL80211_CHAN_WIDTH_20,	1,	1,	120},
	{RDK_VENDOR_NL80211_STANDARD_AC,	RDK_VENDOR_NL80211_CHAN_WIDTH_20,	1,	2,	241},
	{RDK_VENDOR_NL80211_STANDARD_AC,	RDK_VENDOR_NL80211_CHAN_WIDTH_20,	1,	3,	361},
	{RDK_VENDOR_NL80211_STANDARD_AC,	RDK_VENDOR_NL80211_CHAN_WIDTH_20,	1,	4,	481},
	{RDK_VENDOR_NL80211_STANDARD_AC,	RDK_VENDOR_NL80211_CHAN_WIDTH_20,	1,	5,	601},
	{RDK_VENDOR_NL80211_STANDARD_AC,	RDK_VENDOR_NL80211_CHAN_WIDTH_20,	1,	6,	722},
	{RDK_VENDOR_NL80211_STANDARD_AC,	RDK_VENDOR_NL80211_CHAN_WIDTH_20,	1,	7,	842},
	{RDK_VENDOR_NL80211_STANDARD_AC,	RDK_VENDOR_NL80211_CHAN_WIDTH_20,	1,	8,	962},

	{RDK_VENDOR_NL80211_STANDARD_AC,	RDK_VENDOR_NL80211_CHAN_WIDTH_20,	-1,	1,	108},
	{RDK_VENDOR_NL80211_STANDARD_AC,	RDK_VENDOR_NL80211_CHAN_WIDTH_20,	-1,	2,	217},
	{RDK_VENDOR_NL80211_STANDARD_AC,	RDK_VENDOR_NL80211_CHAN_WIDTH_20,	-1,	3,	325},
	{RDK_VENDOR_NL80211_STANDARD_AC,	RDK_VENDOR_NL80211_CHAN_WIDTH_20,	-1,	4,	433},
	{RDK_VENDOR_NL80211_STANDARD_AC,	RDK_VENDOR_NL80211_CHAN_WIDTH_20,	-1,	5,	541},
	{RDK_VENDOR_NL80211_STANDARD_AC,	RDK_VENDOR_NL80211_CHAN_WIDTH_20,	-1,	6,	650},
	{RDK_VENDOR_NL80211_STANDARD_AC,	RDK_VENDOR_NL80211_CHAN_WIDTH_20,	-1,	7,	758},
	{RDK_VENDOR_NL80211_STANDARD_AC,	RDK_VENDOR_NL80211_CHAN_WIDTH_20,	-1,	8,	866},

	{RDK_VENDOR_NL80211_STANDARD_AC,	RDK_VENDOR_NL80211_CHAN_WIDTH_40,	1,	1,	250},
	{RDK_VENDOR_NL80211_STANDARD_AC,	RDK_VENDOR_NL80211_CHAN_WIDTH_40,	1,	2,	500},
	{RDK_VENDOR_NL80211_STANDARD_AC,	RDK_VENDOR_NL80211_CHAN_WIDTH_40,	1,	3,	750},
	{RDK_VENDOR_NL80211_STANDARD_AC,	RDK_VENDOR_NL80211_CHAN_WIDTH_40,	1,	4,	1000},
	{RDK_VENDOR_NL80211_STANDARD_AC,	RDK_VENDOR_NL80211_CHAN_WIDTH_40,	1,	5,	1250},
	{RDK_VENDOR_NL80211_STANDARD_AC,	RDK_VENDOR_NL80211_CHAN_WIDTH_40,	1,	6,	1500},
	{RDK_VENDOR_NL80211_STANDARD_AC,	RDK_VENDOR_NL80211_CHAN_WIDTH_40,	1,	7,	1750},
	{RDK_VENDOR_NL80211_STANDARD_AC,	RDK_VENDOR_NL80211_CHAN_WIDTH_40,	1,	8,	2000},

	{RDK_VENDOR_NL80211_STANDARD_AC,	RDK_VENDOR_NL80211_CHAN_WIDTH_40,	-1,	1,	225},
	{RDK_VENDOR_NL80211_STANDARD_AC,	RDK_VENDOR_NL80211_CHAN_WIDTH_40,	-1,	2,	450},
	{RDK_VENDOR_NL80211_STANDARD_AC,	RDK_VENDOR_NL80211_CHAN_WIDTH_40,	-1,	3,	675},
	{RDK_VENDOR_NL80211_STANDARD_AC,	RDK_VENDOR_NL80211_CHAN_WIDTH_40,	-1,	4,	900},
	{RDK_VENDOR_NL80211_STANDARD_AC,	RDK_VENDOR_NL80211_CHAN_WIDTH_40,	-1,	5,	1125},
	{RDK_VENDOR_NL80211_STANDARD_AC,	RDK_VENDOR_NL80211_CHAN_WIDTH_40,	-1,	6,	1350},
	{RDK_VENDOR_NL80211_STANDARD_AC,	RDK_VENDOR_NL80211_CHAN_WIDTH_40,	-1,	7,	1575},
	{RDK_VENDOR_NL80211_STANDARD_AC,	RDK_VENDOR_NL80211_CHAN_WIDTH_40,	-1,	8,	1800},

	{RDK_VENDOR_NL80211_STANDARD_AC,	RDK_VENDOR_NL80211_CHAN_WIDTH_80,	1,	1,	542},
	{RDK_VENDOR_NL80211_STANDARD_AC,	RDK_VENDOR_NL80211_CHAN_WIDTH_80,	1,	2,	1084},
	{RDK_VENDOR_NL80211_STANDARD_AC,	RDK_VENDOR_NL80211_CHAN_WIDTH_80,	1,	3,	1625},
	{RDK_VENDOR_NL80211_STANDARD_AC,	RDK_VENDOR_NL80211_CHAN_WIDTH_80,	1,	4,	2167},
	{RDK_VENDOR_NL80211_STANDARD_AC,	RDK_VENDOR_NL80211_CHAN_WIDTH_80,	1,	5,	2709},
	{RDK_VENDOR_NL80211_STANDARD_AC,	RDK_VENDOR_NL80211_CHAN_WIDTH_80,	1,	6,	3251},
	{RDK_VENDOR_NL80211_STANDARD_AC,	RDK_VENDOR_NL80211_CHAN_WIDTH_80,	1,	7,	3792},
	{RDK_VENDOR_NL80211_STANDARD_AC,	RDK_VENDOR_NL80211_CHAN_WIDTH_80,	1,	8,	4334},

	{RDK_VENDOR_NL80211_STANDARD_AC,	RDK_VENDOR_NL80211_CHAN_WIDTH_80,	-1,	1,	488},
	{RDK_VENDOR_NL80211_STANDARD_AC,	RDK_VENDOR_NL80211_CHAN_WIDTH_80,	-1,	2,	975},
	{RDK_VENDOR_NL80211_STANDARD_AC,	RDK_VENDOR_NL80211_CHAN_WIDTH_80,	-1,	3,	1463},
	{RDK_VENDOR_NL80211_STANDARD_AC,	RDK_VENDOR_NL80211_CHAN_WIDTH_80,	-1,	4,	1950},
	{RDK_VENDOR_NL80211_STANDARD_AC,	RDK_VENDOR_NL80211_CHAN_WIDTH_80,	-1,	5,	2438},
	{RDK_VENDOR_NL80211_STANDARD_AC,	RDK_VENDOR_NL80211_CHAN_WIDTH_80,	-1,	6,	2925},
	{RDK_VENDOR_NL80211_STANDARD_AC,	RDK_VENDOR_NL80211_CHAN_WIDTH_80,	-1,	7,	3413},
	{RDK_VENDOR_NL80211_STANDARD_AC,	RDK_VENDOR_NL80211_CHAN_WIDTH_80,	-1,	8,	3900},
};

wifi_params_mcs_rate_t non_brcm_mcs_rate_tbl[] = {
	/* Standard,	BW,	GI,	NSS,	Rate */
	{RDK_VENDOR_NL80211_STANDARD_N,	RDK_VENDOR_NL80211_CHAN_WIDTH_20,	1,	1,	72},
	{RDK_VENDOR_NL80211_STANDARD_N,	RDK_VENDOR_NL80211_CHAN_WIDTH_20,	1,	2,	144},
	{RDK_VENDOR_NL80211_STANDARD_N,	RDK_VENDOR_NL80211_CHAN_WIDTH_20,	1,	3,	217},
	{RDK_VENDOR_NL80211_STANDARD_N,	RDK_VENDOR_NL80211_CHAN_WIDTH_20,	1,	4,	289},

	{RDK_VENDOR_NL80211_STANDARD_N,	RDK_VENDOR_NL80211_CHAN_WIDTH_20,	-1,	1,	65},
	{RDK_VENDOR_NL80211_STANDARD_N,	RDK_VENDOR_NL80211_CHAN_WIDTH_20,	-1,	2,	130},
	{RDK_VENDOR_NL80211_STANDARD_N,	RDK_VENDOR_NL80211_CHAN_WIDTH_20,	-1,	3,	195},
	{RDK_VENDOR_NL80211_STANDARD_N,	RDK_VENDOR_NL80211_CHAN_WIDTH_20,	-1,	4,	260},

	{RDK_VENDOR_NL80211_STANDARD_N,	RDK_VENDOR_NL80211_CHAN_WIDTH_40,	1,	1,	150},
	{RDK_VENDOR_NL80211_STANDARD_N,	RDK_VENDOR_NL80211_CHAN_WIDTH_40,	1,	2,	300},
	{RDK_VENDOR_NL80211_STANDARD_N,	RDK_VENDOR_NL80211_CHAN_WIDTH_40,	1,	3,	450},
	{RDK_VENDOR_NL80211_STANDARD_N,	RDK_VENDOR_NL80211_CHAN_WIDTH_40,	1,	4,	600},

	{RDK_VENDOR_NL80211_STANDARD_N,	RDK_VENDOR_NL80211_CHAN_WIDTH_40,	-1,	1,	135},
	{RDK_VENDOR_NL80211_STANDARD_N,	RDK_VENDOR_NL80211_CHAN_WIDTH_40,	-1,	2,	270},
	{RDK_VENDOR_NL80211_STANDARD_N,	RDK_VENDOR_NL80211_CHAN_WIDTH_40,	-1,	3,	405},
	{RDK_VENDOR_NL80211_STANDARD_N,	RDK_VENDOR_NL80211_CHAN_WIDTH_40,	-1,	4,	540},

	{RDK_VENDOR_NL80211_STANDARD_AC,	RDK_VENDOR_NL80211_CHAN_WIDTH_20,	1,	1,	86},
	{RDK_VENDOR_NL80211_STANDARD_AC,	RDK_VENDOR_NL80211_CHAN_WIDTH_20,	1,	2,	173},
	{RDK_VENDOR_NL80211_STANDARD_AC,	RDK_VENDOR_NL80211_CHAN_WIDTH_20,	1,	3,	260},
	{RDK_VENDOR_NL80211_STANDARD_AC,	RDK_VENDOR_NL80211_CHAN_WIDTH_20,	1,	4,	347},
	{RDK_VENDOR_NL80211_STANDARD_AC,	RDK_VENDOR_NL80211_CHAN_WIDTH_20,	1,	5,	434},
	{RDK_VENDOR_NL80211_STANDARD_AC,	RDK_VENDOR_NL80211_CHAN_WIDTH_20,	1,	6,	521},
	{RDK_VENDOR_NL80211_STANDARD_AC,	RDK_VENDOR_NL80211_CHAN_WIDTH_20,	1,	7,	608},
	{RDK_VENDOR_NL80211_STANDARD_AC,	RDK_VENDOR_NL80211_CHAN_WIDTH_20,	1,	8,	695},

	{RDK_VENDOR_NL80211_STANDARD_AC,	RDK_VENDOR_NL80211_CHAN_WIDTH_20,	-1,	1,	78},
	{RDK_VENDOR_NL80211_STANDARD_AC,	RDK_VENDOR_NL80211_CHAN_WIDTH_20,	-1,	2,	156},
	{RDK_VENDOR_NL80211_STANDARD_AC,	RDK_VENDOR_NL80211_CHAN_WIDTH_20,	-1,	3,	234},
	{RDK_VENDOR_NL80211_STANDARD_AC,	RDK_VENDOR_NL80211_CHAN_WIDTH_20,	-1,	4,	312},
	{RDK_VENDOR_NL80211_STANDARD_AC,	RDK_VENDOR_NL80211_CHAN_WIDTH_20,	-1,	5,	390},
	{RDK_VENDOR_NL80211_STANDARD_AC,	RDK_VENDOR_NL80211_CHAN_WIDTH_20,	-1,	6,	468},
	{RDK_VENDOR_NL80211_STANDARD_AC,	RDK_VENDOR_NL80211_CHAN_WIDTH_20,	-1,	7,	546},
	{RDK_VENDOR_NL80211_STANDARD_AC,	RDK_VENDOR_NL80211_CHAN_WIDTH_20,	-1,	8,	624},

	{RDK_VENDOR_NL80211_STANDARD_AC,	RDK_VENDOR_NL80211_CHAN_WIDTH_40,	1,	1,	200},
	{RDK_VENDOR_NL80211_STANDARD_AC,	RDK_VENDOR_NL80211_CHAN_WIDTH_40,	1,	2,	400},
	{RDK_VENDOR_NL80211_STANDARD_AC,	RDK_VENDOR_NL80211_CHAN_WIDTH_40,	1,	3,	600},
	{RDK_VENDOR_NL80211_STANDARD_AC,	RDK_VENDOR_NL80211_CHAN_WIDTH_40,	1,	4,	800},
	{RDK_VENDOR_NL80211_STANDARD_AC,	RDK_VENDOR_NL80211_CHAN_WIDTH_40,	1,	5,	1000},
	{RDK_VENDOR_NL80211_STANDARD_AC,	RDK_VENDOR_NL80211_CHAN_WIDTH_40,	1,	6,	1200},
	{RDK_VENDOR_NL80211_STANDARD_AC,	RDK_VENDOR_NL80211_CHAN_WIDTH_40,	1,	7,	1400},
	{RDK_VENDOR_NL80211_STANDARD_AC,	RDK_VENDOR_NL80211_CHAN_WIDTH_40,	1,	8,	1600},

	{RDK_VENDOR_NL80211_STANDARD_AC,	RDK_VENDOR_NL80211_CHAN_WIDTH_40,	-1,	1,	180},
	{RDK_VENDOR_NL80211_STANDARD_AC,	RDK_VENDOR_NL80211_CHAN_WIDTH_40,	-1,	2,	360},
	{RDK_VENDOR_NL80211_STANDARD_AC,	RDK_VENDOR_NL80211_CHAN_WIDTH_40,	-1,	3,	540},
	{RDK_VENDOR_NL80211_STANDARD_AC,	RDK_VENDOR_NL80211_CHAN_WIDTH_40,	-1,	4,	720},
	{RDK_VENDOR_NL80211_STANDARD_AC,	RDK_VENDOR_NL80211_CHAN_WIDTH_40,	-1,	5,	900},
	{RDK_VENDOR_NL80211_STANDARD_AC,	RDK_VENDOR_NL80211_CHAN_WIDTH_40,	-1,	6,	1080},
	{RDK_VENDOR_NL80211_STANDARD_AC,	RDK_VENDOR_NL80211_CHAN_WIDTH_40,	-1,	7,	1260},
	{RDK_VENDOR_NL80211_STANDARD_AC,	RDK_VENDOR_NL80211_CHAN_WIDTH_40,	-1,	8,	1440},

	{RDK_VENDOR_NL80211_STANDARD_AC,	RDK_VENDOR_NL80211_CHAN_WIDTH_80,	1,	1,	433},
	{RDK_VENDOR_NL80211_STANDARD_AC,	RDK_VENDOR_NL80211_CHAN_WIDTH_80,	1,	2,	866},
	{RDK_VENDOR_NL80211_STANDARD_AC,	RDK_VENDOR_NL80211_CHAN_WIDTH_80,	1,	3,	1300},
	{RDK_VENDOR_NL80211_STANDARD_AC,	RDK_VENDOR_NL80211_CHAN_WIDTH_80,	1,	4,	1733},
	{RDK_VENDOR_NL80211_STANDARD_AC,	RDK_VENDOR_NL80211_CHAN_WIDTH_80,	1,	5,	2166},
	{RDK_VENDOR_NL80211_STANDARD_AC,	RDK_VENDOR_NL80211_CHAN_WIDTH_80,	1,	6,	2600},
	{RDK_VENDOR_NL80211_STANDARD_AC,	RDK_VENDOR_NL80211_CHAN_WIDTH_80,	1,	7,	3033},
	{RDK_VENDOR_NL80211_STANDARD_AC,	RDK_VENDOR_NL80211_CHAN_WIDTH_80,	1,	8,	3466},

	{RDK_VENDOR_NL80211_STANDARD_AC,	RDK_VENDOR_NL80211_CHAN_WIDTH_80,	-1,	1,	390},
	{RDK_VENDOR_NL80211_STANDARD_AC,	RDK_VENDOR_NL80211_CHAN_WIDTH_80,	-1,	2,	780},
	{RDK_VENDOR_NL80211_STANDARD_AC,	RDK_VENDOR_NL80211_CHAN_WIDTH_80,	-1,	3,	1170},
	{RDK_VENDOR_NL80211_STANDARD_AC,	RDK_VENDOR_NL80211_CHAN_WIDTH_80,	-1,	4,	1560},
	{RDK_VENDOR_NL80211_STANDARD_AC,	RDK_VENDOR_NL80211_CHAN_WIDTH_80,	-1,	5,	1950},
	{RDK_VENDOR_NL80211_STANDARD_AC,	RDK_VENDOR_NL80211_CHAN_WIDTH_80,	-1,	6,	2340},
	{RDK_VENDOR_NL80211_STANDARD_AC,	RDK_VENDOR_NL80211_CHAN_WIDTH_80,	-1,	7,	2730},
	{RDK_VENDOR_NL80211_STANDARD_AC,	RDK_VENDOR_NL80211_CHAN_WIDTH_80,	-1,	8,	3120},

	{RDK_VENDOR_NL80211_STANDARD_AC,	RDK_VENDOR_NL80211_CHAN_WIDTH_160,	1,	1,	867},
	{RDK_VENDOR_NL80211_STANDARD_AC,	RDK_VENDOR_NL80211_CHAN_WIDTH_160,	1,	2,	1733},
	{RDK_VENDOR_NL80211_STANDARD_AC,	RDK_VENDOR_NL80211_CHAN_WIDTH_160,	1,	3,	2340},
	{RDK_VENDOR_NL80211_STANDARD_AC,	RDK_VENDOR_NL80211_CHAN_WIDTH_160,	1,	4,	3467},
	{RDK_VENDOR_NL80211_STANDARD_AC,	RDK_VENDOR_NL80211_CHAN_WIDTH_160,	1,	5,	4333},
	{RDK_VENDOR_NL80211_STANDARD_AC,	RDK_VENDOR_NL80211_CHAN_WIDTH_160,	1,	6,	5200},
	{RDK_VENDOR_NL80211_STANDARD_AC,	RDK_VENDOR_NL80211_CHAN_WIDTH_160,	1,	7,	6067},
	{RDK_VENDOR_NL80211_STANDARD_AC,	RDK_VENDOR_NL80211_CHAN_WIDTH_160,	1,	8,	6933},

	{RDK_VENDOR_NL80211_STANDARD_AC,	RDK_VENDOR_NL80211_CHAN_WIDTH_160,	-1,	1,	780},
	{RDK_VENDOR_NL80211_STANDARD_AC,	RDK_VENDOR_NL80211_CHAN_WIDTH_160,	-1,	2,	1560},
	{RDK_VENDOR_NL80211_STANDARD_AC,	RDK_VENDOR_NL80211_CHAN_WIDTH_160,	-1,	3,	2106},
	{RDK_VENDOR_NL80211_STANDARD_AC,	RDK_VENDOR_NL80211_CHAN_WIDTH_160,	-1,	4,	3120},
	{RDK_VENDOR_NL80211_STANDARD_AC,	RDK_VENDOR_NL80211_CHAN_WIDTH_160,	-1,	5,	3900},
	{RDK_VENDOR_NL80211_STANDARD_AC,	RDK_VENDOR_NL80211_CHAN_WIDTH_160,	-1,	6,	4680},
	{RDK_VENDOR_NL80211_STANDARD_AC,	RDK_VENDOR_NL80211_CHAN_WIDTH_160,	-1,	7,	5460},
	{RDK_VENDOR_NL80211_STANDARD_AC,	RDK_VENDOR_NL80211_CHAN_WIDTH_160,	-1,	8,	6240},

	{RDK_VENDOR_NL80211_STANDARD_AX,	RDK_VENDOR_NL80211_CHAN_WIDTH_20,	1,	1,	143},
	{RDK_VENDOR_NL80211_STANDARD_AX,	RDK_VENDOR_NL80211_CHAN_WIDTH_20,	1,	2,	287},
	{RDK_VENDOR_NL80211_STANDARD_AX,	RDK_VENDOR_NL80211_CHAN_WIDTH_20,	1,	3,	430},
	{RDK_VENDOR_NL80211_STANDARD_AX,	RDK_VENDOR_NL80211_CHAN_WIDTH_20,	1,	4,	574},
	{RDK_VENDOR_NL80211_STANDARD_AX,	RDK_VENDOR_NL80211_CHAN_WIDTH_20,	1,	5,	717},
	{RDK_VENDOR_NL80211_STANDARD_AX,	RDK_VENDOR_NL80211_CHAN_WIDTH_20,	1,	6,	860},
	{RDK_VENDOR_NL80211_STANDARD_AX,	RDK_VENDOR_NL80211_CHAN_WIDTH_20,	1,	7,	1004},
	{RDK_VENDOR_NL80211_STANDARD_AX,	RDK_VENDOR_NL80211_CHAN_WIDTH_20,	1,	8,	1147},

	{RDK_VENDOR_NL80211_STANDARD_AX,	RDK_VENDOR_NL80211_CHAN_WIDTH_40,	1,	1,	287},
	{RDK_VENDOR_NL80211_STANDARD_AX,	RDK_VENDOR_NL80211_CHAN_WIDTH_40,	1,	2,	574},
	{RDK_VENDOR_NL80211_STANDARD_AX,	RDK_VENDOR_NL80211_CHAN_WIDTH_40,	1,	3,	860},
	{RDK_VENDOR_NL80211_STANDARD_AX,	RDK_VENDOR_NL80211_CHAN_WIDTH_40,	1,	4,	1147},
	{RDK_VENDOR_NL80211_STANDARD_AX,	RDK_VENDOR_NL80211_CHAN_WIDTH_40,	1,	5,	1434},
	{RDK_VENDOR_NL80211_STANDARD_AX,	RDK_VENDOR_NL80211_CHAN_WIDTH_40,	1,	6,	1721},
	{RDK_VENDOR_NL80211_STANDARD_AX,	RDK_VENDOR_NL80211_CHAN_WIDTH_40,	1,	7,	2007},
	{RDK_VENDOR_NL80211_STANDARD_AX,	RDK_VENDOR_NL80211_CHAN_WIDTH_40,	1,	8,	2294},

	{RDK_VENDOR_NL80211_STANDARD_AX,	RDK_VENDOR_NL80211_CHAN_WIDTH_80,	1,	1,	600},
	{RDK_VENDOR_NL80211_STANDARD_AX,	RDK_VENDOR_NL80211_CHAN_WIDTH_80,	1,	2,	1201},
	{RDK_VENDOR_NL80211_STANDARD_AX,	RDK_VENDOR_NL80211_CHAN_WIDTH_80,	1,	3,	1802},
	{RDK_VENDOR_NL80211_STANDARD_AX,	RDK_VENDOR_NL80211_CHAN_WIDTH_80,	1,	4,	2402},
	{RDK_VENDOR_NL80211_STANDARD_AX,	RDK_VENDOR_NL80211_CHAN_WIDTH_80,	1,	5,	3002},
	{RDK_VENDOR_NL80211_STANDARD_AX,	RDK_VENDOR_NL80211_CHAN_WIDTH_80,	1,	6,	3603},
	{RDK_VENDOR_NL80211_STANDARD_AX,	RDK_VENDOR_NL80211_CHAN_WIDTH_80,	1,	7,	4203},
	{RDK_VENDOR_NL80211_STANDARD_AX,	RDK_VENDOR_NL80211_CHAN_WIDTH_80,	1,	8,	4804},

	{RDK_VENDOR_NL80211_STANDARD_AX,	RDK_VENDOR_NL80211_CHAN_WIDTH_160,	1,	1,	1201},
	{RDK_VENDOR_NL80211_STANDARD_AX,	RDK_VENDOR_NL80211_CHAN_WIDTH_160,	1,	2,	2402},
	{RDK_VENDOR_NL80211_STANDARD_AX,	RDK_VENDOR_NL80211_CHAN_WIDTH_160,	1,	3,	3603},
	{RDK_VENDOR_NL80211_STANDARD_AX,	RDK_VENDOR_NL80211_CHAN_WIDTH_160,	1,	4,	4804},
	{RDK_VENDOR_NL80211_STANDARD_AX,	RDK_VENDOR_NL80211_CHAN_WIDTH_160,	1,	5,	6005},
	{RDK_VENDOR_NL80211_STANDARD_AX,	RDK_VENDOR_NL80211_CHAN_WIDTH_160,	1,	6,	7206},
	{RDK_VENDOR_NL80211_STANDARD_AX,	RDK_VENDOR_NL80211_CHAN_WIDTH_160,	1,	7,	8407},
	{RDK_VENDOR_NL80211_STANDARD_AX,	RDK_VENDOR_NL80211_CHAN_WIDTH_160,	1,	8,	9608},

	{RDK_VENDOR_NL80211_STANDARD_BE,	RDK_VENDOR_NL80211_CHAN_WIDTH_20,	1,	1,	172},
	{RDK_VENDOR_NL80211_STANDARD_BE,	RDK_VENDOR_NL80211_CHAN_WIDTH_20,	1,	2,	344},
	{RDK_VENDOR_NL80211_STANDARD_BE,	RDK_VENDOR_NL80211_CHAN_WIDTH_20,	1,	3,	516},
	{RDK_VENDOR_NL80211_STANDARD_BE,	RDK_VENDOR_NL80211_CHAN_WIDTH_20,	1,	4,	688},
	{RDK_VENDOR_NL80211_STANDARD_BE,	RDK_VENDOR_NL80211_CHAN_WIDTH_20,	1,	5,	860},
	{RDK_VENDOR_NL80211_STANDARD_BE,	RDK_VENDOR_NL80211_CHAN_WIDTH_20,	1,	6,	1032},
	{RDK_VENDOR_NL80211_STANDARD_BE,	RDK_VENDOR_NL80211_CHAN_WIDTH_20,	1,	7,	1204},
	{RDK_VENDOR_NL80211_STANDARD_BE,	RDK_VENDOR_NL80211_CHAN_WIDTH_20,	1,	8,	1377},

	{RDK_VENDOR_NL80211_STANDARD_BE,	RDK_VENDOR_NL80211_CHAN_WIDTH_40,	1,	1,	344},
	{RDK_VENDOR_NL80211_STANDARD_BE,	RDK_VENDOR_NL80211_CHAN_WIDTH_40,	1,	2,	688},
	{RDK_VENDOR_NL80211_STANDARD_BE,	RDK_VENDOR_NL80211_CHAN_WIDTH_40,	1,	3,	1032},
	{RDK_VENDOR_NL80211_STANDARD_BE,	RDK_VENDOR_NL80211_CHAN_WIDTH_40,	1,	4,	1377},
	{RDK_VENDOR_NL80211_STANDARD_BE,	RDK_VENDOR_NL80211_CHAN_WIDTH_40,	1,	5,	1721},
	{RDK_VENDOR_NL80211_STANDARD_BE,	RDK_VENDOR_NL80211_CHAN_WIDTH_40,	1,	6,	2065},
	{RDK_VENDOR_NL80211_STANDARD_BE,	RDK_VENDOR_NL80211_CHAN_WIDTH_40,	1,	7,	2409},
	{RDK_VENDOR_NL80211_STANDARD_BE,	RDK_VENDOR_NL80211_CHAN_WIDTH_40,	1,	8,	2753},

	{RDK_VENDOR_NL80211_STANDARD_BE,	RDK_VENDOR_NL80211_CHAN_WIDTH_80,	1,	1,	721},
	{RDK_VENDOR_NL80211_STANDARD_BE,	RDK_VENDOR_NL80211_CHAN_WIDTH_80,	1,	2,	1441},
	{RDK_VENDOR_NL80211_STANDARD_BE,	RDK_VENDOR_NL80211_CHAN_WIDTH_80,	1,	3,	2162},
	{RDK_VENDOR_NL80211_STANDARD_BE,	RDK_VENDOR_NL80211_CHAN_WIDTH_80,	1,	4,	2882},
	{RDK_VENDOR_NL80211_STANDARD_BE,	RDK_VENDOR_NL80211_CHAN_WIDTH_80,	1,	5,	3603},
	{RDK_VENDOR_NL80211_STANDARD_BE,	RDK_VENDOR_NL80211_CHAN_WIDTH_80,	1,	6,	4324},
	{RDK_VENDOR_NL80211_STANDARD_BE,	RDK_VENDOR_NL80211_CHAN_WIDTH_80,	1,	7,	5044},
	{RDK_VENDOR_NL80211_STANDARD_BE,	RDK_VENDOR_NL80211_CHAN_WIDTH_80,	1,	8,	5765},

	{RDK_VENDOR_NL80211_STANDARD_BE,	RDK_VENDOR_NL80211_CHAN_WIDTH_160,	1,	1,	1441},
	{RDK_VENDOR_NL80211_STANDARD_BE,	RDK_VENDOR_NL80211_CHAN_WIDTH_160,	1,	2,	2882},
	{RDK_VENDOR_NL80211_STANDARD_BE,	RDK_VENDOR_NL80211_CHAN_WIDTH_160,	1,	3,	4324},
	{RDK_VENDOR_NL80211_STANDARD_BE,	RDK_VENDOR_NL80211_CHAN_WIDTH_160,	1,	4,	5765},
	{RDK_VENDOR_NL80211_STANDARD_BE,	RDK_VENDOR_NL80211_CHAN_WIDTH_160,	1,	5,	7206},
	{RDK_VENDOR_NL80211_STANDARD_BE,	RDK_VENDOR_NL80211_CHAN_WIDTH_160,	1,	6,	8547},
	{RDK_VENDOR_NL80211_STANDARD_BE,	RDK_VENDOR_NL80211_CHAN_WIDTH_160,	1,	7,	10088},
	{RDK_VENDOR_NL80211_STANDARD_BE,	RDK_VENDOR_NL80211_CHAN_WIDTH_160,	1,	8,	11529},

	{RDK_VENDOR_NL80211_STANDARD_BE,	RDK_VENDOR_NL80211_CHAN_WIDTH_320,	1,	1,	2882},
	{RDK_VENDOR_NL80211_STANDARD_BE,	RDK_VENDOR_NL80211_CHAN_WIDTH_320,	1,	2,	5765},
	{RDK_VENDOR_NL80211_STANDARD_BE,	RDK_VENDOR_NL80211_CHAN_WIDTH_320,	1,	3,	8647},
	{RDK_VENDOR_NL80211_STANDARD_BE,	RDK_VENDOR_NL80211_CHAN_WIDTH_320,	1,	4,	11529},
	{RDK_VENDOR_NL80211_STANDARD_BE,	RDK_VENDOR_NL80211_CHAN_WIDTH_320,	1,	5,	14412},
	{RDK_VENDOR_NL80211_STANDARD_BE,	RDK_VENDOR_NL80211_CHAN_WIDTH_320,	1,	6,	17294},
	{RDK_VENDOR_NL80211_STANDARD_BE,	RDK_VENDOR_NL80211_CHAN_WIDTH_320,	1,	7,	20177},
	{RDK_VENDOR_NL80211_STANDARD_BE,	RDK_VENDOR_NL80211_CHAN_WIDTH_320,	1,	8,	23059},

};
static int
wl_cfgvendor_get_max_tx_rx_rate(struct wiphy *wiphy, struct wireless_dev *wdev, uint32 standard,
	uint8 bw, uint8 nss, u32 *max_tx_rate, u32 *max_rx_rate, bool is_brcm_sta)
{
	int i, ret, mcs_gi;
	unsigned int gi;

	*max_tx_rate = 0;
	*max_rx_rate = 0;

	if (standard == RDK_VENDOR_NL80211_STANDARD_A ||
		standard == RDK_VENDOR_NL80211_STANDARD_G) {
		*max_tx_rate = 54;
		*max_rx_rate = 54;
		return 0;
	}

	if (standard == RDK_VENDOR_NL80211_STANDARD_B) {
		*max_tx_rate = 11;
		*max_rx_rate = 11;
		return 0;
	}

	ret = wl_cfgvendor_get_guard_interval(wiphy, wdev, &gi);
	if (ret < 0) {
		WL_ERR(("Failed to get guard interval, error: %d\n", ret));
		return ret;
	}

	mcs_gi = (gi == guard_interval_800) ? -1 : 1;
	if (is_brcm_sta) {
		for (i = 0; i < ARRAY_SIZE(mcs_rate_tbl); i++) {
			if (mcs_rate_tbl[i].oper_standard == standard &&
				mcs_rate_tbl[i].bw == bw &&
				mcs_rate_tbl[i].gi == mcs_gi &&
				mcs_rate_tbl[i].nss == nss) {
				*max_tx_rate = mcs_rate_tbl[i].rate;
				*max_rx_rate = mcs_rate_tbl[i].rate;
				return 0;
			}
		}
        }
	for (i = 0; i < ARRAY_SIZE(non_brcm_mcs_rate_tbl); i++) {
		if (non_brcm_mcs_rate_tbl[i].oper_standard == standard &&
			non_brcm_mcs_rate_tbl[i].bw == bw &&
			non_brcm_mcs_rate_tbl[i].gi == mcs_gi &&
			non_brcm_mcs_rate_tbl[i].nss == nss) {
			*max_tx_rate = non_brcm_mcs_rate_tbl[i].rate;
			*max_rx_rate = non_brcm_mcs_rate_tbl[i].rate;
			break;
		}
	}

	if (i == ARRAY_SIZE(non_brcm_mcs_rate_tbl)) {
		WL_ERR(("Failed to find max rate, standard: %d bw: %d nss: %d gi: %d\n", standard,
			bw, nss, mcs_gi));
		return -1;
	}

	return 0;
}

static int
wl_cfgvendor_get_station_params(const void *data, int len, u8 *mac)
{
	int ret;
	struct nlattr *tb[RDK_VENDOR_ATTR_MAX + 1];

	ret = nla_parse(tb, RDK_VENDOR_ATTR_MAX, data, len, wl_cfgvendor_get_station_policy,
		NULL);
	if (ret < 0) {
		WL_ERR(("Failed to parse get station attributes, error: %d\n", ret));
		return ret;
	}

	if (!tb[RDK_VENDOR_ATTR_MAC]) {
		WL_ERR(("station MAC address attribute is missing\n"));
		return -EINVAL;
	}

	memcpy(mac, nla_data(tb[RDK_VENDOR_ATTR_MAC]), ETH_ALEN);

	return ret;
}

static int
wl_cfgvendor_get_chanim_stats_ioctl(struct wiphy *wiphy, struct wireless_dev *wdev, int count,
	char *buf, int buf_size)
{
	int ret;
	u32 buflen;
	struct net_device *ndev;
	wl_chanim_stats_t chanim_stats;
	struct bcm_cfg80211 *cfg = wiphy_priv(wiphy);

	ndev = wdev_to_wlc_ndev(wdev, cfg);
	if (!ndev) {
		WL_ERR(("Failed to get ndev\n"));
		return BCME_ERROR;
	}
	buflen = snprintf(buf, buf_size, "chanim_stats") + 1;

	if (buf_size < sizeof(wl_chanim_stats_t) + buflen) {
		WL_ERR(("Incorrect buf_size:%d\n", buf_size));
		return BCME_ERROR;
	}
	memset(&chanim_stats, 0, sizeof(wl_chanim_stats_t));
	chanim_stats.buflen = htod32(buf_size);
	chanim_stats.count = count;
	chanim_stats.version = WL_CHANIM_STATS_VERSION;
	memcpy(buf + buflen, &chanim_stats, sizeof(wl_chanim_stats_t));

	ret = wldev_ioctl_get(ndev, WLC_GET_VAR, buf, buf_size);
	if (ret < 0) {
		WL_ERR(("chanim stats ioctl failed, error: %d\n", ret));
	}
	return ret;
}

static int
wl_cfgvendor_get_station_reply(struct wiphy *wiphy, struct wireless_dev *wdev,
	sta_info_t *sta_info)
{
	int ret = 0;
	int32 rssi;
	uint8 bw, nss;
	struct sk_buff *skb;
	struct nlattr *nlattr;
	char *buf = NULL;
	wl_chanim_stats_t *chanim_stats;
	uint32 standard, max_tx_rate, max_rx_rate;
	struct nl80211_sta_flag_update sta_flags = {};
#ifdef WL_STA_MLO_CAP
	char mld_mac[ETH_ALEN] = {};
#endif /* WL_STA_MLO_CAP */
	uint64 assoc_num, tx_bytes = 0, rx_bytes = 0, tx_packets = 0,
		tx_packets_ack = 0, tx_packets_no_ack = 0, rx_packets = 0,
		tx_errors = 0, rx_errors = 0, tx_retransmits = 0, tx_failed_retries = 0,
		tx_retries = 0, rx_retries = 0, tx_frames = 0;
	bool is_brcm_sta = false;

	if (sta_info->flags & WL_STA_SCBSTATS) {
		tx_packets_ack = dtoh32(sta_info->tx_pkts_total);
		tx_packets_no_ack = dtoh32(sta_info->tx_pkts_retry_exhausted);
		tx_bytes = dtoh64(sta_info->tx_tot_bytes);
		rx_bytes = dtoh64(sta_info->rx_tot_bytes);
		tx_packets = tx_packets_ack + tx_packets_no_ack;
		rx_packets = dtoh32(sta_info->rx_tot_pkts);
		tx_errors = dtoh32(sta_info->tx_failures);
		rx_errors = dtoh32(sta_info->rx_decrypt_failures);
		tx_retransmits = dtoh32(sta_info->tx_pkts_retries);
		tx_failed_retries = dtoh32(sta_info->tx_pkts_retry_exhausted);
		tx_retries = dtoh32(sta_info->tx_pkts_retried);
		rx_retries = dtoh32(sta_info->rx_pkts_retried);
		tx_frames = dtoh32(sta_info->tx_tot_pkts);
	}

	if (sta_info->flags & WL_STA_AUTHE) {
		sta_flags.set |= BIT(NL80211_STA_FLAG_AUTHORIZED);
		sta_flags.mask |= BIT(NL80211_STA_FLAG_AUTHORIZED);
	}

	skb = cfg80211_vendor_cmd_alloc_reply_skb(wiphy, NLMSG_DEFAULT_SIZE);
	if (!skb) {
		WL_ERR(("Failed to allocate skb\n"));
		return -ENOMEM;
	}

	ret = nla_put(skb, RDK_VENDOR_ATTR_MAC, ETH_ALEN, &sta_info->ea);
	if (ret < 0) {
		WL_ERR(("Failed to add MAC address attribute, error: %d\n", ret));
		goto error;
	}

	nlattr = nla_nest_start(skb, RDK_VENDOR_ATTR_STA_INFO);
	if (!nlattr) {
		WL_ERR(("Failed to add STA info attribute\n"));
		ret = -EMSGSIZE;
		goto error;
	}

	ret = nla_put(skb, RDK_VENDOR_ATTR_STA_INFO_STA_FLAGS,
		sizeof(struct nl80211_sta_flag_update), &sta_flags);
	if (ret < 0) {
		WL_ERR(("Failed to add STA flags attribute, error: %d\n", ret));
		goto error;
	}

	ret = nla_put_u32(skb, RDK_VENDOR_ATTR_STA_INFO_RX_BITRATE_LAST,
		sta_info->rx_rate / 1000);
	if (ret < 0) {
		WL_ERR(("Failed to add rx bitrate attribute, error: %d\n", ret));
		goto error;
	}

	ret = nla_put_u32(skb, RDK_VENDOR_ATTR_STA_INFO_TX_BITRATE_LAST,
		sta_info->tx_rate / 1000);
	if (ret < 0) {
		WL_ERR(("Failed to add tx bitrate attribute, error: %d\n", ret));
		goto error;
	}

	wl_cfgvendor_get_station_rssi(sta_info, &rssi);
	ret = nla_put_s32(skb, RDK_VENDOR_ATTR_STA_INFO_SIGNAL_AVG, rssi);
	if (ret < 0) {
		WL_ERR(("Failed to add signal attribute, error: %d\n", ret));
		goto error;
	}

	/* not supported */
	ret = nla_put_s32(skb, RDK_VENDOR_ATTR_STA_INFO_SIGNAL_MIN, 0);
	if (ret < 0) {
		WL_ERR(("Failed to add min signal attribute, error: %d\n", ret));
		goto error;
	}

	/* not supported */
	ret = nla_put_s32(skb, RDK_VENDOR_ATTR_STA_INFO_SIGNAL_MAX, 0);
	if (ret < 0) {
		WL_ERR(("Failed to add max signal attribute, error: %d\n", ret));
		goto error;
	}

	/* total retransmissions, percentage is not supported */
	ret = nla_put_u32(skb, RDK_VENDOR_ATTR_STA_INFO_TX_RETRIES_PERCENT, tx_retransmits);
	if (ret < 0) {
		WL_ERR(("Failed to add tx retries attribute, error: %d\n", ret));
		goto error;
	}

	/* always active */
	ret = nla_put_u8(skb, RDK_VENDOR_ATTR_STA_INFO_ACTIVE, 1);
	if (ret < 0) {
		WL_ERR(("Failed to add STA active attribute, error: %d\n", ret));
		goto error;
	}

	ret = wl_cfgvendor_get_80211_standard(wiphy, wdev, sta_info, &standard);
	if (ret < 0) {
		WL_ERR(("Failed to get standard, error: %d\n", ret));
		goto error;
	}

	ret = nla_put_u32(skb, RDK_VENDOR_ATTR_STA_INFO_OPER_STANDARD, standard);
	if (ret < 0) {
		WL_ERR(("Failed to add standard attribute, error: %d\n", ret));
		goto error;
	}

	wl_cfgvendor_get_station_bw(sta_info, &bw);
	ret = nla_put_u8(skb, RDK_VENDOR_ATTR_STA_INFO_OPER_CHANNEL_BW, bw);
	if (ret < 0) {
		WL_ERR(("Failed to add channel BW attribute, error: %d\n", ret));
		goto error;
	}
	buf = kmalloc(WLC_IOCTL_MAXLEN, GFP_KERNEL);
	if (!buf) {
		WL_ERR(("Failed to alloc chanim buffer\n"));
		goto error;
	}
	ret = wl_cfgvendor_get_chanim_stats_ioctl(wiphy, wdev, 1, buf, WLC_IOCTL_MAXLEN);
	if (ret < 0) {
		WL_ERR(("Failed to get noise, error: %d\n", ret));
		goto error;
	}
	chanim_stats = (wl_chanim_stats_t *)buf;

	ret = nla_put_s32(skb, RDK_VENDOR_ATTR_STA_INFO_SNR,
		rssi - chanim_stats->stats[0].bgnoise);
	if (ret < 0) {
		WL_ERR(("Failed to add SNR attribute, error: %d\n", ret));
		goto error;
	}

	ret = nla_put_u64_64bit(skb, RDK_VENDOR_ATTR_STA_INFO_TX_PACKETS_ACK, tx_packets_ack,
		RDK_VENDOR_ATTR_PAD);
	if (ret < 0) {
		WL_ERR(("Failed to add tx packets ack attribute, error: %d\n", ret));
		goto error;
	}

	ret = nla_put_u64_64bit(skb, RDK_VENDOR_ATTR_STA_INFO_TX_PACKETS_NO_ACK,
		tx_packets_no_ack, RDK_VENDOR_ATTR_PAD);
	if (ret < 0) {
		WL_ERR(("Failed to add tx packets no ack attribute, error: %d\n", ret));
		goto error;
	}

	ret = nla_put_u64_64bit(skb, RDK_VENDOR_ATTR_STA_INFO_TX_BYTES64, tx_bytes,
		RDK_VENDOR_ATTR_PAD);
	if (ret < 0) {
		WL_ERR(("Failed to add tx bytes attribute, error: %d\n", ret));
		goto error;
	}

	ret = nla_put_u64_64bit(skb, RDK_VENDOR_ATTR_STA_INFO_RX_BYTES64, rx_bytes,
		RDK_VENDOR_ATTR_PAD);
	if (ret < 0) {
		WL_ERR(("Failed to add rx bytes attribute, error: %d\n", ret));
		goto error;
	}

	/* not supported */
	ret = nla_put_u32(skb, RDK_VENDOR_ATTR_STA_INFO_DISASSOC_NUM, 0);
	if (ret < 0) {
		WL_ERR(("Failed to add disassoc num attribute, error: %d\n", ret));
		goto error;
	}

	/* not supported */
	ret = nla_put_u32(skb, RDK_VENDOR_ATTR_STA_INFO_AUTH_FAILS, 0);
	if (ret < 0) {
		WL_ERR(("Failed to add auth fails attribute, error: %d\n", ret));
		goto error;
	}

	ret = wl_cfgvendor_get_assoc_num(wdev, &sta_info->ea, &assoc_num);
	if (ret < 0) {
		WL_ERR(("Failed to get assoc num, error: %d\n", ret));
		goto error;
	}
	ret = nla_put_u64_64bit(skb, RDK_VENDOR_ATTR_STA_INFO_ASSOC_NUM, assoc_num,
		RDK_VENDOR_ATTR_PAD);
	if (ret < 0) {
		WL_ERR(("Failed to add assoc num attribute, error: %d\n", ret));
		goto error;
	}

	ret = nla_put_u64_64bit(skb, RDK_VENDOR_ATTR_STA_INFO_TX_PACKETS64, tx_packets,
		RDK_VENDOR_ATTR_PAD);
	if (ret < 0) {
		WL_ERR(("Failed to add tx packets attribute, error: %d\n", ret));
		goto error;
	}

	ret = nla_put_u64_64bit(skb, RDK_VENDOR_ATTR_STA_INFO_RX_PACKETS64, rx_packets,
		RDK_VENDOR_ATTR_PAD);
	if (ret < 0) {
		WL_ERR(("Failed to add rx packets attribute, error: %d\n", ret));
		goto error;
	}

	ret = nla_put_u64_64bit(skb, RDK_VENDOR_ATTR_STA_INFO_TX_ERRORS,
		tx_errors, RDK_VENDOR_ATTR_PAD);
	if (ret < 0) {
		WL_ERR(("Failed to add tx failed attribute, error: %d\n", ret));
		goto error;
	}

	ret = nla_put_u64_64bit(skb, RDK_VENDOR_ATTR_STA_INFO_TX_RETRANSMIT,
		tx_retransmits, RDK_VENDOR_ATTR_PAD);
	if (ret < 0) {
		WL_ERR(("Failed to add retransmits attribute, error: %d\n", ret));
		goto error;
	}

	ret = nla_put_u64_64bit(skb, RDK_VENDOR_ATTR_STA_INFO_TX_FAILED_RETRIES,
		tx_failed_retries, RDK_VENDOR_ATTR_PAD);
	if (ret < 0) {
		WL_ERR(("Failed to add tx failed retries attribute, error: %d\n", ret));
		goto error;
	}

	ret = nla_put_u64_64bit(skb, RDK_VENDOR_ATTR_STA_INFO_TX_RETRIES,
		tx_retries, RDK_VENDOR_ATTR_PAD);
	if (ret < 0) {
		WL_ERR(("Failed to add tx retries attribute, error: %d\n", ret));
		goto error;
	}

	/* not supported */
	ret = nla_put_u64_64bit(skb, RDK_VENDOR_ATTR_STA_INFO_TX_MULT_RETRIES, 0,
		RDK_VENDOR_ATTR_PAD);
	if (ret < 0) {
		WL_ERR(("Failed to add tx mult retries attribute, error: %d\n", ret));
		goto error;
	}

	wl_cfgvendor_get_spatial_stream_num(sta_info, &nss);
	ret = nla_put_u8(skb, RDK_VENDOR_ATTR_STA_INFO_SPATIAL_STREAM_NUM, nss);
	if (ret < 0) {
		WL_ERR(("Failed to add spatial streams attribute, error: %d\n", ret));
		goto error;
	}
	is_brcm_sta = (sta_info->flags & WL_BRCM_STA) ? true : false;
	ret = wl_cfgvendor_get_max_tx_rx_rate(wiphy, wdev, standard, bw, nss, &max_tx_rate,
		&max_rx_rate, is_brcm_sta);
	if (ret < 0) {
		WL_ERR(("Failed to get max tx/rx rate, error: %d\n", ret));
		goto error;
	}

	ret = nla_put_u32(skb, RDK_VENDOR_ATTR_STA_INFO_TX_RATE_MAX, max_tx_rate);
	if (ret < 0) {
		WL_ERR(("Failed to add tx rate max attribute, error: %d\n", ret));
		goto error;
	}

	ret = nla_put_u32(skb, RDK_VENDOR_ATTR_STA_INFO_RX_RATE_MAX, max_rx_rate);
	if (ret < 0) {
		WL_ERR(("Failed to add rx rate max attribute, error: %d\n", ret));
		goto error;
	}

	ret = nla_put_u64_64bit(skb, RDK_VENDOR_ATTR_STA_INFO_TX_FRAMES, tx_frames,
		RDK_VENDOR_ATTR_PAD);
	if (ret < 0) {
		WL_ERR(("Failed to add STA tx frames attribute, error: %d\n", ret));
		goto error;
	}

	ret = nla_put_u64_64bit(skb, RDK_VENDOR_ATTR_STA_INFO_RX_RETRIES, rx_retries,
		RDK_VENDOR_ATTR_PAD);
	if (ret < 0) {
		WL_ERR(("Failed to add STA rx retries attribute, error: %d\n", ret));
		goto error;
	}

	ret = nla_put_u64_64bit(skb, RDK_VENDOR_ATTR_STA_INFO_RX_ERRORS, rx_errors,
		RDK_VENDOR_ATTR_PAD);
	if (ret < 0) {
		WL_ERR(("Failed to add STA rx errors attribute, error: %d\n", ret));
		goto error;
	}

#ifdef WL_STA_MLO_CAP
	ret = nla_put_u8(skb, RDK_VENDOR_ATTR_STA_INFO_MLD_ENAB,
		(sta_info->flags & WL_STA_MLO_CAP) ? 1: 0);
	if (ret < 0) {
		WL_ERR(("Failed to add STA mld enab attribute, error: %d\n", ret));
		goto error;
	}
	memcpy(mld_mac, &sta_info->peer_mld, sizeof(mld_mac));
	ret = nla_put(skb, RDK_VENDOR_ATTR_STA_INFO_MLD_MAC, ETH_ALEN, mld_mac);
	if (ret < 0) {
		WL_ERR(("Failed to add STA mld mac attribute, error: %d\n", ret));
		goto error;
	}
#endif /* WL_STA_MLO_CAP */

	nla_nest_end(skb, nlattr);

	ret = cfg80211_vendor_cmd_reply(skb);
	if (ret < 0) {
		WL_ERR(("Failed to send reply, error: %d\n", ret));
	}
	if (buf)
		kfree(buf);
	return ret;
error:
	if (skb)
		kfree_skb(skb);
	if (buf)
		kfree(buf);
	return ret;
}

static int
wl_cfgvendor_get_station_ioctl(struct wiphy *wiphy, struct wireless_dev *wdev, u8 *mac,
	sta_info_t *sta_info)
{
	int ret;
	u32 buflen;
	struct net_device *ndev;
	char *buf = (char *)sta_info;
	struct bcm_cfg80211 *cfg = wiphy_priv(wiphy);

	ndev = wdev_to_wlc_ndev(wdev, cfg);
	if (!ndev) {
		WL_ERR(("Failed to get ndev\n"));
		return BCME_ERROR;
	}

	memset(sta_info, '\0', sizeof(sta_info_t));
	buflen = snprintf(buf, sizeof(sta_info_t), "sta_info") + 1;
	memcpy(buf + buflen, mac, ETH_ALEN);

	ret = wldev_ioctl_get(ndev, WLC_GET_VAR, buf, sizeof(sta_info_t));
	if (ret < 0) {
		WL_ERR(("ioctl failed, error: %d\n", ret));
	}
	return ret;
}

static int
wl_cfgvendor_get_station_handler(struct wiphy *wiphy,
	struct wireless_dev *wdev, const void *data, int len)
{
	int ret;
	u8 mac[ETH_ALEN];
	sta_info_t sta_info;

	ret = wl_cfgvendor_get_station_params(data, len, mac);
	if (ret < 0) {
		WL_ERR(("Failed to get params, error: %d\n", ret));
		return ret;
	}

	ret = wl_cfgvendor_get_station_ioctl(wiphy, wdev, mac, &sta_info);
	if (ret < 0) {
		WL_ERR(("Station ioctl failed, error: %d\n", ret));
		return ret;
	}

	ret = wl_cfgvendor_get_station_reply(wiphy, wdev, &sta_info);
	if (ret < 0) {
		WL_ERR(("Failed to send station info reply, error: %d\n", ret));
		return ret;
	}

	return 0;
}

static int
wl_cfgvendor_get_station_list_reply(struct wiphy *wiphy, struct wireless_dev *wdev,
	char *buf)
{
	int ret;
	struct sk_buff *skb;
	struct nlattr *nlattr;
	struct maclist *assoc_list;
	uint32 mem_needed, i, assoc_cnt;

	assoc_list = (struct maclist *)buf;
	assoc_cnt = dtoh32(assoc_list->count);

	mem_needed = VENDOR_REPLY_OVERHEAD + ATTRIBUTE_U32_LEN +
		assoc_cnt * (ETH_ALEN + NLA_HDRLEN);

	skb = cfg80211_vendor_cmd_alloc_reply_skb(wiphy, mem_needed);
	if (!skb) {
		WL_ERR(("Failed to allocate skb\n"));
		return -ENOMEM;
	}

	ret = nla_put_u32(skb, RDK_VENDOR_ATTR_STA_NUM, assoc_cnt);
	if (ret < 0) {
		WL_ERR(("Failed to add station num attribute, error: %d\n", ret));
		goto error;
	}

	nlattr = nla_nest_start(skb, RDK_VENDOR_ATTR_STA_LIST);
	if (!nlattr) {
		WL_ERR(("Failed to add station list attribute\n"));
		ret = -EMSGSIZE;
		goto error;
	}

	for (i = 0; i < assoc_cnt; i++) {
		ret = nla_put(skb, RDK_VENDOR_ATTR_MAC, ETH_ALEN, &assoc_list->ea[i]);
		if (ret < 0) {
			WL_ERR(("Failed to add MAC address attribute, error: %d\n", ret));
			goto error;
		}
	}

	nla_nest_end(skb, nlattr);

	ret = cfg80211_vendor_cmd_reply(skb);
	if (ret < 0) {
		WL_ERR(("Failed to send reply, error: %d\n", ret));
	}
	return ret;

error:
	kfree_skb(skb);
	return ret;
}

static int
wl_cfgvendor_get_station_list_ioctl(struct wiphy *wiphy, struct wireless_dev *wdev,
	char *buf, unsigned int len)
{
	int ret;
	struct net_device *ndev;
	struct bcm_cfg80211 *cfg = wiphy_priv(wiphy);

	ndev = wdev_to_wlc_ndev(wdev, cfg);
	if (!ndev) {
		WL_ERR(("Failed to get ndev\n"));
		return BCME_ERROR;
	}

	ret = wldev_ioctl_get(ndev, WLC_GET_ASSOCLIST, buf, len);
	if (ret < 0) {
		WL_ERR(("get assoc list ioctl failed, error: %d\n", ret));
		return ret;
	}

	return ret;
}

static int
wl_cfgvendor_get_station_list_handler(struct wiphy *wiphy,
	struct wireless_dev *wdev, const void *data, int len)
{
	int ret = 0;
	char *ioctl_buf;

	ioctl_buf = kzalloc(WLC_IOCTL_MAXLEN, GFP_KERNEL);
	if (!ioctl_buf) {
		WL_ERR(("Failed to alloc buffer\n"));
		return -ENOMEM;
	}

	ret = wl_cfgvendor_get_station_list_ioctl(wiphy, wdev, ioctl_buf, WLC_IOCTL_MAXLEN);
	if (ret < 0) {
		WL_ERR(("Failed to get station list, error: %d\n", ret));
		goto error;
	}

	ret = wl_cfgvendor_get_station_list_reply(wiphy, wdev, ioctl_buf);
	if (ret < 0) {
		WL_ERR(("Failed to send station list, error: %d\n", ret));
		goto error;
	}
error:
	kfree(ioctl_buf);
	return ret;
}

typedef struct wl_cfgvendor_channel_stats {
	int			ch_number;
	int			ch_noise;
	char			ch_radar_noise;
	int			ch_max_80211_rssi;
	int			ch_non_80211_noise;
	int			ch_utilization;
	unsigned long long	ch_utilization_total;
	unsigned long long	ch_utilization_busy;
	unsigned long long	ch_utilization_busy_tx;
	unsigned long long	ch_utilization_busy_rx;
	unsigned long long	ch_utilization_busy_self;
	unsigned long long	ch_utilization_busy_ext;
	chanspec_t		ch_spec;
} wl_cfgvendor_channel_stats_t;

static int
wl_cfgvendor_get_channel_stats(struct wiphy *wiphy, struct wireless_dev *wdev,
	wl_cfgvendor_channel_stats_t *stats, int stats_size, int *stats_num)
{
	char *buf, *curr_buf;
	int i, ret;
	uint currCh, usCh;
	wl_chanim_stats_t *chanim_ptr, *curr_chanim_ptr;
	wl_chanim_stats_us_v2_t *chanim_us_ptr, *currChstats_us_ptr;
	chanim_stats_us_v2_t currChstats_us_v2 = {0}, *chstats_us_v2ptr = NULL;

	buf = kmalloc(WLC_IOCTL_MAXLEN, GFP_KERNEL);
	if (!buf) {
		WL_ERR(("Failed to alloc chanim buffer\n"));
		return -ENOMEM;
	}
	curr_buf = kmalloc(WLC_IOCTL_MAXLEN, GFP_KERNEL);
	if (!curr_buf) {
		WL_ERR(("Failed to alloc chanim buffer for current chanspec\n"));
		kfree(buf);
		return -ENOMEM;
	}
	ret = wl_cfgvendor_get_chanim_stats_ioctl(wiphy, wdev, WL_CHANIM_COUNT_ALL, buf,
		WLC_IOCTL_MAXLEN);
	if (ret < 0) {
		WL_ERR(("Failed to chanim stats all, error: %d\n", ret));
		goto exit;
	}

	chanim_ptr = (wl_chanim_stats_t *)buf;
	chanim_ptr->version = dtoh32(chanim_ptr->version);
	chanim_ptr->count = dtoh32(chanim_ptr->count);
	if (chanim_ptr->version != WL_CHANIM_STATS_VERSION) {
		WL_ERR(("Wrong chanim_stats version: %d\n", chanim_ptr->version));
		ret = -1;
		goto exit;
	}

	ret = wl_cfgvendor_get_chanim_stats_ioctl(wiphy, wdev, WL_CHANIM_COUNT_ONE, curr_buf,
		WLC_IOCTL_MAXLEN);
	if (ret < 0) {
		WL_ERR(("Failed to chanim stats, error: %d\n", ret));
		goto exit;
	}
	curr_chanim_ptr = (wl_chanim_stats_t *)curr_buf;
	currCh = wf_chspec_ctlchan(dtoh16(curr_chanim_ptr->stats->chanspec));

	*stats_num = chanim_ptr->count;
	for (i = 0; i < chanim_ptr->count && i < stats_size; i++) {
		stats[i].ch_number = CHSPEC_CHANNEL(chanim_ptr->stats[i].chanspec);
		stats[i].ch_radar_noise = 0;
		stats[i].ch_non_80211_noise = 0;
		stats[i].ch_max_80211_rssi = 0;
		/* The stats value for home channel is not accurate in chanim_stats all. */
		if (stats[i].ch_number == currCh) {
			stats[i].ch_noise = curr_chanim_ptr->stats->bgnoise;
			stats[i].ch_utilization = curr_chanim_ptr->stats->channel_utilization;
		}
		else {
			stats[i].ch_noise = chanim_ptr->stats[i].bgnoise;
			stats[i].ch_utilization = chanim_ptr->stats[i].channel_utilization;
		}
		stats[i].ch_spec = chanim_ptr->stats[i].chanspec;
	}

	if (i == stats_size) {
		WL_ERR(("Channel buffer too short: %d\n", stats_size));
		ret = -1;
		goto exit;
	}

	ret = wl_cfgvendor_get_chanim_stats_ioctl(wiphy, wdev, WL_CHANIM_COUNT_US_ONE, buf,
		WLC_IOCTL_MAXLEN);
	if (ret < 0) {
		WL_ERR(("Failed to chanim us stats, error: %d\n", ret));
		goto exit;
	}
	currChstats_us_ptr = (wl_chanim_stats_us_v2_t *) buf;
	memcpy(&currChstats_us_v2, currChstats_us_ptr->stats_us_v2, sizeof(chanim_stats_us_v2_t));
	currCh = wf_chspec_ctlchan(dtoh16(currChstats_us_v2.chanspec));

	ret = wl_cfgvendor_get_chanim_stats_ioctl(wiphy, wdev, WL_CHANIM_COUNT_US_ALL, buf,
		WLC_IOCTL_MAXLEN);
	if (ret < 0) {
		WL_ERR(("Failed to chanim us stats, error: %d\n", ret));
		goto exit;
	}

	chanim_us_ptr = (wl_chanim_stats_us_v2_t *)buf;

	for (i = 0; i < chanim_us_ptr->count && i < *stats_num; i++) {
		if (CHSPEC_CHANNEL(chanim_us_ptr->stats_us_v2[i].chanspec) != stats[i].ch_number) {
			WL_ERR(("chanim channel mismatch\n"));
			ret = -1;
			goto exit;
		}
		usCh = wf_chspec_ctlchan(dtoh16(chanim_us_ptr->stats_us_v2[i].chanspec));
		chstats_us_v2ptr =
			(currCh == usCh) ? &currChstats_us_v2 : &(chanim_us_ptr->stats_us_v2[i]);
		stats[i].ch_utilization_total = dtoh64(chstats_us_v2ptr->total_tm);
		stats[i].ch_utilization_busy =
			stats[i].ch_utilization_total -
			dtoh64(chstats_us_v2ptr->ccastats_us[CCASTATS_TXOP]);
		stats[i].ch_utilization_busy_tx =
			dtoh64(chstats_us_v2ptr->ccastats_us[CCASTATS_TXDUR]);
		stats[i].ch_utilization_busy_rx =
			dtoh64(chstats_us_v2ptr->ccastats_us[CCASTATS_OBSS]) +
			dtoh64(chstats_us_v2ptr->ccastats_us[CCASTATS_INBSS]) +
			dtoh64(chstats_us_v2ptr->ccastats_us[CCASTATS_NOPKT]) +
			dtoh64(chstats_us_v2ptr->ccastats_us[CCASTATS_NOCTG]);
		stats[i].ch_utilization_busy_self =
			dtoh64(chstats_us_v2ptr->ccastats_us[CCASTATS_INBSS]);
		stats[i].ch_utilization_busy_ext =
			dtoh64(chstats_us_v2ptr->rxcrs_sec20);
	}

exit:
	kfree(buf);
	kfree(curr_buf);
	return ret;
}

static int
wl_cfgvendor_get_survey_reply(struct wiphy *wiphy, struct wireless_dev *wdev,
	wl_cfgvendor_channel_stats_t *stats, int stats_num)
{
	int i, ret;
	struct sk_buff *skb;
	struct nlattr *nlattr_survey, *nlattr_freq;

	skb = cfg80211_vendor_cmd_alloc_reply_skb(wiphy, NLMSG_DEFAULT_SIZE);
	if (!skb) {
		WL_ERR(("Failed to allocate skb\n"));
		return -ENOMEM;
	}

	nlattr_survey = nla_nest_start(skb, RDK_VENDOR_ATTR_SURVEY_INFO);
	if (!nlattr_survey) {
		WL_ERR(("Failed to add survey info attribute\n"));
		ret = -EMSGSIZE;
		goto error;
	}

	for (i = 0; i < stats_num; i++) {
		nlattr_freq = nla_nest_start(skb, i);
		if (!nlattr_freq) {
			WL_ERR(("Failed to add freq attribute\n"));
			ret = -EMSGSIZE;
			goto error;
		}

		ret = nla_put_u32(skb, RDK_VENDOR_ATTR_SURVEY_INFO_FREQUENCY,
			wf_chanspec2mhz(stats[i].ch_spec));
		if (ret < 0) {
			WL_ERR(("Failed to add freq attribute, error: %d\n", ret));
			goto error;
		}

		ret = nla_put_s32(skb, RDK_VENDOR_ATTR_SURVEY_INFO_NOISE, stats[i].ch_noise);
		if (ret < 0) {
			WL_ERR(("Failed to add noise attribute, error: %d\n", ret));
			goto error;
		}

		ret = nla_put_s32(skb, RDK_VENDOR_ATTR_SURVEY_INFO_RADAR_NOISE,
			stats[i].ch_radar_noise);
		if (ret < 0) {
			WL_ERR(("Failed to add radar noise attribute, error: %d\n", ret));
			goto error;
		}

		ret = nla_put_s32(skb, RDK_VENDOR_ATTR_SURVEY_INFO_MAX_RSSI,
			stats[i].ch_max_80211_rssi);
		if (ret < 0) {
			WL_ERR(("Failed to add max rssi attribute, error: %d\n", ret));
			goto error;
		}

		ret = nla_put_s32(skb, RDK_VENDOR_ATTR_SURVEY_INFO_NON_80211_NOISE,
			stats[i].ch_non_80211_noise);
		if (ret < 0) {
			WL_ERR(("Failed to add non 80211 noise attribute, error: %d\n", ret));
			goto error;
		}

		ret = nla_put_u8(skb, RDK_VENDOR_ATTR_SURVEY_INFO_CHAN_UTIL,
			stats[i].ch_utilization);
		if (ret < 0) {
			WL_ERR(("Failed to add chan util attribute, error: %d\n", ret));
			goto error;
		}

		ret = nla_put_u64_64bit(skb, RDK_VENDOR_ATTR_SURVEY_INFO_TIME_ACTIVE,
			stats[i].ch_utilization_total, RDK_VENDOR_ATTR_SURVEY_INFO_PAD);
		if (ret < 0) {
			WL_ERR(("Failed to add time active attribute, error: %d\n", ret));
			goto error;
		}

		ret = nla_put_u64_64bit(skb, RDK_VENDOR_ATTR_SURVEY_INFO_TIME_BUSY,
			stats[i].ch_utilization_busy, RDK_VENDOR_ATTR_SURVEY_INFO_PAD);
		if (ret < 0) {
			WL_ERR(("Failed to add time busy attribute, error: %d\n", ret));
			goto error;
		}

		ret = nla_put_u64_64bit(skb, RDK_VENDOR_ATTR_SURVEY_INFO_TIME_BUSY_TX,
			stats[i].ch_utilization_busy_tx, RDK_VENDOR_ATTR_SURVEY_INFO_PAD);
		if (ret < 0) {
			WL_ERR(("Failed to add time busy tx attribute, error: %d\n", ret));
			goto error;
		}

		ret = nla_put_u64_64bit(skb, RDK_VENDOR_ATTR_SURVEY_INFO_TIME_BUSY_RX,
			stats[i].ch_utilization_busy_rx, RDK_VENDOR_ATTR_SURVEY_INFO_PAD);
		if (ret < 0) {
			WL_ERR(("Failed to add time busy rx attribute, error: %d\n", ret));
			goto error;
		}

		ret = nla_put_u64_64bit(skb, RDK_VENDOR_ATTR_SURVEY_INFO_TIME_BUSY_RX_SELF,
			stats[i].ch_utilization_busy_self, RDK_VENDOR_ATTR_SURVEY_INFO_PAD);
		if (ret < 0) {
			WL_ERR(("Failed to add time busy self attribute, error: %d\n", ret));
			goto error;
		}

		ret = nla_put_u64_64bit(skb, RDK_VENDOR_ATTR_SURVEY_INFO_TIME_EXT_BUSY,
			stats[i].ch_utilization_busy_ext, RDK_VENDOR_ATTR_SURVEY_INFO_PAD);
		if (ret < 0) {
			WL_ERR(("Failed to add time busy ext attribute, error: %d\n", ret));
			goto error;
		}

		nla_nest_end(skb, nlattr_freq);
	}

	nla_nest_end(skb, nlattr_survey);

	ret = cfg80211_vendor_cmd_reply(skb);
	if (ret < 0) {
		WL_ERR(("Failed to send reply, error: %d\n", ret));
	}
	return ret;
error:
	kfree_skb(skb);
	return ret;
}

static int
wl_cfgvendor_get_survey_handler(struct wiphy *wiphy, struct wireless_dev *wdev,
	const void *data, int len)
{
	int ret, stats_num;
	wl_cfgvendor_channel_stats_t *stats;

	stats = kmalloc(sizeof(wl_cfgvendor_channel_stats_t) * WL_CHANIM_COUNT_ALL, GFP_KERNEL);
	if (!stats) {
		WL_ERR(("Failed to alloc stats buffer\n"));
		return -ENOMEM;
	}
	ret = wl_cfgvendor_get_channel_stats(wiphy, wdev, stats, WL_CHANIM_COUNT_ALL, &stats_num);
	if (ret < 0) {
		WL_ERR(("Failed to get channel stats, error: %d\n", ret));
		goto error;
	}

	ret = wl_cfgvendor_get_survey_reply(wiphy, wdev, stats, stats_num);
	if (ret < 0) {
		WL_ERR(("Failed to send station list, error: %d\n", ret));
		goto error;
	}
error:
	kfree(stats);
	return ret;
}

#ifdef WL_CSIMON_CUST_SENDUP
int rdk_nl80211_csimon_sendup(struct wiphy *wiphy, struct wireless_dev *wdev,
	void *buf, ssize_t len)
{
	struct nlattr *nlattr;
	struct sk_buff *skb;
	bcm_csimon_hdr_t *hdr;
	bcm_csi_rpt_config_t *cfg;
	wifi_frame_info_t hal_frame_info;
	int n, ret;

	RDK_WL_CSI_DBG(("%s@%d Enter ... \n", __FUNCTION__, __LINE__));

	if (buf == NULL) {
		RDK_WL_CSI_ERR(("buf is NULL. exit\n"));
		return BCME_ERROR;
	}

	hdr = (bcm_csimon_hdr_t *)buf;
	cfg = (bcm_csi_rpt_config_t *)(hdr + 1);

	RDK_WL_CSI_INFO(("%s@%d wiphy:%p wdev:%p buf:%p len:%d", __FUNCTION__, __LINE__,
		(void *)wiphy, (void *)wdev, (void *)buf, len));

	if (RDK_WL_CSI_VERBOSE_ENABLED) {
		RDK_WL_CSI_PRT(("Start %s: Header:\n", __FUNCTION__));
		RDK_WL_CSI_PRT(("frameid:%x\n", hdr->data.format_id));
		RDK_WL_CSI_PRT(("Client-ea:"MACF"\n", ETHERP_TO_MACF(hdr->data.client_ea)));
		RDK_WL_CSI_PRT(("bss-ea:"MACF"\n", ETHERP_TO_MACF(hdr->data.bss_ea)));
		RDK_WL_CSI_PRT(("chaspec:%04X\n", hdr->data.chanspec));
		RDK_WL_CSI_PRT(("tx:%X\n", hdr->data.txstreams));
		RDK_WL_CSI_PRT(("rx:%X\n", hdr->data.rxstreams));
		RDK_WL_CSI_PRT(("report_ts:0x%x\n", hdr->data.report_ts));
		RDK_WL_CSI_PRT(("assoc_ts:0x%x\n", hdr->data.assoc_ts));
		RDK_WL_CSI_PRT(("rssi:%02x:%02x:%02x:%02x\n", hdr->data.rssi[0], hdr->data.rssi[1],
			hdr->data.rssi[2], hdr->data.rssi[3]));
		RDK_WL_CSI_PRT(("cfo:0x%x\n", hdr->data.cfo));
	}

	/* fillout HAL csi header */
	hal_frame_info.phy_bw = 20 * (1 << CSI_EXTRACT_CONFIG_WORD(cfg->config, PHYBW));
	hal_frame_info.cap_bw = 20 * (1 << CSI_EXTRACT_CONFIG_WORD(cfg->config, PKTBW));
	hal_frame_info.bw_mode = 1 << (CSI_EXTRACT_CONFIG_WORD(cfg->config, PKTBW));
	hal_frame_info.Nr = CSI_EXTRACT_CONFIG_WORD(cfg->config, NTX) + 1;

	/* Brcm only support 1 stream */
	hal_frame_info.Nc = 1;
	hal_frame_info.num_sc = CSI_EXTRACT_CONFIG_WORD_SPILL(cfg->config, NUM_TONES);

	/* Not applied to Brcm CSI format */
	hal_frame_info.valid_mask = 0xffff;

	/* Brcm only support decimal 0 */
	hal_frame_info.decimation = 0;
	hal_frame_info.channel = wf_chspec_ctlchan(hdr->data.chanspec);
	hal_frame_info.cfo = (int32)(hdr->data.cfo);

	for (n = 0; n < 4; n++) {
		hal_frame_info.nr_rssi[n] = hdr->data.rssi[n];
	}

	if (RDK_WL_CSI_VERBOSE_ENABLED) {
		RDK_WL_CSI_PRT(("frame_info: phy_bw:%x cap_bw:%x Nr:%x Nc:%x num_sc:%x cfo:%x\n",
			hal_frame_info.phy_bw, hal_frame_info.cap_bw, hal_frame_info.Nr,
			hal_frame_info.Nc, hal_frame_info.num_sc, hal_frame_info.cfo));
	}

	/* send csi event, +256 byte for attribute header */
	skb = cfg80211_vendor_event_alloc(wiphy, wdev, len + 256, rdk_csi_event_idx, GFP_KERNEL);

	if (skb) {
		nlattr = nla_nest_start(skb, RDK_VENDOR_ATTR_CSI_INFO);

		/* Add vendor data */
		ret = nla_put(skb, RDK_VENDOR_ATTR_CSI_INFO_MAC, ETH_ALEN,
			(char *)hdr->data.client_ea);
		if (ret < 0) {
			RDK_WL_CSI_ERR(("Failed to RDK_VENDOR_ATTR_CSI_INFO_MAC attribute,"
				"error: %d\n", ret));
			goto error;
		}

		ret = nla_put_u8(skb, RDK_VENDOR_ATTR_CSI_INFO_FRAME_BW, 0);
		if (ret < 0) {
			RDK_WL_CSI_ERR(("Failed to RDK_VENDOR_ATTR_CSI_INFO_FRAME_BW "
				"attribute, error: %d\n", ret));
			goto error;
		}

		ret = nla_put_u32(skb, RDK_VENDOR_ATTR_CSI_FRAME_STANDARD, 0);
		if (ret < 0) {
			RDK_WL_CSI_ERR(("Failed to RDK_VENDOR_ATTR_CSI_FRAME_STANDARD "
				"attribute, error: %d\n", ret));
			goto error;
		}

		ret = nla_put_u8(skb, RDK_VENDOR_ATTR_CSI_FRAME_MCS, 0);
		if (ret < 0) {
			RDK_WL_CSI_ERR(("Failed to RDK_VENDOR_ATTR_CSI_FRAME_MCS "
				"attribute, error: %d\n", ret));
			goto error;
		}

		ret = nla_put_u8(skb, RDK_VENDOR_ATTR_CSI_FRAME_NR, hal_frame_info.Nr);
		if (ret < 0) {
			RDK_WL_CSI_ERR(("Failed to RDK_VENDOR_ATTR_CSI_FRAME_NR "
				"attribute, error: %d\n", ret));
			goto error;
		}

		ret = nla_put_u8(skb, RDK_VENDOR_ATTR_CSI_FRAME_NC, hal_frame_info.Nc);
		if (ret < 0) {
			RDK_WL_CSI_ERR(("Failed to RDK_VENDOR_ATTR_CSI_FRAME_NC "
				"attribute, error: %d\n", ret));
			goto error;
		}

		ret = nla_put(skb, RDK_VENDOR_ATTR_CSI_FRAME_NR_RSSI,
			sizeof(hal_frame_info.nr_rssi), hal_frame_info.nr_rssi);
		if (ret < 0) {
			WL_ERR(("Failed to RDK_VENDOR_ATTR_CSI_FRAME_NR_RSSI attribute,"
				"error: %d\n", ret));
			goto error;
		}

		ret = nla_put_u16(skb, RDK_VENDOR_ATTR_CSI_FRAME_VALID_MASK,
			hal_frame_info.valid_mask);
		if (ret < 0) {
			RDK_WL_CSI_ERR(("Failed to RDK_VENDOR_ATTR_CSI_FRAME_VALID_MASK "
				"attribute, error: %d\n", ret));
			goto error;
		}

		ret = nla_put_u16(skb, RDK_VENDOR_ATTR_CSI_FRAME_PHY_BW, hal_frame_info.phy_bw);
		if (ret < 0) {
			RDK_WL_CSI_ERR(("Failed to RDK_VENDOR_ATTR_CSI_FRAME_PHY_BW "
				"attribute, error: %d\n", ret));
			goto error;
		}

		ret = nla_put_u16(skb, RDK_VENDOR_ATTR_CSI_FRAME_CAP_BW, hal_frame_info.cap_bw);
		if (ret < 0) {
			RDK_WL_CSI_ERR(("Failed to RDK_VENDOR_ATTR_CSI_FRAME_CAP_BW "
				"attribute, error: %d\n", ret));
			goto error;
		}
		ret = nla_put_u32(skb, RDK_VENDOR_ATTR_CSI_FRAME_NUM_SC, hal_frame_info.num_sc);
		if (ret < 0) {
			RDK_WL_CSI_ERR(("Failed to RDK_VENDOR_ATTR_CSI_FRAME_NUM_SC "
				"attribute, error: %d\n", ret));
			goto error;
		}

		ret = nla_put_u32(skb, RDK_VENDOR_ATTR_CSI_FRAME_NUM_PILOTS, 0);
		if (ret < 0) {
			RDK_WL_CSI_ERR(("Failed to RDK_VENDOR_ATTR_CSI_FRAME_NUM_PILOTS "
				"attribute, error: %d\n", ret));
			goto error;
		}

		ret = nla_put_u8(skb, RDK_VENDOR_ATTR_CSI_FRAME_DECIMATION,
			hal_frame_info.decimation);
		if (ret < 0) {
			WL_ERR(("Failed to RDK_VENDOR_ATTR_CSI_FRAME_DECIMATION "
				"attribute, error: %d\n", ret));
			goto error;
		}

		ret = nla_put_u32(skb, RDK_VENDOR_ATTR_CSI_FRAME_CHANNEL, hal_frame_info.channel);
		if (ret < 0) {
			RDK_WL_CSI_ERR(("Failed to RDK_VENDOR_ATTR_CSI_FRAME_CHANNEL "
				"attribute, error: %d\n", ret));
			goto error;
		}

		ret = nla_put_s32(skb, RDK_VENDOR_ATTR_CSI_FRAME_CFO, hal_frame_info.cfo);
		if (ret < 0) {
			RDK_WL_CSI_ERR(("Failed to RDK_VENDOR_ATTR_CSI_FRAME_CFO attribute,"
				"error: %d\n", ret));
			goto error;
		}

		/* set timestamp as 0. Upper apps will recalculate */
		ret = nla_put_u64_64bit(skb, RDK_VENDOR_ATTR_CSI_FRAME_TIMESTAMP,
			0, RDK_VENDOR_ATTR_CSI_INFO_PAD);
		if (ret < 0) {
			RDK_WL_CSI_ERR(("Failed to RDK_VENDOR_ATTR_CSI_FRAME_TIMESTAMP"
			" attribute, error: %d\n", ret));
			goto error;
		}

		ret = nla_put(skb, RDK_VENDOR_ATTR_CSI_MATRIX, len, buf);
		if (ret < 0) {
			RDK_WL_CSI_ERR(("Failed to RDK_VENDOR_ATTR_CSI_MATRIX attribute,"
			"error: %d\n", ret));
			goto error;
		}

		nla_nest_end(skb, nlattr);

		cfg80211_vendor_event(skb, GFP_KERNEL);
		RDK_WL_CSI_INFO(("Send CSI event: %s@%d \n", __FUNCTION__, __LINE__));
	}

	RDK_WL_CSI_DBG(("%s@%d Exit... \n", __FUNCTION__, __LINE__));
	return BCME_OK;

error:
	kfree_skb(skb);
	return ret;
}
#endif /* WL_CSIMON_CUST_SENDUP */

static int
wl_cfgvendor_add_del_csi_station_handler(struct wiphy *wiphy, struct wireless_dev *wdev,
		const void *data, int len, wl_csimon_cfg_cmd_type_t cmd)
{
	int ret;
	u8 mac[ETH_ALEN];
	char mac_str[ETHER_ADDR_STR_LEN];
	u8 *buf;
	wlc_csimon_sta_config_t csimon_cfg;

	struct nlattr *tb[RDK_VENDOR_ATTR_MAX + 1];
	struct net_device *ndev;
	struct bcm_cfg80211 *cfg = wiphy_priv(wiphy);

	RDK_WL_CSI_DBG(("%s@%d Enter...cmd[%d] \n", __FUNCTION__, __LINE__, cmd));

	ndev = wdev_to_wlc_ndev(wdev, cfg);
	if (!ndev) {
		RDK_WL_CSI_ERR(("Failed to get ndev\n"));
		return BCME_ERROR;
	}

	ret = nla_parse(tb, RDK_VENDOR_ATTR_MAX, data, len,
		wl_cfgvendor_add_del_csi_station_policy, NULL);
	if (ret < 0) {
		RDK_WL_CSI_ERR(("%s@%d Failed to parse get station attributes,"
		" error: %d\n", __FUNCTION__, __LINE__, ret));
		return ret;
	}

	if (!tb[RDK_VENDOR_ATTR_MAC]) {
		RDK_WL_CSI_ERR(("RDK_VENDOR_ATTR_MAC attribute missed\n"));
		return -EINVAL;
	}

	memcpy(mac, nla_data(tb[RDK_VENDOR_ATTR_MAC]), ETH_ALEN);

	bcm_ether_ntoa((struct ether_addr *)mac, mac_str);
	RDK_WL_CSI_INFO(("%s@%d Add mac:%s \n", __FUNCTION__, __LINE__, mac_str));

	memset(&csimon_cfg, 0, sizeof(wlc_csimon_sta_config_t));
	csimon_cfg.version = htod16(CSIMON_STACONFIG_VER);
	csimon_cfg.length = htod16(CSIMON_STACONFIG_LENGTH);

	csimon_cfg.cmd = htodenum(cmd);
	csimon_cfg.monitor_interval = CSI_DELAY_PERIOD;
	memcpy(&csimon_cfg.ea, mac, ETH_ALEN);

	RDK_WL_CSI_INFO(("%s@%d ver:%d[%d] len:%d cmd:%d \n", __FUNCTION__, __LINE__,
		csimon_cfg.version, CSIMON_STACONFIG_VER,
		csimon_cfg.length, csimon_cfg.cmd));

	buf = kmalloc(WLC_IOCTL_MEDLEN, GFP_KERNEL);
	if (buf == NULL) {
		RDK_WL_CSI_ERR(("%s: Failed to alloc buffer\n", __FUNCTION__));
		return -ENOMEM;
	}
	ret = wldev_iovar_setbuf(ndev, "csimon", &csimon_cfg,
		sizeof(wlc_csimon_sta_config_t), buf, WLC_IOCTL_MEDLEN, NULL);
	kfree(buf);

	RDK_WL_CSI_DBG(("%s@%d Exit...\n", __FUNCTION__, __LINE__));

	return ret;
}

static int
wl_cfgvendor_add_csi_station_handler(struct wiphy *wiphy, struct wireless_dev *wdev,
		const void *data, int len)
{
	int ret;

	ret = wl_cfgvendor_add_del_csi_station_handler(wiphy, wdev, data, len, CSIMON_CFG_CMD_ADD);
	return ret;
}

static int
wl_cfgvendor_del_csi_station_handler(struct wiphy *wiphy, struct wireless_dev *wdev,
		const void *data, int len)
{
	int ret;

	ret = wl_cfgvendor_add_del_csi_station_handler(wiphy, wdev, data, len, CSIMON_CFG_CMD_DEL);
	return ret;
}

static int
wl_cfgvendor_get_csi_station_handler(struct wiphy *wiphy, struct wireless_dev *wdev,
		const void *data, int len)
{
	int ret, i;
	struct sk_buff *skb;
	struct nlattr *nlattr;
	struct ether_addr *ea;
	char mac_str[ETHER_ADDR_STR_LEN];
	u8 *buf;
	struct maclist *maclist;
	struct net_device *ndev;
	struct bcm_cfg80211 *cfg = wiphy_priv(wiphy);

	RDK_WL_CSI_DBG(("%s@%d Enter...\n", __FUNCTION__, __LINE__));

	ndev = wdev_to_wlc_ndev(wdev, cfg);
	if (!ndev) {
		RDK_WL_CSI_ERR(("Failed to get ndev\n"));
		return BCME_ERROR;
	}

	skb = cfg80211_vendor_cmd_alloc_reply_skb(wiphy, NLMSG_DEFAULT_SIZE);
	if (!skb) {
		RDK_WL_CSI_ERR(("Failed to allocate skb\n"));
		return -ENOMEM;
	}

	buf = kmalloc(WLC_IOCTL_MEDLEN, GFP_KERNEL);
	if (buf == NULL) {
		kfree(skb);
		RDK_WL_CSI_ERR(("%s: Failed to alloc buffer\n", __FUNCTION__));
		return -ENOMEM;
	}
	ret = wldev_iovar_getbuf(ndev, "csimon", NULL, 0, buf, WLC_IOCTL_MEDLEN, NULL);
	RDK_WL_CSI_INFO(("%s@%d calling csimon get. ret :%d\n", __FUNCTION__, __LINE__, ret));
	if (ret < 0) {
		RDK_WL_CSI_ERR(("Failed to read csimon error: %d\n", ret));
		goto error;
	}

	maclist = (struct maclist *) buf;
	maclist->count = dtoh32(maclist->count);
	RDK_WL_CSI_INFO(("%s@%d mac_cnt[%d]\n", __FUNCTION__, __LINE__, maclist->count));

	nlattr = nla_nest_start(skb, RDK_VENDOR_ATTR_STA_LIST);
	if (!nlattr) {
		RDK_WL_CSI_ERR(("Failed to add station list attribute\n"));
		ret = -EMSGSIZE;
		goto error;
	}

	for (i = 0, ea = maclist->ea; i < maclist->count && i < CSI_MAX_STA; i++, ea++) {
		bcm_ether_ntoa(ea, mac_str);
		RDK_WL_CSI_INFO(("%s@%d put... mac[%d]:%s \n", __FUNCTION__, __LINE__, i, mac_str));

		ret = nla_put(skb, RDK_VENDOR_ATTR_MAC, ETH_ALEN, (char *)ea);
		if (ret < 0) {
			RDK_WL_CSI_ERR(("Failed to add MAC address attribute, error: %d\n", ret));
			goto error;
		}
	}

	nla_nest_end(skb, nlattr);
	ret = cfg80211_vendor_cmd_reply(skb);
	if (ret < 0) {
		RDK_WL_CSI_ERR(("Failed to send reply, error: %d\n", ret));
	}
	kfree(buf);

	RDK_WL_CSI_DBG(("%s@%d Exit... \n", __FUNCTION__, __LINE__));
	return ret;

error:
	kfree_skb(skb);
	kfree(buf);
	return ret;
}

static int
wl_cfgvendor_set_csi_handler(struct wiphy *wiphy, struct wireless_dev *wdev,
		const void *data, int len)
{
	int ret;
	u8 enable, *buf;
	wlc_csimon_sta_config_t csimon_cfg;
	struct net_device *ndev;
	struct bcm_cfg80211 *cfg = wiphy_priv(wiphy);
	struct nlattr *tb[RDK_VENDOR_ATTR_MAX + 1];

	RDK_WL_CSI_DBG(("%s@%d Enter... \n", __FUNCTION__, __LINE__));

	ndev = wdev_to_wlc_ndev(wdev, cfg);
	if (!ndev) {
		RDK_WL_CSI_ERR(("Failed to get ndev\n"));
		return BCME_ERROR;
	}

	ret = nla_parse(tb, RDK_VENDOR_ATTR_MAX, data, len, wl_cfgvendor_set_csi_policy,
		NULL);
	if (ret < 0) {
		RDK_WL_CSI_ERR(("Failed to parse get station attributes, error: %d\n", ret));
		return ret;
	}

	if (!tb[RDK_VENDOR_ATTR_CSI_ENABLE]) {
		RDK_WL_CSI_ERR(("RDK_VENDOR_ATTR_CSI_ENABLE attribute missed\n"));
		return -EINVAL;
	}
	enable = nla_get_u8(tb[RDK_VENDOR_ATTR_CSI_ENABLE]);
	RDK_WL_CSI_INFO(("%s@%d enable:%d \n", __FUNCTION__, __LINE__, enable));

	memset(&csimon_cfg, 0, sizeof(wlc_csimon_sta_config_t));
	csimon_cfg.version = htod16(CSIMON_STACONFIG_VER);
	csimon_cfg.length = htod16(CSIMON_STACONFIG_LENGTH);
	if (enable)
		csimon_cfg.cmd = htodenum(CSIMON_CFG_CMD_ENB);
	else
		csimon_cfg.cmd = htodenum(CSIMON_CFG_CMD_DSB);

	RDK_WL_CSI_INFO(("%s@%d ver:%d[%d] len:%d cmd:%d \n", __FUNCTION__, __LINE__,
		csimon_cfg.version, CSIMON_STACONFIG_VER,
		csimon_cfg.length, csimon_cfg.cmd));

	buf = kmalloc(WLC_IOCTL_MEDLEN, GFP_KERNEL);
	if (buf == NULL) {
		RDK_WL_CSI_ERR(("%s: Failed to alloc buffer\n", __FUNCTION__));
		return -ENOMEM;
	}
	ret = wldev_iovar_setbuf(ndev, "csimon", &csimon_cfg,
			sizeof(wlc_csimon_sta_config_t), buf, WLC_IOCTL_MEDLEN, NULL);
	if (ret < 0) {
		RDK_WL_CSI_ERR(("%s@%d Exit... CSI Enable failure. ret:%d\n",
			__FUNCTION__, __LINE__, ret));
		kfree(buf);
		return ret;
	}
	kfree(buf);
	RDK_WL_CSI_INFO(("%s@%d Exit CSI Enable Success.\n", __FUNCTION__, __LINE__));

	/* Calling to register csimon */
#ifdef WL_CSIMON_CUST_SENDUP
	if (enable) {
		RDK_WL_CSI_INFO(("Register rdk_nl80211_csimon_sendup:%p evet_idx:%d\n",
			(void *)rdk_nl80211_csimon_sendup, rdk_csi_event_idx));
		dhd_linux_csimon_cb_register(rdk_nl80211_csimon_sendup);
	}
	else {
		RDK_WL_CSI_INFO(("De Register rdk_nl80211_csimon_sendup \n"));
		dhd_linux_csimon_cb_register((dhd_linux_csimon_func_t)NULL);
	}
#endif /* WL_CSIMON_CUST_SENDUP */

	RDK_WL_CSI_DBG(("%s@%d Exit... \n", __FUNCTION__, __LINE__));
	return ret;
}

static int
wl_cfgvendor_msglevel_handler(struct wiphy *wiphy, struct wireless_dev *wdev,
		const void *data, int len)
{
	int ret;
	struct net_device *ndev;
	struct bcm_cfg80211 *cfg = wiphy_priv(wiphy);
	struct nlattr *tb[RDK_VENDOR_ATTR_MAX + 1];

	RDK_WL_CSI_DBG(("%s@%d Enter... \n", __FUNCTION__, __LINE__));

	ndev = wdev_to_wlc_ndev(wdev, cfg);
	if (!ndev) {
		WL_ERR(("Failed to get ndev\n"));
		return BCME_ERROR;
	}

	ret = nla_parse(tb, RDK_VENDOR_ATTR_MAX, data, len, wl_cfgvendor_msglevel_policy,
		NULL);
	if (ret < 0) {
		RDK_WL_CSI_ERR(("Failed to parse get station attributes, error: %d\n", ret));
		return ret;
	}

	if (!tb[RDK_VENDOR_ATTR_MSGLEVEL]) {
		RDK_WL_CSI_ERR(("RDK_VENDOR_ATTR_CSI_ENABLE attribute missed\n"));
		return -EINVAL;
	}

	rdk_msglevel = nla_get_u32(tb[RDK_VENDOR_ATTR_MSGLEVEL]);
	RDK_WL_CSI_PRT(("rdk_msglevel:0x%x\n", rdk_msglevel));
	return ret;
}

static int
wl_cfgvendor_get_csi_handler(struct wiphy *wiphy, struct wireless_dev *wdev,
		const void *data, int len)
{
	int ret;
	struct sk_buff *skb;
	u8 enable, *buf;
	csimon_state_t *state = NULL;
	struct net_device *ndev;
	struct bcm_cfg80211 *cfg = wiphy_priv(wiphy);

	RDK_WL_CSI_DBG(("%s@%d Enter... NLMSG_DEFAULT_SIZE:%ld \n",
		__FUNCTION__, __LINE__, NLMSG_DEFAULT_SIZE));

	ndev = wdev_to_wlc_ndev(wdev, cfg);
	if (!ndev) {
		RDK_WL_CSI_ERR(("Failed to get ndev\n"));
		return BCME_ERROR;
	}

	skb = cfg80211_vendor_cmd_alloc_reply_skb(wiphy, NLMSG_DEFAULT_SIZE);
	if (!skb) {
		RDK_WL_CSI_ERR(("Failed to allocate skb\n"));
		return -ENOMEM;
	}

	buf = kmalloc(WLC_IOCTL_MEDLEN, GFP_KERNEL);
	if (buf == NULL) {
		kfree(skb);
		RDK_WL_CSI_ERR(("%s: Failed to alloc buffer\n", __FUNCTION__));
		return -ENOMEM;
	}
	ret = wldev_iovar_getbuf(ndev, "csimon_state", NULL, 0, buf, WLC_IOCTL_MEDLEN, NULL);
	if (ret < 0) {
		RDK_WL_CSI_ERR(("Failed to read csimon_state, error: %d\n", ret));
		goto error;
	}

	state = (csimon_state_t *)buf;
	enable = state->enabled;

	RDK_WL_CSI_INFO(("%s@%d add enable: RDK_VENDOR_ATTR_CSI_ENABLE:%d enable:%d\n",
		__FUNCTION__, __LINE__, RDK_VENDOR_ATTR_CSI_ENABLE, enable));
	ret = nla_put_u8(skb, RDK_VENDOR_ATTR_CSI_ENABLE, enable);
	if (ret < 0) {
		RDK_WL_CSI_ERR(("Failed to add enable attribute, error: %d\n", ret));
		goto error;
	}
	RDK_WL_CSI_INFO(("calling vendor_cmd_reply: %s@%d \n", __FUNCTION__, __LINE__));

	ret = cfg80211_vendor_cmd_reply(skb);
	if (ret < 0) {
		RDK_WL_CSI_ERR(("Failed to send reply, error: %d\n", ret));
	}

	kfree(buf);
	RDK_WL_CSI_DBG(("%s@%d Exit... \n", __FUNCTION__, __LINE__));
	return ret;

error:
	kfree(skb);
	kfree(buf);

	WL_ERR(("%s@%d Error Exit... ret:%d \n", __FUNCTION__, __LINE__, ret));
	return ret;
}

static int
wl_cfgvendor_get_mgt_frame_pwr_params(const void *data, int *pwr, int len)
{
	int ret;
	struct nlattr *tb[RDK_VENDOR_ATTR_MAX + 1];

	ret = nla_parse(tb, RDK_VENDOR_ATTR_MAX, data, len,
		wl_cfgvendor_set_mgt_frame_pwr_policy, NULL);
	if (ret < 0) {
		WL_ERR(("Failed to parse mgt frame pwr attributes, error: %d\n", ret));
		return ret;
	}

	if (!tb[RDK_VENDOR_ATTR_MGT_FRAME_PWR_LEVEL]) {
		WL_ERR(("mgt frame pwr level attribute is missing\n"));
		return -EINVAL;
	}
	*pwr = nla_get_s32(tb[RDK_VENDOR_ATTR_MGT_FRAME_PWR_LEVEL]);

	return ret;
}

static int
wl_cfgvendor_set_mgt_frame_pwr_ioctl(struct wiphy *wiphy, struct wireless_dev *wdev,
	int pwr)
{
	int ret;
	u32 buflen;
	struct net_device *ndev;
	char buf[WLC_IOCTL_SMLEN] = {0};
	struct bcm_cfg80211 *cfg = wiphy_priv(wiphy);

	if (pwr < 0)
		pwr = -pwr;

	ndev = wdev_to_wlc_ndev(wdev, cfg);
	if (!ndev) {
		WL_ERR(("Failed to get ndev\n"));
		return BCME_ERROR;
	}

	strncpy(buf, "bcnprs_txpwr_offset", sizeof(buf) - 1);
	buflen = strlen(buf) + 1;
	memcpy(buf + buflen, &pwr, sizeof(pwr));
	ret = wldev_ioctl_set(ndev, WLC_SET_VAR, buf, sizeof(buf));
	if (ret < 0) {
		WL_ERR(("bcnprs_txpwr_offset ioctl failed, error: %d\n", ret));
		return ret;
	}

	return ret;
}

static int
wl_cfgvendor_set_mgt_frame_pwr_handler(struct wiphy *wiphy, struct wireless_dev *wdev,
	const void *data, int len)
{
	int ret, pwr;

	ret = wl_cfgvendor_get_mgt_frame_pwr_params(data, &pwr, len);
	if (ret < 0) {
		WL_ERR(("Failed to get mgt frame pwr params, error: %d\n", ret));
		return ret;
	}

	ret = wl_cfgvendor_set_mgt_frame_pwr_ioctl(wiphy, wdev, pwr);
	if (ret < 0) {
		WL_ERR(("Failed to set mgt frame pwr, error: %d\n", ret));
		return ret;
	}

	return 0;
}

#ifdef BCM_WLMLO
static int
wl_cfgvendor_get_mld_params(const void *data, int len, u8 *mld_enable, u8 *mld_unit,
	u8 *link_id, u8 *mac, int *set_mac, u8 *apply)
{
	int ret;
	struct nlattr *tb[RDK_VENDOR_ATTR_MAX + 1];
	u8 val;

	ret = nla_parse(tb, RDK_VENDOR_ATTR_MAX, data, len,
		wl_cfgvendor_set_mld_policy, NULL);
	if (ret < 0) {
		WL_ERR(("Failed to parse mld attributes, error: %d\n", ret));
		return ret;
	}

	if (!tb[RDK_VENDOR_ATTR_MLD_ENABLE]) {
		WL_ERR(("MLD_ENABLE attribute is missing\n"));
		return -EINVAL;
	}

	*mld_enable = nla_get_u8(tb[RDK_VENDOR_ATTR_MLD_ENABLE]);
	if (*mld_enable) {
		if (!tb[RDK_VENDOR_ATTR_MLD_UNIT]) {
			WL_ERR(("MLD_UNIT attribute is missing\n"));
			return -EINVAL;
		}

		*mld_unit = nla_get_u8(tb[RDK_VENDOR_ATTR_MLD_UNIT]);
		if (*mld_unit != (u8)-1 && *mld_unit >= ((RDK_VENDOR_MAX_NUM_MLD_UNIT > WL_MAXMLDUNIT) ?
		    WL_MAXMLDUNIT : RDK_VENDOR_MAX_NUM_MLD_UNIT)) {
			WL_ERR(("Invalid MLD_UNIT: %u\n", *mld_unit));
			return -EINVAL;
		}

		if (tb[RDK_VENDOR_ATTR_MLD_LINK_ID]) {
			*link_id = nla_get_u8(tb[RDK_VENDOR_ATTR_MLD_LINK_ID]);
			if ((*link_id != (u8)-1) && *link_id >= RDK_VENDOR_MAX_NUM_MLD_LINKS) {
				WL_ERR(("Invalid mld_link_id: %u\n", *link_id));
				return -EINVAL;
			}
		}
		else {
			*link_id = (u8)-1;
		}

		if (tb[RDK_VENDOR_ATTR_MLD_MAC]) {
			*set_mac = 1;
			(void)memcpy(mac, nla_data(tb[RDK_VENDOR_ATTR_MLD_MAC]), ETH_ALEN);
			WL_DBG(("MLD MAC: %02x:%02x:%02x:%02x:%02x:%02x\n",
				mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]));
		}
		else {
			*set_mac = 0;
			WL_DBG(("MLD_MAC attribute is not set\n"));
		}
	}
	else {
		*mld_unit = *link_id = (u8)-1;
		*set_mac = 0;
	}

	if (tb[RDK_VENDOR_ATTR_MLD_CONFIG_APPLY]) {
		val = nla_get_u8(tb[RDK_VENDOR_ATTR_MLD_CONFIG_APPLY]);
		*apply = val ? 1 : 0;
	}
	else {
		*apply = 0;
	}

	{	char eabuf[ETHER_ADDR_STR_LEN];

		WIFI_DBG("### %s[%s/%s]: mld_enable=%u, mld_unit=%u link_id=%u set_mac=%d mac=%s apply=%d ###\n",
			__FUNCTION__, current->comm, current->real_parent->comm, *mld_enable, *mld_unit, *link_id,
			*set_mac, bcm_ether_ntoa((struct ether_addr *)mac, eabuf), *apply);
	}
	return 0;
}

static int
wl_cfgvendor_set_mld_handler(struct wiphy *wiphy, struct wireless_dev *wdev,
	const void *data, int len)
{
	struct net_device *ndev;
	struct bcm_cfg80211 *cfg;
	int ret;
	u8 mld_enable;
	u8 mld_unit;
	u8 link_id;
	u8 mac[ETH_ALEN] = { };
	u8 apply;
	int wl_unit = -1, wl_subunit = -1, set_mac = 0;

	ret = wl_cfgvendor_get_mld_params(data, len, &mld_enable, &mld_unit, &link_id,
		mac, &set_mac, &apply);
	if (ret < 0) {
		WL_ERR(("Failed to get mld params, error: %d\n", ret));
		return ret;
	}

	cfg = wiphy_priv(wiphy);
	ndev = wdev_to_wlc_ndev(wdev, cfg);
	if (!ndev) {
		WL_ERR(("Failed to get ndev\n"));
		return -ENODEV;
	}

	if (bcm_get_ifname_unit(ndev->name, &wl_unit, &wl_subunit) < 0) {
		WL_ERR(("Invalid virtual interface name %s\n", ndev->name));
		return -ENODEV;
	}
	if (wl_subunit == -1) {
		wl_subunit = 0;
	} else {
		(void)wlhal_mld_ndev_reg(ndev, wl_unit, wl_subunit);
	}

	WIFI_DBG("### %s: ndev=%s wiphy=%s wl_unit=%d wl_subunit=%d ###\n", __FUNCTION__,
		wdev->netdev->name, wiphy_name(wiphy), wl_unit, wl_subunit);

	ret = wlhal_mld_cfg_set(wl_unit, wl_subunit, mld_enable, mld_unit != (u8)-1 ? mld_unit : -1);
	if (ret == 0 && set_mac)
		ret = wlhal_mld_mac_set(mld_unit, mac);
	if (ret == 0) {
		if (apply)
			ret = wlhal_mlo_apply();
		else
			wlhal_mlo_dump();
	}

	return ret;
}

#endif /* BCM_WLMLO */

static const struct wiphy_vendor_command rdk_nl80211_vendor_cmds [] = {
	{
		{
			.vendor_id = OUI_COMCAST,
			.subcmd = RDK_VENDOR_NL80211_SUBCMD_GET_STATION
		},
		.policy = wl_cfgvendor_get_station_policy,
		.flags = WIPHY_VENDOR_CMD_NEED_WDEV | WIPHY_VENDOR_CMD_NEED_NETDEV,
		.doit = wl_cfgvendor_get_station_handler,
		.maxattr = RDK_VENDOR_ATTR_MAX
	},
	{
		{
			.vendor_id = OUI_COMCAST,
			.subcmd = RDK_VENDOR_NL80211_SUBCMD_GET_STATION_LIST
		},
		.policy = wl_cfgvendor_get_station_list_policy,
		.flags = WIPHY_VENDOR_CMD_NEED_WDEV | WIPHY_VENDOR_CMD_NEED_NETDEV,
		.doit = wl_cfgvendor_get_station_list_handler,
		.maxattr = RDK_VENDOR_ATTR_MAX
	},
	{
		{
			.vendor_id = OUI_COMCAST,
			.subcmd = RDK_VENDOR_NL80211_SUBCMD_GET_WIPHY_TEMP
		},
		.policy = wl_cfgvendor_get_phy_temp_policy,
		.flags = WIPHY_VENDOR_CMD_NEED_WDEV | WIPHY_VENDOR_CMD_NEED_NETDEV,
		.doit = wl_cfgvendor_get_phy_temp_handler,
		.maxattr = RDK_VENDOR_ATTR_MAX
	},
	{
		{
			.vendor_id = OUI_COMCAST,
			.subcmd = RDK_VENDOR_NL80211_SUBCMD_GET_SURVEY
		},
		.policy = wl_cfgvendor_get_survey_policy,
		.flags = WIPHY_VENDOR_CMD_NEED_WDEV | WIPHY_VENDOR_CMD_NEED_NETDEV,
		.doit = wl_cfgvendor_get_survey_handler,
		.maxattr = RDK_VENDOR_ATTR_MAX
	},
	{
		{
			.vendor_id = OUI_COMCAST,
			.subcmd = RDK_VENDOR_NL80211_SUBCMD_SET_MGT_FRAME_PWR
		},
		.policy = wl_cfgvendor_set_mgt_frame_pwr_policy,
		.flags = WIPHY_VENDOR_CMD_NEED_WDEV | WIPHY_VENDOR_CMD_NEED_NETDEV,
		.doit = wl_cfgvendor_set_mgt_frame_pwr_handler,
		.maxattr = RDK_VENDOR_ATTR_MAX
	},
#ifdef BCM_WLMLO
	{
		{
			.vendor_id = OUI_COMCAST,
			.subcmd = RDK_VENDOR_NL80211_SUBCMD_SET_MLD
		},
		.policy = wl_cfgvendor_set_mld_policy,
		.flags = WIPHY_VENDOR_CMD_NEED_WDEV | WIPHY_VENDOR_CMD_NEED_NETDEV,
		.doit = wl_cfgvendor_set_mld_handler,
		.maxattr = RDK_VENDOR_ATTR_MAX
	},
#endif /* BCM_WLMLO */

	{
		{
			.vendor_id = OUI_COMCAST,
			.subcmd = RDK_VENDOR_NL80211_SUBCMD_ADD_CSI_STATION
		},
		.policy = wl_cfgvendor_add_del_csi_station_policy,
		.flags = WIPHY_VENDOR_CMD_NEED_WDEV | WIPHY_VENDOR_CMD_NEED_NETDEV,
		.doit = wl_cfgvendor_add_csi_station_handler,
		.maxattr = RDK_VENDOR_ATTR_MAX
	},

	{
		{
			.vendor_id = OUI_COMCAST,
			.subcmd = RDK_VENDOR_NL80211_SUBCMD_DEL_CSI_STATION
		},
		.policy = wl_cfgvendor_add_del_csi_station_policy,
		.flags = WIPHY_VENDOR_CMD_NEED_WDEV | WIPHY_VENDOR_CMD_NEED_NETDEV,
		.doit = wl_cfgvendor_del_csi_station_handler,
		.maxattr = RDK_VENDOR_ATTR_MAX
	},

	{
		{
			.vendor_id = OUI_COMCAST,
			.subcmd = RDK_VENDOR_NL80211_SUBCMD_GET_CSI_STATION
		},
		.policy = wl_cfgvendor_get_csi_station_policy,
		.flags = WIPHY_VENDOR_CMD_NEED_WDEV | WIPHY_VENDOR_CMD_NEED_NETDEV,
		.doit = wl_cfgvendor_get_csi_station_handler,
		.maxattr = RDK_VENDOR_ATTR_MAX
	},

	{
		{
			.vendor_id = OUI_COMCAST,
			.subcmd = RDK_VENDOR_NL80211_SUBCMD_SET_CSI
		},
		.policy = wl_cfgvendor_set_csi_policy,
		.flags = WIPHY_VENDOR_CMD_NEED_WDEV | WIPHY_VENDOR_CMD_NEED_NETDEV,
		.doit = wl_cfgvendor_set_csi_handler,
		.maxattr = RDK_VENDOR_ATTR_MAX
	},

	{
		{
			.vendor_id = OUI_COMCAST,
			.subcmd = RDK_VENDOR_NL80211_SUBCMD_GET_CSI
		},
		.policy = wl_cfgvendor_get_csi_policy,
		.flags = WIPHY_VENDOR_CMD_NEED_WDEV | WIPHY_VENDOR_CMD_NEED_NETDEV,
		.doit = wl_cfgvendor_get_csi_handler,
		.maxattr = RDK_VENDOR_ATTR_MAX
	},
	{
		{
			.vendor_id = OUI_COMCAST,
			.subcmd = RDK_VENDOR_NL80211_SUBCMD_MSGLEVEL
		},
		.policy = wl_cfgvendor_msglevel_policy,
		.flags = WIPHY_VENDOR_CMD_NEED_WDEV | WIPHY_VENDOR_CMD_NEED_NETDEV,
		.doit = wl_cfgvendor_msglevel_handler,
		.maxattr = RDK_VENDOR_ATTR_MAX
	},
};

static const struct nl80211_vendor_cmd_info rdk_nl80211_vendor_events[] = {
	{ OUI_COMCAST, RDK_VENDOR_NL80211_SUBCMD_CSI_STATS },
};

/* Loadable Module Support for wiphy_vendor_command.
 *
 * This module shall be insmoded after wl.ko and dhd.ko to
 * override the wiphy_vendor_command.
 * Similarly, rmmod this module before wl.ko and dhd.ko to
 * restore the wiphy_vendor_command.
 */
#define MAX_VENDOR_DEVS		(4)

struct wireless_vendor_info {
	struct net_device *dev;
	int n_vendor_commands;					/* Original number of commands */
	struct wiphy_vendor_command const *vendor_commands;	/* Original vendor commands */
	struct wiphy_vendor_command *alloced_cmds;		/* Allcated vendor commands */
	int n_vendor_events;					/* Original number of events */
	struct nl80211_vendor_cmd_info const *vendor_events;	/* Original vendor events */
	struct nl80211_vendor_cmd_info *alloced_events;		/* Allcated vendor events */
} _wifidevs[MAX_VENDOR_DEVS];

static void __exit
rdk_nlhal_module_exit(void)
{
	int i;
	struct net_device *dev;
	struct wireless_dev *wdev;
	struct wiphy *wiphy;

	read_lock(&dev_base_lock);

	/* Find the wiphy devices */
	dev = first_net_device(&init_net);
	while (dev) {
		if ((wdev = dev->ieee80211_ptr) == NULL || (wiphy = wdev->wiphy) == NULL)
			goto next_dev;

		for (i = 0; i < MAX_VENDOR_DEVS; i++) {
			if (_wifidevs[i].dev == NULL)
				continue;
			if (_wifidevs[i].dev == dev ||
			    wiphy == _wifidevs[i].dev->ieee80211_ptr->wiphy) {
				/* Previously recorded dev or wiphy */
				break;
			}
		}
		if (i >= MAX_VENDOR_DEVS) {
			/* Not found */
			WL_TRACE(("%s: wifi dev %s is previously ignored\n",
				__FUNCTION__, dev->name));
			goto next_dev;
		}
		WL_DBG(("%s: wifi dev %s wiphy %px vendor (%d, %px)\n",
			__FUNCTION__, dev->name, wiphy,
			wiphy->n_vendor_commands, wiphy->vendor_commands));
		if (_wifidevs[i].dev != dev) {
			WL_TRACE(("%s: dev %s has the same wiphy %px as %s, ignored\n",
				__FUNCTION__, dev->name, wiphy,	_wifidevs[i].dev->name));
			goto next_dev;
		}

		/* Restore the original vendor command info */
		WL_DBG(("%s: restore (%d, %px) back to (%d, %px) slot %d for dev %s\n",
			__FUNCTION__, wiphy->n_vendor_commands, wiphy->vendor_commands,
			_wifidevs[i].n_vendor_commands, _wifidevs[i].vendor_commands,
			i, dev->name));
		wiphy->n_vendor_commands = _wifidevs[i].n_vendor_commands;
		wiphy->vendor_commands = _wifidevs[i].vendor_commands;
		wiphy->n_vendor_events = _wifidevs[i].n_vendor_events;
		wiphy->vendor_events = _wifidevs[i].vendor_events;
next_dev:
		dev = next_net_device(dev);
	}

	read_unlock(&dev_base_lock);

	for (i = 0; i < MAX_VENDOR_DEVS; i++) {
		if (_wifidevs[i].alloced_cmds != NULL) {
			/* Free the previously allocated memory */
			kfree(_wifidevs[i].alloced_cmds);
		}

		if (_wifidevs[i].alloced_events != NULL) {
			kfree(_wifidevs[i].alloced_events);
		}
	}
	memset(_wifidevs, 0, sizeof(_wifidevs));
}

static int __init
rdk_nlhal_module_init(void)
{
	int i, ret = 0;
	struct net_device *dev;
	struct wireless_dev *wdev;
	struct wiphy *wiphy;

	memset(_wifidevs, 0, sizeof(_wifidevs));
	rdk_csi_event_idx = 0;

	read_lock(&dev_base_lock);

	/* Find the wiphy devices */
	dev = first_net_device(&init_net);
	while (dev) {
		if ((wdev = dev->ieee80211_ptr) == NULL || (wiphy = wdev->wiphy) == NULL)
			goto next_dev;

		if (wiphy->n_vendor_commands == 0 && wiphy->vendor_commands == NULL) {
			/* The vendor commands are not enabled. Ignore this dev */
			goto next_dev;
		}
		for (i = 0; i < MAX_VENDOR_DEVS; i++) {
			if (_wifidevs[i].dev == NULL) {
				/* Available slot */
				break;
			}
			/* Check if the dev has the same wiphy as that in this slot */
			if (_wifidevs[i].dev->ieee80211_ptr->wiphy == wiphy) {
				/* Same wiphy is found, vendor commands already changed */
				WL_TRACE(("%s: dev %s has the same wiphy %px as %s, ignored\n",
					__FUNCTION__, dev->name, wiphy,	_wifidevs[i].dev->name));
				goto next_dev;
			}
		}
		if (i >= MAX_VENDOR_DEVS) {
			/* No slot is available */
			WL_ERR(("%s: no more slot for dev %s\n", __FUNCTION__, dev->name));
			break;
		}

		WL_DBG(("%s: wifi dev %s wiphy %px vendor (%d, %px)\n",
			__FUNCTION__, dev->name, wiphy,
			wiphy->n_vendor_commands, wiphy->vendor_commands));
		/* Store the original vendor command info */
		_wifidevs[i].dev = dev;
		_wifidevs[i].n_vendor_commands = wiphy->n_vendor_commands;
		_wifidevs[i].vendor_commands = wiphy->vendor_commands;
		_wifidevs[i].n_vendor_events = wiphy->n_vendor_events;
		_wifidevs[i].vendor_commands = wiphy->vendor_commands;

		if (wiphy->vendor_commands != NULL) {
			int num_cmds, size = sizeof(struct wiphy_vendor_command);
			struct wiphy_vendor_command *new_cmds;

			/* Allocate and copy the original cmds,
			 * then append the rdk_nl80211_vendor_cmds.
			 */
			num_cmds = wiphy->n_vendor_commands +
				ARRAY_SIZE(rdk_nl80211_vendor_cmds);
			new_cmds = kmalloc(num_cmds * size, GFP_KERNEL);
			if (new_cmds) {
				memcpy(new_cmds,
					wiphy->vendor_commands,
					wiphy->n_vendor_commands * size);
				memcpy(new_cmds + wiphy->n_vendor_commands,
					rdk_nl80211_vendor_cmds,
					sizeof(rdk_nl80211_vendor_cmds));
				_wifidevs[i].alloced_cmds = new_cmds;
				wiphy->vendor_commands = new_cmds;
				wiphy->n_vendor_commands = num_cmds;
			} else {
				WL_ERR(("%s: alloc failed, keep orig cmds (%d, %px) for dev %s\n",
					__FUNCTION__, wiphy->n_vendor_commands,
					wiphy->vendor_commands, dev->name));
			}
		} else {
			/* Simply assign the static const array to it */
			wiphy->vendor_commands = rdk_nl80211_vendor_cmds;
			wiphy->n_vendor_commands = ARRAY_SIZE(rdk_nl80211_vendor_cmds);
		}

		if (wiphy->vendor_events != NULL) {
			/* Allocate, append to the original events  */
			int num_events, size = sizeof(struct nl80211_vendor_cmd_info);
			struct nl80211_vendor_cmd_info *new_events;

			num_events = wiphy->n_vendor_events +
				ARRAY_SIZE(rdk_nl80211_vendor_events);
			new_events = kmalloc(num_events * size, GFP_KERNEL);
			if (new_events) {
				/* set csi event idx */
				rdk_csi_event_idx = wiphy->n_vendor_events;

				memcpy(new_events, wiphy->vendor_events,
					wiphy->n_vendor_events * size);
				memcpy(new_events + wiphy->n_vendor_events,
					rdk_nl80211_vendor_events,
					sizeof(rdk_nl80211_vendor_events));

				_wifidevs[i].alloced_events = new_events;
				wiphy->vendor_events = new_events;
				wiphy->n_vendor_events = num_events;
			} else {
				WL_ERR(("%s: alloc failed, keep orig events (%d, 0x%px) "
					"for dev %s\n",
					__FUNCTION__, wiphy->n_vendor_events,
					wiphy->vendor_events, dev->name));
			}
		} else {
			/* assign the static const array */
			wiphy->vendor_events = rdk_nl80211_vendor_events;
			wiphy->n_vendor_events = ARRAY_SIZE(rdk_nl80211_vendor_events);
			rdk_csi_event_idx = 0;
		}
		WL_DBG(("%s: vndr cmds (%d, %px) events(%d, 0x%px idx:%d)slot %d for dev %s\n",
			__FUNCTION__, wiphy->n_vendor_commands, wiphy->vendor_commands,
			wiphy->n_vendor_events, wiphy->vendor_events, rdk_csi_event_idx,
			i, dev->name));
next_dev:
		dev = next_net_device(dev);
	}

	read_unlock(&dev_base_lock);

#ifdef BCM_WLMLO
	(void)wlhal_mlo_init();
#endif /* BCM_WLMLO */

	return ret;
}

module_exit(rdk_nlhal_module_exit);
module_init(rdk_nlhal_module_init);

MODULE_LICENSE("GPL and additional rights");
