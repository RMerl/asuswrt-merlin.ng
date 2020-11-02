/*
 * wl_feature.c
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
 * $Id: wl_feature.c $
 */
/* */
#if defined(USE_CFG80211)
#include <net/rtnetlink.h>
#endif // endif
#include <bcmutils.h>
#include <wldev_common.h>
#if defined(WL_CFG80211)
#include <wl_cfg80211.h>
#endif // endif
#include <wl_core.h>

#ifndef strtoul
#define strtoul(nptr, endptr, base) bcm_strtoul((nptr), (endptr), (base))
#endif // endif

#if defined(OEM_ANDROID) && defined(SOFTAP)
extern bool ap_cfg_running;
extern bool ap_fw_loaded;
#endif // endif

#ifdef PKT_FILTER_SUPPORT
#include <dngl_stats.h>
/* Pkt filte enable control */
extern uint dhd_pkt_filter_enable;
/* Pkt filter mode control */
extern uint dhd_master_mode;
#endif // endif

#ifdef WLNDOE
/*
* Neighbor Discovery Offload: enable NDO feature
* Called  by ipv6 event handler when interface comes up/goes down
*/
int
wl_cfg80211_ndo_enable(struct bcm_cfg80211 *cfg, int ndo_enable)
{
	int retcode;

	struct net_device *ndev = bcmcfg_to_prmry_ndev(cfg);

	retcode = wldev_iovar_setint(ndev, "ndoe", ndo_enable);
	if (retcode)
		WL_ERR(("%s: failed to enabe ndo to %d, retcode = %d\n",
			__FUNCTION__, ndo_enable, retcode));
	else
		WL_TRACE(("%s: successfully enabed ndo offload to %d\n",
			__FUNCTION__, ndo_enable));

	return retcode;
}
#endif /* WLNDOE */
#ifdef WLTDLS
int wl_cfg80211_tdls_enable(struct bcm_cfg80211 *cfg,
	bool tdls_on, bool auto_on, struct ether_addr *mac)
{
	int ret = 0;
	struct net_device *ndev = bcmcfg_to_prmry_ndev(cfg);
	if (cfg)
		ret = _wl_tdls_enable(ndev, cfg->wlcore, tdls_on, auto_on, mac);
	else
		ret = BCME_ERROR;
	return ret;
}
int
wl_cfg80211_tdls_set_mode(struct bcm_cfg80211 *cfg, bool wfd_mode)
{
	int ret = 0;
	bool auto_on = false;
	uint32 mode =  wfd_mode;
	struct net_device *ndev = bcmcfg_to_prmry_ndev(cfg);

#ifdef CUSTOMER_HW4
	if (wfd_mode) {
		auto_on = false;
	} else {
		auto_on = true;
	}
#else
	auto_on = false;
#endif // endif
	ret = _wl_tdls_enable(ndev, cfg->wlcore, false, auto_on, NULL);
	if (ret < 0) {
		WL_ERR(("Disable tdls_auto_op failed. %d\n", ret));
		return ret;
	}

	if ((ret = wldev_iovar_setint(ndev, "tdls_wfd_mode", mode)) < 0) {
		WL_ERR(("%s: mode %d failed %d\n", __FUNCTION__, mode, ret));
		return ret;
	}

	ret = _wl_tdls_enable(ndev, cfg->wlcore, true, auto_on, NULL);
	if (ret < 0) {
		WL_ERR(("enable tdls_auto_op failed. %d\n", ret));
		return ret;
	}

	cfg->wlcore->tdls_mode = mode;
	return ret;
}

#endif /* WL_TDLS */

#ifdef PKT_FILTER_SUPPORT
void wl_cfg80211_enable_packet_filter(struct bcm_cfg80211 *cfg, int value)
{
	int i;

	WL_TRACE(("%s: enter, value = %d\n", __FUNCTION__, value));
	/* 1 - Enable packet filter, only allow unicast packet to send up */
	/* 0 - Disable packet filter */
	if (dhd_pkt_filter_enable && (!value ||
	    (wl_cfg80211_support_sta_mode(cfg) && !cfg->wlcore->dhcp_in_progress)))
	    {
		for (i = 0; i < cfg->wlcore->pktfilter_count; i++) {
#if defined(PKT_FILTER_SUPPORT) && !defined(GAN_LITE_NAT_KEEPALIVE_FILTER)
			if (value && (i == WL_ARP_FILTER_NUM) &&
				!_turn_on_arp_filter(cfg->wlcore, cfg->wlcore->op_mode)) {
				WL_TRACE(("Do not turn on ARP white list pkt filter:"
					"val %d, cnt %d, op_mode 0x%x\n",
					value, i, cfg->wlcore->op_mode));
				continue;
			}
#endif /* !GAN_LITE_NAT_KEEPALIVE_FILTER */
			wl_cfg80211_pktfilter_offload_enable(cfg, cfg->wlcore->pktfilter[i],
				value, dhd_master_mode);
		}
	}
}
#endif /* PKT_FILTER_SUPPORT */

#ifdef ARP_OFFLOAD_SUPPORT
void
wl_cfg80211_arp_offload_set(struct bcm_cfg80211 *cfg, int arp_mode)
{
	int retcode;
	struct net_device *ndev = bcmcfg_to_prmry_ndev(cfg);
	retcode = wldev_iovar_setint(ndev, "arp_ol", arp_mode);

	retcode = retcode >= 0 ? 0 : retcode;
	if (retcode)
		WL_TRACE(("%s: failed to set ARP offload mode to 0x%x, retcode = %d\n",
			__FUNCTION__, arp_mode, retcode));
	else
		WL_TRACE(("%s: successfully set ARP offload mode to 0x%x\n",
			__FUNCTION__, arp_mode));
}
void
wl_cfg80211_arp_offload_enable(struct bcm_cfg80211 *cfg, int arp_enable)
{
	int retcode;

	struct net_device *ndev = bcmcfg_to_prmry_ndev(cfg);
	retcode = wldev_iovar_setint(ndev, "arpoe", arp_enable);

	retcode = retcode >= 0 ? 0 : retcode;
	if (retcode)
		WL_TRACE(("%s: failed to enabe ARP offload to %d, retcode = %d\n",
			__FUNCTION__, arp_enable, retcode));
	else
		WL_TRACE(("%s: successfully enabed ARP offload to %d\n",
			__FUNCTION__, arp_enable));
	if (arp_enable) {
		uint32 version;
		retcode = wldev_iovar_getint(ndev, "arp_version", &version);
		if (retcode) {
			WL_INFORM(("%s: fail to get version (maybe version 1:retcode = %d\n",
				__FUNCTION__, retcode));
			cfg->wlcore->arp_version = 1;
		}
		else {
			WL_INFORM(("%s: ARP Version= %x\n", __FUNCTION__, version));
			cfg->wlcore->arp_version = version;
		}
	}
}
#endif /* ARP_OFFLOAD_SUPPORT */

#if defined(PKT_FILTER_SUPPORT) && !defined(GAN_LITE_NAT_KEEPALIVE_FILTER)
bool
_turn_on_arp_filter(struct wl_core_priv *wlcore, int op_mode)
{
	bool _apply = FALSE;

	/* In case of IBSS mode, apply arp pkt filter */
	if (op_mode & DHD_FLAG_IBSS_MODE) {
		_apply = TRUE;
		goto exit;
	}
	/* In case of P2P GO or GC, apply pkt filter to pass arp pkt to host */
	if ((wlcore->arp_version == 1) &&
		(op_mode & (DHD_FLAG_P2P_GC_MODE | DHD_FLAG_P2P_GO_MODE))) {
		_apply = TRUE;
		goto exit;
	}

exit:
	return _apply;
}
#endif /* PKT_FILTER_SUPPORT && !GAN_LITE_NAT_KEEPALIVE_FILTER */
#ifdef PKT_FILTER_SUPPORT
void
wl_cfg80211_pktfilter_offload_enable(struct bcm_cfg80211 *cfg,
		char *arg, int enable, int master_mode)
{
	char				*argv[8];
	int					i = 0;
	const char			*str;
	int					buf_len;
	int					str_len;
	char				*arg_save = 0, *arg_org = 0;
	int					rc;
	char				buf[32] = {0};
	wl_pkt_filter_enable_t	enable_parm;
	wl_pkt_filter_enable_t	* pkt_filterp;
	struct net_device *ndev = NULL;
	osl_t *osh;
	ndev = bcmcfg_to_prmry_ndev(cfg);
	if (!arg)
		return;

	osh = wl_get_pub_osh(cfg->pub);
	if (!(arg_save = MALLOC(osh, strlen(arg) + 1))) {
		WL_ERR(("%s: MALLOC failed\n", __FUNCTION__));
		goto fail;
	}
	arg_org = arg_save;
	memcpy(arg_save, arg, strlen(arg) + 1);

	argv[i] = bcmstrtok(&arg_save, " ", 0);

	i = 0;
	if (argv[i] == NULL) {
		WL_ERR(("No args provided\n"));
		goto fail;
	}

	str = "pkt_filter_enable";
	str_len = strlen(str);
	bcm_strncpy_s(buf, sizeof(buf) - 1, str, sizeof(buf) - 1);
	buf[ sizeof(buf) - 1 ] = '\0';
	buf_len = str_len + 1;

	pkt_filterp = (wl_pkt_filter_enable_t *)(buf + str_len + 1);

	/* Parse packet filter id. */
	enable_parm.id = htod32(strtoul(argv[i], NULL, 0));

	/* Parse enable/disable value. */
	enable_parm.enable = htod32(enable);

	buf_len += sizeof(enable_parm);
	memcpy((char *)pkt_filterp,
	       &enable_parm,
	       sizeof(enable_parm));
	/* Enable/disable the specified filter. */
	rc = wldev_ioctl_set(ndev, WLC_SET_VAR, buf, buf_len);
	rc = rc >= 0 ? 0 : rc;
	if (rc)
		WL_TRACE(("%s: failed to add pktfilter %s, retcode = %d\n",
		__FUNCTION__, arg, rc));
	else
		WL_TRACE(("%s: successfully added pktfilter %s\n",
		__FUNCTION__, arg));

	/* Contorl the master mode */
	rc = wldev_iovar_setint(ndev, "pkt_filter_mode", master_mode);
	rc = rc >= 0 ? 0 : rc;
	if (rc)
		WL_TRACE(("%s: failed to add pktfilter %s, retcode = %d\n",
		__FUNCTION__, arg, rc));

fail:
	if (arg_org)
		MFREE(osh, arg_org, strlen(arg) + 1);
}

#endif /* PKT_FILTER_SUPPORT */
