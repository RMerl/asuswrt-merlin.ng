/*
 * wl_core.c
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
 *
 * <<Broadcom-WL-IPTag/Open:>>
 *
 * $Id: wl_core.c 825898 2023-06-02 02:10:24Z $
 */

/**
 * This source file is used both by NIC and DHD builds.
 */

#include <typedefs.h>

#include <osl.h>
#include <wl_dbg.h>
#include<wldev_common.h>
#include <wl_core.h>

#ifdef PROP_TXSTATUS /* a form of flow control between host and dongle */
#include <wlfc_proto.h>
#include <dhd_wlfc.h>
#endif

#ifdef WL_CFG80211
static void wl_cfg80211_wdev_free(struct net_device *ndev);
#endif /* WL_CFG80211 */

void
wl_core_init(wl_core_t *core)
{
	struct wl_core_priv* core_priv;
	osl_t *osh;
#if defined(WL_CFG80211)
	struct net_device *ndev = NULL;
	struct bcm_cfg80211 *cfg = NULL;
#endif
	osh = core->osh;
	core_priv = MALLOCZ(osh, sizeof(struct wl_core_priv));
	if (core_priv == NULL)
		WL_ERROR(("%s: core_init  alloc failure\n", __FUNCTION__));
#if defined(WL_CFG80211)
	ndev = core->ndev;
	cfg = wl_get_cfg(ndev);
	if (cfg != NULL)
		cfg->wlcore = core_priv;
#endif
	core->core_priv = core_priv;
}

int
wl_core_deinit(wl_core_t *core)
{
	osl_t *osh;
	struct wl_core_priv* core_priv;
	core_priv = core->core_priv;
	if (!core_priv) {
		WL_TRACE(("%s: core priv not allocated\n", __FUNCTION__));
		return FALSE;
	}
	osh = core->osh;
	MFREE(osh, core_priv, sizeof(*core_priv));
	return TRUE;
}

bool
wl_is_associated(struct net_device *dev, int *retval)
{
	char bssid[6], zbuf[6];
	int ret = -1;

	bzero(bssid, 6);
	bzero(zbuf, 6);
	ret = wldev_ioctl_get(dev, WLC_GET_BSSID, &bssid, sizeof(bssid));
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

#ifdef WL_CFG80211
static void
wl_cfg80211_wdev_free(struct net_device *ndev)
{
	if (ndev && ndev->ieee80211_ptr) {
		kfree(ndev->ieee80211_ptr);
		ndev->ieee80211_ptr = NULL;
	}
}
#endif

void
wl_netdev_free(struct net_device *ndev)
{

	if (ndev) {
#ifdef WL_CFG80211
		wl_cfg80211_wdev_free(ndev);
#endif /* WL_CFG80211 */
#if (LINUX_VERSION_CODE < KERNEL_VERSION(4, 12, 0))
		free_netdev(ndev);
#endif /* KERNEL_VERSION < 4.12 */
	}
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
