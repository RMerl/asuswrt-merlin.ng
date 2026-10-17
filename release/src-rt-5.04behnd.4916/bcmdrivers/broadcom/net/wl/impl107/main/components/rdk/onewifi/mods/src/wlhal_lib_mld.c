/*
 * The WIFI HAL lib for MLO/MLD.
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

#ifdef MODULE
#include <linux/netdevice.h>
#include <net/cfg80211.h>
#endif /* MODULE */

#include <wlioctl.h>
#include <bcmutils.h>
#ifdef MODULE
#include <wldev_common.h>
#include <wl_cfg80211.h>
#include <wl_cfgvendor_common.h>
#include "rdk_nl80211_hal.h"
#endif /* MODULE */

#include "wlhal_lib_mld.h"

#ifndef ether_etoa
#define ether_etoa(n, a)	bcm_ether_ntoa((struct ether_addr *)n, a)
#endif /* ether_etoa */

#ifndef WIFI_ERR
#define WIFI_ERR(...)		WL_ERR((__VA_ARGS__))
#endif /* WIFI_ERR */
#ifndef WIFI_DBG
#define WIFI_DBG(...)		WL_DBG((__VA_ARGS__))
#endif /* WIFI_DBG */

/***********************
*  802.11be MLO support.
***********************/
#define MAX_MLO_RADIOS				(4)

static int mlo_MAP = -1;			/* Main AP index */
static int mlo_config[MAX_MLO_RADIOS] =
{ -1, -1, -1, -1 };					/* wl_mlo_config values */
static int mlo_config_cnt = 0;		/* Number of MLO radios */
static int mlo_config_map = 0;		/* Bitmap, set if a radio is MLO enabled */
#ifndef MODULE
static int mlo_init_map = -1;		/* Bitmap, set if apply is called to init the radio */
#else
static struct net_device *mld_ndev[MAX_MLO_RADIOS][WL_MAX_NUM_SSID] = { };
#endif /* MODULE */

/*
 * The wifi_createVAP/RDK_VENDOR_NL80211_SUBCMD_SET_MLD can be called in any order.
 * We need store MLD configs to check MLDs are configured properly.
 */
/* Current MLD mac and config */
static unsigned char	_mld_mac[WL_MAXMLDUNIT][ETHER_ADDR_LEN] = { };
static mld_config_t	_mld_cfg[MAX_MLO_RADIOS][WL_MAX_NUM_SSID] = { };

/* Temporary buffered values for CMD_SET */
static unsigned char	tmp_mld_mac[WL_MAXMLDUNIT][ETHER_ADDR_LEN] = { {0} };
static mld_config_t	tmp_mld_cfg[MAX_MLO_RADIOS][WL_MAX_NUM_SSID] = { 0 };

#define MLD_MAC_SIZE	(sizeof(unsigned char) * WL_MAXMLDUNIT * ETHER_ADDR_LEN)
#define MLD_CFG_SIZE	(sizeof(mld_config_t) * MAX_MLO_RADIOS * WL_MAX_NUM_SSID)

/* Get from the buffered tmp_mld_mac[] */
unsigned char *
tmp_mld_mac_get(int mld_unit)
{
	return &tmp_mld_mac[mld_unit][0];
}

/* Get from the buffered tmp_mld_cfg[][] */
mld_config_t *
tmp_mld_cfg_get(int wl_unit, int wl_subunit)
{
	return &tmp_mld_cfg[wl_unit][wl_subunit];
}

/* Check if the mld_unit is unique for its radio in the buffered tmp_mld_cfg[][] */
int
tmp_mld_unit_is_unique(int wl_unit, int wl_subunit, int mld_unit)
{
	int j, k;

	for (j = 0, k = 0; j < WL_MAX_NUM_SSID; j++) {
		if (tmp_mld_cfg[wl_unit][j].mld_unit < 0)
			continue;

		if (tmp_mld_cfg[wl_unit][j].mld_unit == mld_unit && j != wl_subunit)
			k++;
	}
	return (k == 0) ? TRUE : FALSE;
}

void wlhal_mlo_dump(void)
{
	return;
}

static void
mld_cfg_reset(mld_config_t cfg[MAX_MLO_RADIOS][WL_MAX_NUM_SSID])
{
	int i, j;

	for (i = 0; i < MAX_MLO_RADIOS; i++) {
		for (j = 0; j < WL_MAX_NUM_SSID; j++) {
			cfg[i][j].flags = 0;
			cfg[i][j].enable = FALSE;
			cfg[i][j].mld_unit = -1;
		}
	}
}

#ifdef MODULE
int
wlhal_is_brcm_dev(struct net_device *dev)
{
	struct wiphy *wiphy = dev->ieee80211_ptr->wiphy;
	struct bcm_cfg80211 *cfg = wiphy_priv(wiphy);

	if ((cfg->magic >> 16) != 0x14e4)
		return FALSE;

	WIFI_DBG("[%s]: cfg=%px wiphy=%px magic=%x\n", dev->name, cfg, wiphy, cfg->magic);
	return TRUE;
}

/*
 * Save the ndev to mld_ndev[wl_unit][wl_subunit].
 * This is to support ndevs created dynamically after insmoding.
 */
int
wlhal_mld_ndev_reg(struct net_device *ndev, int wl_unit, int wl_subunit)
{
	/* Primary IFs are stored during wlhal_mlo_init() */
	if (wl_subunit <= 0)
		return 0;

	/* Store the virtual ndev */
	if (mld_ndev[wl_unit][wl_subunit] == NULL) {
		mld_ndev[wl_unit][wl_subunit] = ndev;
		WIFI_DBG("### %s: store ndev=%px for %s at [%d][%d]###\n", __FUNCTION__,
			ndev, ndev->name, wl_unit, wl_subunit);
		return 0;
	}
	if (mld_ndev[wl_unit][wl_subunit] != ndev) {
		WIFI_ERR("### %s: ndev=%px/%s found at %d.%d/%s but different ###\n", __FUNCTION__,
			ndev, ndev->name, wl_unit, wl_subunit, mld_ndev[wl_unit][wl_subunit]->name);
	}
	return -1;
}
#endif /* MODULE */

#ifndef MODULE
/* For CcspWifiSsp/wifi_hal/libwldm.so */
int
wlhal_mlo_init(void)
{
	int i;
	char *mld_str, *osifname, *wl_mlo_config = nvram_kget("wl_mlo_config");
	char buf[WLC_IOCTL_MEDLEN];

	if (wl_mlo_config == NULL) {
		WIFI_DBG("%s: wl_mlo_config is not set.\n", __FUNCTION__);
		return 0;
	}
	if (sscanf(wl_mlo_config, "%d %d %d %d", &mlo_config[0], &mlo_config[1],
	    &mlo_config[2], &mlo_config[3]) != 4) {
		WIFI_ERR("%s: invalid wl_mlo_config %s.\n", __FUNCTION__, wl_mlo_config);
		mlo_config[0] = mlo_config[1] = mlo_config[2] = mlo_config[3] = -1;
		return -2;
	}
	mlo_config_cnt = 0;
	/* Init and then populate _mld_cfg per nvram */
	mld_cfg_reset(_mld_cfg);
	for (i = 0; i < MAX_MLO_RADIOS; i++) {
		osifname =  wldm_get_radio_osifname(i);

		if (osifname == NULL)
			continue;

		if (mlo_config[i] > 3 || mlo_config[i] < -1) {
			WIFI_ERR("%s: invalid wl_mlo_config %d for %s.\n", __FUNCTION__,
				mlo_config[i], osifname);
			mlo_config[i] = -1;
		}

		/* Get cap to make sure this radio supports mlo */
		if (wl_iovar_get(osifname, "cap", (void *)buf, sizeof(buf)) ||
		    strstr(buf, " mlo ") == NULL) {
			mlo_config[i] = -1;
		}

		if (mlo_config[i] == -1) {
			/* Non-MLO radio */
			continue;
		}

		mlo_config_cnt++;
		wl_mld_cfg_load_from_nvram(i, _mld_cfg);
		mlo_config_map |= 1 << i;	/* Radio configured */
	}
	if (mlo_config_cnt <= 1) {
		WIFI_ERR("%s: Invalid mlo_config_cnt %d!\n", __FUNCTION__, mlo_config_cnt);
		return -3;
	}

	mlo_init_map = 0;	/* Clear bitmap to defer apply during init */
	for (i = 0; i < WL_MAXMLDUNIT; i++) {
		snprintf(buf, sizeof(buf), "mld%d_macaddr", i);
		mld_str = nvram_get(buf);
		if (mld_str)
			ether_atoe(mld_str, (unsigned char *)&_mld_mac[i]);
	}

	mld_cfg_reset(tmp_mld_cfg);	/* Reset tmp_mld_cfg[][] */
	memset(tmp_mld_mac, 0, MLD_MAC_SIZE);

	return 0;
}
#else
/* For OneWifi/rdk_wifi_hal/rdk_nlhal.ko */
int
wlhal_mlo_init(void)
{
	int wl_unit, wl_subunit, mlo_unit, ret = 0;
	struct net_device *dev;
	struct wireless_dev *wdev;
	struct wiphy *wiphy;

	mlo_config[0] = mlo_config[1] = mlo_config[2] = mlo_config[3] = mlo_MAP = -1;
	mlo_config_cnt = mlo_config_map = 0;
	mld_cfg_reset(_mld_cfg);	/* Reset _mld_cfg[][] */
	mld_cfg_reset(tmp_mld_cfg);	/* Reset tmp_mld_cfg[][] */
	memset(tmp_mld_mac, 0, MLD_MAC_SIZE);

	read_lock(&dev_base_lock);

	/* Find the wiphy devices */
	dev = first_net_device(&init_net);
	while (dev) {
		if ((wdev = dev->ieee80211_ptr) == NULL || (wiphy = wdev->wiphy) == NULL)
			goto next_dev;

		if (wlhal_is_brcm_dev(dev) == FALSE)
			goto next_dev;

		if (bcm_get_ifname_unit(dev->name, &wl_unit, &wl_subunit) < 0)
			goto next_dev;

#ifdef BCA_CPEROUTER
		read_unlock(&dev_base_lock);
#endif
		ret = wldev_iovar_getint(dev, "mlo_unit", &mlo_unit);
#ifdef BCA_CPEROUTER
		read_lock(&dev_base_lock);
#endif
		if (ret != 0 || mlo_unit == -1)	/* Non-MLO radio */
			goto next_dev;

		if (wl_subunit < 0) {
			wl_subunit = 0;
			mlo_config[wl_unit] = mlo_unit;
			mlo_config_cnt++;
			mlo_config_map |= 1 << wl_unit;	/* Radio configured */
		}
		if (mlo_MAP < 0 && mlo_unit == 0)
			mlo_MAP = wl_unit;
		mld_ndev[wl_unit][wl_subunit] = dev;
		WIFI_DBG("### %s: %s at mlo_ndev[%d][%d] ###\n", __FUNCTION__,
			dev->name, wl_unit, wl_subunit);
next_dev:
		dev = next_net_device(dev);
	}

	read_unlock(&dev_base_lock);

	WIFI_DBG("### %s: mlo_config=[%d %d %d %d] map=%0x count=%d ###\n", __FUNCTION__,
		mlo_config[0], mlo_config[1], mlo_config[2], mlo_config[3],
		mlo_config_map, mlo_config_cnt);
	if (mlo_config_map && mlo_config_cnt < 2) {
		WIFI_ERR("%s: MLO radios %d is less than 2!\n", __FUNCTION__, mlo_config_cnt);
	}
	return 0;
}
#endif /* MODULE */

/*
 * Populate
 *    nlinks[WL_MAXMLDUNIT]
 *    newcfg[MAX_MLO_RADIOS][WL_MAX_NUM_SSID]
 * according to _mld_cfg[][] and tmp_mld_cfg[][].
 *
 * Return < 0, error
 *   >= 0, modified count
 */
static int
mld_config_validate(int nlinks[WL_MAXMLDUNIT],
	mld_config_t newcfg[MAX_MLO_RADIOS][WL_MAX_NUM_SSID])
{
	int i, j, k, modified = 0, num_radios = MAX_MLO_RADIOS;

	memset(nlinks, 0, sizeof(int) * WL_MAXMLDUNIT);
	memcpy(newcfg, _mld_cfg, MLD_CFG_SIZE);

	for (j = 0; j < WL_MAX_NUM_SSID; j++) {
		for (i = 0; i < num_radios; i++) {
			if (mlo_config[i] == -1)	/* Non-MLO radio */
				continue;

			/* nlinks in current _mld_cfg[] */
			if (_mld_cfg[i][j].enable && _mld_cfg[i][j].mld_unit != -1) {
				k = _mld_cfg[i][j].mld_unit;
				nlinks[k]++;
			}

			/* Either no change or error, ignore it. */
			if (tmp_mld_cfg[i][j].flags != MLD_CFG_FLAG_SET)
				continue;

			/* Set with the same value as current one */
			if (tmp_mld_cfg[i][j].enable == _mld_cfg[i][j].enable &&
			    tmp_mld_cfg[i][j].mld_unit == _mld_cfg[i][j].mld_unit)
				continue;

			/* Adjust nlinks per tmp_mld_cfg[] */
			if (tmp_mld_cfg[i][j].enable) {
				newcfg[i][j].enable = TRUE;
				if ((k = tmp_mld_cfg[i][j].mld_unit) >= 0) {
					/* Valid mld_unit */
					nlinks[k]++;
					newcfg[i][j].mld_unit = k;
				}
				if (k >= 0 && k != _mld_cfg[i][j].mld_unit) {
					/* Enable but mld_unit has changed in tmp_mld_cfg */
					k = _mld_cfg[i][j].mld_unit;
					nlinks[k]--;
				}
			}
			else {
				if ((k = _mld_cfg[i][j].mld_unit) >= 0) {
					nlinks[k]--;
				}
				newcfg[i][j].enable = FALSE;
				newcfg[i][j].mld_unit = -1;
			}
			newcfg[i][j].flags |= MLD_CFG_FLAG_SET;
			modified++;
		}
	}

	/* Check nlinks, must >= 2 for any valid mld_unit */
	for (k = 0; k < WL_MAXMLDUNIT; k++) {
		if (nlinks[k] == 0)
			continue;
		if (nlinks[k] <= 1 || nlinks[k] > WL_MAX_MLO_LINKS) {
			WIFI_ERR("%s: invalid total links %d, mld_unit %d\n",
				__FUNCTION__, nlinks[k], k);
			return -2;
		}
	}
	WIFI_ERR("%s: ret %d, nlinks [%d %d %d %d %d %d %d %d]\n", __FUNCTION__, modified,
		nlinks[0], nlinks[1], nlinks[2], nlinks[3], nlinks[4], nlinks[5], nlinks[6], nlinks[7]);
	return modified;
}

/*
 * Check if the mld_mac[] is unique in the wifi subsystem, populate
 *	newmac[][] by combining _mld_mac and tmp_mld_mac.
 * per nlinks[] input when nlinks is valid.
 */
static int
mld_mac_validate(unsigned char newmac[WL_MAXMLDUNIT][ETHER_ADDR_LEN],
	int nlinks[WL_MAXMLDUNIT])
{
	int j, k, modified = 0;
	char macstr[ETHER_ADDR_STR_LEN];

	memset(newmac, 0, WL_MAXMLDUNIT * ETHER_ADDR_LEN);

	/* Populate newmac[][] */
	for (j = 0; j < WL_MAXMLDUNIT; j++) {
		if (nlinks[j] == 0)
			continue;

		if (memcmp(&tmp_mld_mac[j][0], &_mld_mac[j][0], ETHER_ADDR_LEN) == 0)
			continue;

		modified++;
		if (ETHER_ISNULLADDR(&tmp_mld_mac[j]))
			memcpy(&newmac[j][0], &_mld_mac[j][0], ETHER_ADDR_LEN);
		else
			memcpy(&newmac[j][0], &tmp_mld_mac[j][0], ETHER_ADDR_LEN);
	}

	/* Check duplicate MAC in newmac[][] */
	for (j = 0; j < WL_MAXMLDUNIT; j++) {
		if (nlinks[j] == 0)
			continue;

		for (k = 0; k < WL_MAXMLDUNIT; k++) {
			if (k == j || ETHER_ISNULLADDR(&newmac[j]))
				continue;

			if (memcmp(&newmac[j][0], &newmac[k][0], ETHER_ADDR_LEN) == 0) {
				WIFI_ERR("%s: duplicate MAC %s at %d and %d!\n",
					__FUNCTION__, ether_etoa(&newmac[j][0], macstr), j, k);
				return -1;
			}
		}
	}
	return modified;
}

int
wlhal_mld_mac_set(int mld_unit, char *mld_mac)
{
	char *pmac = tmp_mld_mac_get(mld_unit);

	if (memcmp(pmac, mld_mac, ETHER_ADDR_LEN) != 0) {
		char oldbuf[ETHER_ADDR_STR_LEN];
		char newbuf[ETHER_ADDR_STR_LEN];

		/* Override existing configured mac */
		WIFI_DBG("%s: update mld_mac[%d] from %s to %s!\n", __FUNCTION__,
			mld_unit, ether_etoa((unsigned char *)pmac, oldbuf),
			ether_etoa((unsigned char *)mld_mac, newbuf));
		memcpy(pmac, mld_mac, ETHER_ADDR_LEN);
	}
	return 0;
}

/*
 *  wl_unit: cf radioIndex
 *  wl_subunit: cf bssIndex
 */
int
wlhal_mld_cfg_set(int wl_unit, int wl_subunit, int mld_enable, int mld_unit)
{
	mld_config_t *pmld_cfg;

	if ((wl_unit < 0 || wl_unit >= MAX_MLO_RADIOS) ||
	    (wl_subunit < 0 || wl_subunit >= WL_MAX_NUM_SSID) ||
	    mlo_config[wl_unit] == -1)	/* Non-MLO radio */
		return -EINVAL;

	if (tmp_mld_unit_is_unique(wl_unit, wl_subunit, mld_unit) != TRUE) {
		WIFI_ERR("%s: mld_unit %d is not unique in radio %d!\n", __FUNCTION__, mld_unit, wl_unit);
		return -EINVAL;
	}

	pmld_cfg = tmp_mld_cfg_get(wl_unit, wl_subunit);
	pmld_cfg->enable = mld_enable ? 1 : 0;
	pmld_cfg->mld_unit = mld_unit;
	pmld_cfg->flags |= MLD_CFG_FLAG_SET;
	WIFI_DBG("### %s: Setting temp_mld_cfg[%d][%d]: enab %d mld_unit %d ###\n", __FUNCTION__,
		wl_unit, wl_subunit, pmld_cfg->enable, pmld_cfg->mld_unit);
	return 0;
}

#ifndef MODULE
/* For CcspWifiSsp/wifi_hal/libwldm.so */
static int
mld_config_apply(int nlinks[WL_MAXMLDUNIT],
	mld_config_t newcfg[MAX_MLO_RADIOS][WL_MAX_NUM_SSID],
	unsigned char newmac[WL_MAXMLDUNIT][ETHER_ADDR_LEN])
{
	int i, j, value, bss_enable;
	char nvnamebuf[IFNAMSIZ], *nvifname = nvnamebuf;

	/* Apply nvrams */
	for (i = 0; i < MAX_MLO_RADIOS; i++) {
		if (mlo_config[i] == -1)	/* Non-MLO radio */
			continue;

		for (j = 0; j < WL_MAX_NUM_SSID; j++) {
			if (j == 0)
				snprintf(nvnamebuf, sizeof(nvnamebuf), "wl%d", i);
			else
				snprintf(nvnamebuf, sizeof(nvnamebuf), "wl%d.%d", i, j);

			NVRAM_INT_GET(nvifname, "bss_enabled", &bss_enable);
			if (bss_enable == FALSE)
				continue;

			/* Config bss_mlo_mode */
			value = (newcfg[i][j].enable) ? 1 : 0;
			NVRAM_INT_SET(nvifname, "bss_mlo_mode", value);

			/* Config mld_unit */
			value = newcfg[i][j].mld_unit;
			NVRAM_INT_SET(nvifname, "mld_unit", value);
		}
	}

	/* Config mldX_macaddr nvram */
	for (j = 0; j < WL_MAXMLDUNIT; j++) {
		char macstr[ETHER_ADDR_STR_LEN], varname[32];

		if (ETHER_ISNULLADDR(&newmac[j]))
			continue;

		ether_etoa((unsigned char *)&newmac[j], macstr);
		snprintf(varname, sizeof(varname), "mld%d_macaddr", j);
		NVRAM_SET(varname, macstr);
	}

	BCM_REFERENCE(nlinks);

	return 0;
}
#else
static int
mld_config_apply(int nlinks[WL_MAXMLDUNIT],
	mld_config_t newcfg[MAX_MLO_RADIOS][WL_MAX_NUM_SSID],
	unsigned char newmac[WL_MAXMLDUNIT][ETHER_ADDR_LEN])
{
	struct net_device *ndev;
	int i, j, ret, value, set_mld_mac = FALSE;
	char *ifname, smbuf[WLC_IOCTL_SMLEN];

	/* Check if any mld_mac[] is to be configured */
	for (j = 0; j < WL_MAXMLDUNIT; j++) {
		 if (!ETHER_ISNULLADDR(&newmac[j]))	{
			/* Some mld_mac is set */
			set_mld_mac = TRUE;
			break;
		 }
	}

	/* Do "wl mlo_down" on MAP */
	value = 0;
	ret = wldev_ioctl_set(mld_ndev[mlo_MAP][0], WLC_MLO_DOWN, &value, sizeof(value));

	/* The bss_mlo_mode needs wl down state */
	/* Do "wl --all down" on MLO radios */
	value = 0;
	for (i = 0; i < MAX_MLO_RADIOS; i++) {
		if (mlo_config[i] == -1 || (ndev = mld_ndev[i][0]) == NULL)
			continue;
		ret = wldev_ioctl_set(ndev, WLC_DOWN, &value, sizeof(value));
		OSL_DELAY(1000);
	}
	for (i = 0; i < MAX_MLO_RADIOS; i++) {
		if (mlo_config[i] == -1 || (ndev = mld_ndev[i][0]) == NULL)
			continue;

		ifname = ndev->name;
		for (j = 0; j < WL_MAX_NUM_SSID; j++) {
			if (newcfg[i][j].flags == MLD_CFG_FLAG_SET) {
				/* Enable/Disable ML-IE from this BSS */
				value = (newcfg[i][j].enable) ? 1 : 0;
				ret = wldev_iovar_setint_bsscfg(ndev, "bss_mlo_mode", value, j);
				if (ret) {
					WIFI_ERR("%s: %d.%d set %s(%d) to %s ret %d!\n",
						__FUNCTION__, i, j, "bss_mlo_mode",
						value, ifname, ret);
				}

				/* Config mld_unit */
				value = newcfg[i][j].mld_unit;
				ret = wldev_iovar_setint_bsscfg(ndev, "mld_unit", value, j);
				if (ret) {
					WIFI_ERR("%s: %d.%d set %s(%d) to %s ret %d!\n",
						__FUNCTION__, i, j, "mld_unit",
						value, ifname, ret);
				}

				newcfg[i][j].flags = 0;
			}

			/* Config mld_nlinks per its mld_unit value */
			value = newcfg[i][j].mld_unit;
			if (value < 0) {
				if (newcfg[i][j].enable == FALSE)
					continue;
				value = 1;	/* Single link */
			}
			else if ((value = nlinks[value]) == 0)
				continue;

			ret = wldev_iovar_setint_bsscfg(ndev, "mld_nlinks", value, j);
			if (ret) {
				WIFI_ERR("%s: %d.%d set %s(%d) to %s ret %d!\n",
					__FUNCTION__, i, j, "mld_nlinks",
					value, ifname, ret);
			}
		}

		/* Config mld_macaddr, which is per radio, not per bss */
		if (set_mld_mac) {
			WIFI_ERR("### %s: set %s for %s!\n", __FUNCTION__, "mld_macaddr", ifname);
			ret = wldev_iovar_setbuf(ndev, "mld_macaddr", &newmac[0], MLD_MAC_SIZE,
				smbuf, sizeof(smbuf), NULL);
			if (ret)
				WIFI_ERR("%s: set %s to %s failed!\n", __FUNCTION__, "mld_macaddr", ifname);
		}
	}

	/* Do "wl --all up" on MLO radios */
	value = 0;
	for (i = 0; i < MAX_MLO_RADIOS; i++) {
		if (mlo_config[i] == -1 || (ndev = mld_ndev[i][0]) == NULL)
			continue;
		ifname = ndev->name;
		ret = wldev_ioctl_set(ndev, WLC_UP, &value, sizeof(value));
		if (ret)
			WIFI_ERR("%s: %s WLC_UP failed!\n", __FUNCTION__, ifname);
		OSL_DELAY(250000);
	}

	/* Do "wl mlo_up" on MAP */
	value = 0;
	ndev = mld_ndev[mlo_MAP][0];
	ifname = ndev->name;
	ret = wldev_ioctl_set(ndev, WLC_MLO_UP, &value, sizeof(value));
	if (ret) {
		WIFI_ERR("%s: %s WLC_MLO_UP ret %d!\n", __FUNCTION__, ifname, ret);
	}

	return 0;
}
#endif /* MODULE */

int
wlhal_mlo_apply(void)
{
	int ret, apply = 0, tot_nlinks[WL_MAXMLDUNIT];
	mld_config_t newcfg[MAX_MLO_RADIOS][WL_MAX_NUM_SSID];
	unsigned char newmac[WL_MAXMLDUNIT][ETHER_ADDR_LEN];

	if ((ret = mld_config_validate(tot_nlinks, newcfg)) <= 0)
		return ret;
	if (ret > 0)
		apply |= TRUE;

	if ((ret = mld_mac_validate(newmac, tot_nlinks)) < 0)
		return -2;
	if (ret > 0)
		apply |= TRUE;

	if (apply) {
		ret = mld_config_apply(tot_nlinks, newcfg, newmac);
		memcpy(_mld_cfg, newcfg, MLD_CFG_SIZE);
		memcpy(_mld_mac, tmp_mld_mac, MLD_MAC_SIZE);
	}
	mld_cfg_reset(tmp_mld_cfg);				/* Reset tmp_mld_cfg[][] */
	memset(tmp_mld_mac, 0, MLD_MAC_SIZE);	/* Reset tmp_mld_mad[][] */

	return ret;
}
