/*
 * The Linux nl80211 WIFI HAL device driver
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

#include <bcmutils.h>
#include <wlioctl.h>
#include <bcmutils.h>
#include <bcmwifi_rspec.h>

#include "rdk_nl80211_hal.h"
#include <wldev_common.h>
#include <wl_cfg80211.h>
#include <wl_cfgvendor_common.h>

u32 wl_dbg_level = WL_DBG_ERR;
module_param(wl_dbg_level, int, 0);

static const struct nla_policy
wl_cfgvendor_get_station_policy[RDK_VENDOR_ATTR_MAX + 1] = {
	[RDK_VENDOR_ATTR_MAC] = { .type = NLA_EXACT_LEN, .len = ETH_ALEN },
};

static const struct nla_policy
wl_cfgvendor_get_station_list_policy[RDK_VENDOR_ATTR_MAX + 1] = {
};

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

	*standard = (sta_info->flags & WL_STA_HE_CAP) ? RDK_VENDOR_NL80211_STANDARD_AX :
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
	u32 buflen;
	struct net_device *ndev;
	char buf[WLC_IOCTL_MEDLEN] = {0};
	struct bcm_cfg80211 *cfg = wiphy_priv(wiphy);

	ndev = wdev_to_wlc_ndev(wdev, cfg);
	if (!ndev) {
		WL_ERR(("Failed to get ndev\n"));
		return BCME_ERROR;
	}

	strncpy(buf, "nrate", sizeof(buf) - 1);
	buflen = strlen(buf) + 1;
	ret = wldev_ioctl_get(ndev, WLC_GET_VAR, buf, sizeof(buf));
	if (ret < 0) {
		WL_ERR(("nrate ioctl failed, error: %d\n", ret));
		return ret;
	}

	memcpy(nrate, buf, sizeof(unsigned int));

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
	guard_interval_auto =	0x10,
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
	int		gi;		/* guard interval 400nsec/Auto = 1 800nsec = -1*/
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
 */
wifi_params_mcs_rate_t mcs_rate_tbl[] = {
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
};

static int
wl_cfgvendor_get_max_tx_rx_rate(struct wiphy *wiphy, struct wireless_dev *wdev, uint32 standard,
	uint8 bw, uint8 nss, u32 *max_tx_rate, u32 *max_rx_rate)
{
	int i, ret, gi, mcs_gi;

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

	for (i = 0; i < ARRAY_SIZE(mcs_rate_tbl); i++) {
		if (mcs_rate_tbl[i].oper_standard == standard &&
			mcs_rate_tbl[i].bw == bw &&
			mcs_rate_tbl[i].gi == mcs_gi &&
			mcs_rate_tbl[i].nss == nss) {
			*max_tx_rate = mcs_rate_tbl[i].rate;
			*max_rx_rate = mcs_rate_tbl[i].rate;
			break;
		}
	}

	if (i == ARRAY_SIZE(mcs_rate_tbl)) {
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
wl_cfgvendor_get_chanim_stats_ioctl(struct wiphy *wiphy, struct wireless_dev *wdev,
	wl_chanim_stats_t *chanim_stats)
{
	int ret;
	u32 buflen;
	struct net_device *ndev;
	char buf[WLC_IOCTL_MEDLEN] = {0};
	struct bcm_cfg80211 *cfg = wiphy_priv(wiphy);

	ndev = wdev_to_wlc_ndev(wdev, cfg);
	if (!ndev) {
		WL_ERR(("Failed to get ndev\n"));
		return BCME_ERROR;
	}

	strncpy(buf, "chanim_stats", sizeof(buf) - 1);
	buflen = strlen(buf) + 1;

	memset(chanim_stats, 0, sizeof(wl_chanim_stats_t));
	chanim_stats->buflen = htod32(sizeof(wl_chanim_stats_t));
	chanim_stats->count = 1;
	chanim_stats->version = WL_CHANIM_STATS_VERSION;
	memcpy(buf + buflen, chanim_stats, sizeof(wl_chanim_stats_t));

	ret = wldev_ioctl_get(ndev, WLC_GET_VAR, buf, sizeof(buf));
	if (ret < 0) {
		WL_ERR(("chanim stats ioctl failed, error: %d\n", ret));
		return ret;
	}

	memcpy(chanim_stats, buf, sizeof(wl_chanim_stats_t));

	return ret;
}

static int
wl_cfgvendor_get_station_reply(struct wiphy *wiphy, struct wireless_dev *wdev,
	sta_info_t *sta_info)
{
	int ret;
	int32 rssi;
	uint8 bw, nss;
	struct sk_buff *skb;
	wl_chanim_stats_t chanim_stats;
	uint32 standard, max_tx_rate, max_rx_rate;
	struct nl80211_sta_flag_update sta_flags = {};
	uint64 assoc_num, tx_bytes = 0, rx_bytes = 0, tx_packets = 0,
		tx_packets_ack = 0, tx_packets_no_ack = 0, rx_packets = 0,
		tx_errors = 0, rx_errors = 0, tx_retransmits = 0, tx_failed_retries = 0,
		tx_retries = 0, rx_retries = 0, tx_frames = 0;

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

	ret = wl_cfgvendor_get_chanim_stats_ioctl(wiphy, wdev, &chanim_stats);
	if (ret < 0) {
		WL_ERR(("Failed to get noise, error: %d\n", ret));
		goto error;
	}

	ret = nla_put_s32(skb, RDK_VENDOR_ATTR_STA_INFO_SNR,
		rssi - chanim_stats.stats[0].bgnoise);
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

	ret = wl_cfgvendor_get_max_tx_rx_rate(wiphy, wdev, standard, bw, nss, &max_tx_rate,
		&max_rx_rate);
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

	ret = cfg80211_vendor_cmd_reply(skb);
	if (ret < 0) {
		WL_ERR(("Failed to send reply, error: %d\n", ret));
		return ret;
	}

	return 0;

error:
	kfree_skb(skb);
	return ret;
}

static int
wl_cfgvendor_get_station_ioctl(struct wiphy *wiphy, struct wireless_dev *wdev, u8 *mac,
	sta_info_t *sta_info)
{
	int ret;
	u32 buflen;
	struct net_device *ndev;
	char buf[WLC_IOCTL_MEDLEN] = {0};
	struct bcm_cfg80211 *cfg = wiphy_priv(wiphy);

	ndev = wdev_to_wlc_ndev(wdev, cfg);
	if (!ndev) {
		WL_ERR(("Failed to get ndev\n"));
		return BCME_ERROR;
	}

	strncpy(buf, "sta_info", sizeof(buf) - 1);
	buflen = strlen(buf) + 1;
	memcpy(buf + buflen, mac, ETH_ALEN);

	ret = wldev_ioctl_get(ndev, WLC_GET_VAR, buf, sizeof(buf));
	if (ret < 0) {
		WL_ERR(("ioctl failed, error: %d\n", ret));
		return ret;
	}

	memcpy(sta_info, buf, sizeof(sta_info_t));

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

	ret = nla_put_u32(skb, RDK_VENDOR_ATTR_STATION_NUM, assoc_cnt);
	if (ret < 0) {
		WL_ERR(("Failed to add station num attribute, error: %d\n", ret));
		goto error;
	}

	nlattr = nla_nest_start(skb, RDK_VENDOR_ATTR_STATION_LIST);
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
		return ret;
	}

	return 0;

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
	struct wireless_dev *wdev, const void  *data, int len)
{
	int ret;
	char ioctl_buf[WLC_IOCTL_MEDLEN];

	ret = wl_cfgvendor_get_station_list_ioctl(wiphy, wdev, ioctl_buf, sizeof(ioctl_buf));
	if (ret < 0) {
		WL_ERR(("Failed to get station list, error: %d\n", ret));
		return ret;
	}

	ret = wl_cfgvendor_get_station_list_reply(wiphy, wdev, ioctl_buf);
	if (ret < 0) {
		WL_ERR(("Failed to send station list, error: %d\n", ret));
		return ret;
	}

	return 0;
}

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
		WL_ERR(("%s: wifi dev %s wiphy %px vendor (%d, %px)\n",
			__FUNCTION__, dev->name, wiphy,
			wiphy->n_vendor_commands, wiphy->vendor_commands));
		if (_wifidevs[i].dev != dev) {
			WL_TRACE(("%s: dev %s has the same wiphy %px as %s, ignored\n",
				__FUNCTION__, dev->name, wiphy,	_wifidevs[i].dev->name));
			goto next_dev;
		}

		/* Restore the original vendor command info */
		WL_ERR(("%s: restore (%d, %px) back to (%d, %px) slot %d for dev %s\n",
			__FUNCTION__, wiphy->n_vendor_commands, wiphy->vendor_commands,
			_wifidevs[i].n_vendor_commands, _wifidevs[i].vendor_commands,
			i, dev->name));
		wiphy->n_vendor_commands = _wifidevs[i].n_vendor_commands;
		wiphy->vendor_commands = _wifidevs[i].vendor_commands;
next_dev:
		dev = next_net_device(dev);
	}

	read_unlock(&dev_base_lock);

	for (i = 0; i < MAX_VENDOR_DEVS; i++) {
		if (_wifidevs[i].alloced_cmds != NULL) {
			/* Free the previously allocated memory */
			kfree(_wifidevs[i].alloced_cmds);
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

		WL_ERR(("%s: wifi dev %s wiphy %px vendor (%d, %px)\n",
			__FUNCTION__, dev->name, wiphy,
			wiphy->n_vendor_commands, wiphy->vendor_commands));
		/* Store the original vendor command info */
		_wifidevs[i].dev = dev;
		_wifidevs[i].n_vendor_commands = wiphy->n_vendor_commands;
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
		WL_ERR(("%s: vndr cmds (%d, %px) slot %d for dev %s\n",
			__FUNCTION__, wiphy->n_vendor_commands, wiphy->vendor_commands,
			i, dev->name));
next_dev:
		dev = next_net_device(dev);
	}

	read_unlock(&dev_base_lock);

	return ret;
}

module_exit(rdk_nlhal_module_exit);
module_init(rdk_nlhal_module_init);

MODULE_LICENSE("GPL and additional rights");
