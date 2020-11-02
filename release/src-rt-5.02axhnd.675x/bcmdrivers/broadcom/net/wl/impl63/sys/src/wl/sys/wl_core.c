/*
 * wl_core.c
 *
 * Copyright (C) 2020, Broadcom. All Rights Reserved.
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
 *
 * <<Broadcom-WL-IPTag/Open:>>
 *
 * $Id: wl_core.c $
 */

/**
 * This source file is used both by NIC and DHD builds.
 */

#include <typedefs.h>

#include <osl.h>
#include <wl_dbg.h>
#if defined(BCMDONGLEHOST)
#include <dhd_linux.h>
#else
#include<wl_linux.h>
#endif // endif
#include<wldev_common.h>
#include <wl_core.h>

static void wl_set_corepriv(void *pub, struct wl_core_priv *core_priv);

#ifdef WL_CFG80211

static struct net_device *wl_get_primary_netdev(void *pub);
static void wl_cfg80211_wdev_free(struct net_device *ndev);

static struct
net_device *wl_get_primary_netdev(void *pub)
{
	struct net_device *ndev = NULL;
#if defined(BCMDONGLEHOST)
	dhd_pub_t *dhdp = (dhd_pub_t *)pub;
	ndev = dhd_linux_get_primary_netdev(dhdp);
#else
	wl_info_t *drv_info = (wl_info_t *)pub;
	ndev = drv_info->dev;
#endif // endif
	return ndev;
}

#endif /* WL_CFG80211 */

static void
wl_set_corepriv(void *pub, struct wl_core_priv *core_priv)
{
#if defined(BCMDONGLEHOST)
	dhd_pub_t *dhdp = (dhd_pub_t *)pub;
	dhdp->wlcore = core_priv;
#else
	wl_info_t *drv_info = (wl_info_t *)pub;
	drv_info->wlcore = core_priv;
#endif // endif
}

osl_t *
wl_get_pub_osh(void *pub)
{
	osl_t *osh;
#if defined(BCMDONGLEHOST)
	osh = ((dhd_pub_t *)pub)->osh;
#else
	osh = ((wl_info_t *)pub)->osh;
#endif // endif
	return osh;
}

void
wl_core_init(void *pub)
{
	struct wl_core_priv* core_priv;
	osl_t *osh;
#if defined(WL_CFG80211)
	struct net_device *ndev = NULL;
	struct bcm_cfg80211 *cfg = NULL;
#endif // endif
	osh = wl_get_pub_osh(pub);
	core_priv = MALLOCZ(osh, sizeof(struct wl_core_priv));
	if (core_priv == NULL)
		WL_ERROR(("%s: core_init  alloc failure\n", __FUNCTION__));

#if defined(WL_CFG80211)
	ndev = wl_get_primary_netdev(pub);
	cfg = wl_get_cfg(ndev);
	if (cfg != NULL)
		cfg->wlcore = core_priv;
#endif // endif
	wl_set_corepriv(pub, core_priv);
}

int
wl_core_deinit(void *pub)
{
	osl_t *osh;
	struct wl_core_priv* core_priv;
#if defined(BCMDONGLEHOST)
	dhd_pub_t *dhdp = (dhd_pub_t *)pub;
	core_priv = dhdp->wlcore;
#else
	wl_info_t *drv_info = (wl_info_t *)pub;
	core_priv = drv_info->wlcore;
#endif // endif
	if (!core_priv) {
		WL_TRACE(("%s: core priv not allocated\n", __FUNCTION__));
		return FALSE;
	}
	osh = wl_get_pub_osh(pub);
	MFREE(osh, core_priv, sizeof(*core_priv));
	return TRUE;
}

#if defined(BCMDONGLEHOST) || defined(WL_CFG80211)
bool
wl_is_associated(struct net_device *dev, int *retval)
{
	char bssid[6], zbuf[6];
	int ret = -1;

	bzero(bssid, 6);
	bzero(zbuf, 6);
	ret = wldev_ioctl_get(dev, WLC_GET_BSSID, &bssid, sizeof(bssid));
	/* XXX:AS!!! res can be: -17(BCME_NOTASSOCIATED),-22(BCME_NORESOURCE), and 0(OK)
	 * OK - doesn't mean associated yet, the returned bssid
	 * still needs to be checked for non zero array
	 */
	WL_TRACE((" %s WLC_GET_BSSID ioctl res = %d\n", __FUNCTION__, ret));

	if (ret == BCME_NOTASSOCIATED) {
		WL_TRACE(("%s: not associated! res:%d\n", __FUNCTION__, ret));
	}

	if (retval)
		*retval = ret;

	if (ret < 0)
		return FALSE;

	if ((memcmp(bssid, zbuf, ETHER_ADDR_LEN) == 0)) {
		WL_TRACE(("%s: WLC_GET_BSSID ioctl returned zero bssid\n", __FUNCTION__));
		return FALSE;
	}
	return TRUE;
}
#endif /* BCMDONGLEHOST || WL_CFG80211 */

#if defined(WL_CFG80211)
/**
 * @param rtnl_lock_reqd   Should be false if the call path leading to this function already
 *                         acquired the rtnetlink mutex.
 */
int
wl_cfg80211_register_if(void *pub,
        int ifidx, struct net_device *ndev, bool rtnl_lock_reqd)
{
	/* this calls into either dhd_cfg80211.c or wl_linux.c */
	return wl_register_interface(pub, ifidx, ndev, rtnl_lock_reqd);
}
#endif /* WL_CFG80211 */

#if defined(PROP_TXSTATUS) && defined(WL_CFG80211)
int
wl_cfg80211_wlfc_get_enable(struct bcm_cfg80211 *cfg, bool *val)
{
#if defined(BCMDONGLEHOST)
	dhd_pub_t *dhd;
	dhd = (dhd_pub_t *)(cfg->pub);
	return dhd_wlfc_get_enable(dhd, val);
#else
	return 0;
#endif // endif
}
#endif /* PROP_TXSTATUS && WL_CFG80211 */

#ifdef WL_CFG80211
static void
wl_cfg80211_wdev_free(struct net_device *ndev)
{
	if (ndev && ndev->ieee80211_ptr) {
		kfree(ndev->ieee80211_ptr);
		ndev->ieee80211_ptr = NULL;
	}
}
#endif // endif

void
wl_netdev_free(struct net_device *ndev)
{

	if (ndev) {
#if (LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 24))
		MFREE(wl->osh, ndev->priv, sizeof(priv_link_t));
		MFREE(wl->osh, ndev, sizeof(struct net_device));
		ndev->priv = NULL;
		ndev = NULL;
#else
#ifdef WL_CFG80211
		wl_cfg80211_wdev_free(ndev);
#endif /* WL_CFG80211 */
#if (LINUX_VERSION_CODE < KERNEL_VERSION(4, 12, 0))
		free_netdev(ndev);
#endif /* KERNEL_VERSION < 4.12 */
#endif /* KERNEL_VERSION(2, 6, 24) */
	}
}

int
wl_get_fw_mode(void *drv_pub)
{
#if defined(BCMDONGLEHOST)
	dhd_pub_t *dhdp = (dhd_pub_t *)drv_pub;
	return dhd_get_fw_mode(dhdp->info);
#else
	return 0;
#endif // endif
}

/* Check if the mode supports STA MODE */
bool
wl_support_sta_mode(struct wl_core_priv *core_priv)
{
#ifdef  WL_CFG80211
	if (!(core_priv->op_mode & DHD_FLAG_STA_MODE))
		return FALSE;
	else
#endif /* WL_CFG80211 */
		return TRUE;
}

#ifdef WLTDLS
int
_wl_tdls_enable(struct net_device *ndev,
		struct wl_core_priv *wlcore, bool tdls_on, bool auto_on, struct ether_addr *mac)
{
	uint32 tdls = tdls_on;
	int ret = 0;
	uint32 tdls_auto_op = 0;
	uint32 tdls_idle_time = CUSTOM_TDLS_IDLE_MODE_SETTING;
	int32 tdls_rssi_high = CUSTOM_TDLS_RSSI_THRESHOLD_HIGH;
	int32 tdls_rssi_low = CUSTOM_TDLS_RSSI_THRESHOLD_LOW;
	BCM_REFERENCE(mac);
	if (!FW_SUPPORTED(wlcore, tdls))
		return BCME_ERROR;
	if (wlcore->tdls_enable == tdls_on)
		goto auto_mode;
	if ((ret = wldev_iovar_setint(ndev, "tdls_enable", tdls)) < 0) {
		WL_ERROR(("%s: tdls %d failed %d\n", __FUNCTION__, tdls, ret));
		goto exit;
	}
	wlcore->tdls_enable = tdls_on;
auto_mode:

	tdls_auto_op = auto_on;
	if ((ret = wldev_iovar_setint(ndev, "tdls_auto_op", tdls_auto_op)) < 0) {
		WL_ERROR(("%s: tdls %d failed %d\n", __FUNCTION__, tdls, ret));
		goto exit;
	}

	if (tdls_auto_op) {
		if ((ret = wldev_iovar_setint(ndev, "tdls_idle_time", tdls_idle_time)) < 0) {
			WL_ERROR(("%s: tdls %d failed %d\n", __FUNCTION__, tdls, ret));
			goto exit;
		}
		if ((ret = wldev_iovar_setint(ndev, "tdls_rssi_high", tdls_rssi_high)) < 0) {
			WL_ERROR(("%s: tdls %d failed %d\n", __FUNCTION__, tdls, ret));
			goto exit;
		}
		if ((ret = wldev_iovar_setint(ndev, "tdls_rssi_low", tdls_rssi_low)) < 0) {
			WL_ERROR(("%s: tdls %d failed %d\n", __FUNCTION__, tdls, ret));
			goto exit;
		}
	}

exit:
	return ret;
}
#endif /* WL_TDLS */
