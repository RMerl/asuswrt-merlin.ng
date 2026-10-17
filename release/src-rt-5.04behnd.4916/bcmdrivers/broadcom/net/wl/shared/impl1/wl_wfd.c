/*
    Copyright (c) 2017 Broadcom
    All Rights Reserved

    <:label-BRCM:2017:DUAL/GPL:standard

    Unless you and Broadcom execute a separate written software license
    agreement governing use of this software, this software is licensed
    to you under the terms of the GNU General Public License version 2
    (the "GPL"), available at http://www.broadcom.com/licenses/GPLv2.php,
    with the following added to such license:

       As a special exception, the copyright holders of this software give
       you permission to link this software with independent modules, and
       to copy and distribute the resulting executable under terms of your
       choice, provided that you also meet, for each linked independent
       module, the terms and conditions of the license of that module.
       An independent module is a module which is not derived from this
       software.  The special exception does not apply to any modifications
       of the software.

    Not withstanding the above, under no circumstances may you combine
    this software in any way with any other Broadcom software provided
    under a license other than the GPL, without Broadcom's express prior
    written consent.

    :>
*/

#if defined(BCM_WFD)

#if !defined(BCM_PKTFWD)

#include <linux/netdevice.h>

int wl_wfd_bind(struct net_device *net, unsigned int unit)
{
	/* dummy function for NO PKTFWD */
	return 0;
}

void wl_wfd_unbind(int wfd_idx)
{
	/* dummy function for NO PKTFWD */
	return;
}

int wl_wfd_registerdevice(int wfd_idx, struct net_device *dev)
{
	/* dummy function for NO PKTFWD */
	return 0;
}

int wl_wfd_unregisterdevice(int wfd_idx, struct net_device *dev)
{
	/* dummy function for NO PKTFWD */
	return 0;
}

#else /* BCM_PKTFWD */

#include <wl_dbg.h>
#include <wl_linux.h>
#include <wl_pktfwd.h>
#include <wl_wfd.h>

#if defined(BCM_AWL)
#include <bcm_archer.h>
#include <wl_awl.h>
#endif /* BCM_AWL */

#if defined(BCM_WLAN_FPI)
#include <fpi_wlan.h>
#endif /* BCM_WLAN_FPI */

#if !defined(BCM_PKTLIST)
#error "BCM_PKTFWD: BCM_PKTLIST is not defined"
#endif

#ifdef MC_RX_LOOPBACK_ENABLED
/* NIC->RUNNER->WFD->BACKHERE into this function */
/* intruct caller to continue when return 0 */
int wl_wfd_rx_mcasthandler(uint32_t wl_radio_idx, unsigned long skb_p, unsigned long dev_p)
{
	wl_if_t *wlif;
	int result;

	if (!dev_p)
		return 0;
	wlif = WL_DEV_IF((struct net_device *)dev_p);
	/* if runner can help with identify mc with info->is_ucast in wfd, no need to check here */
	if (is_multicast_ether_addr(PKTDATA(NULL, skb_p))) {

		WL_LOCK(wlif->wl);
		result = wl_pktfwd_rx_mcast_handler(wlif, (void *)skb_p);
		WL_UNLOCK(wlif->wl);

		return (result == BCME_OK) ? -1 : 0;
	} else {
		return 0;
	}
}
#endif /* MC_RX_LOOPBACK_ENABLED */

#if defined(BCM_WLAN_FPI)
/*
 * Get WiFi (NIC) metadata for flow provisioning interface driver
 *
 * Input:
 *  net:  net device,
 *  sa:   source mac address (not used),
 *  da:   destination mac address
 *  prio: priority (0-7)
 *  data: metadata pointer to fill
 *
 * Return:
 *   0:    Success, 'data' will be updated with valid metadata
 *  <0:    Failure, 'data' will be 0
 */
int wl_wfd_get_fpi_metadata(struct net_device *net, u8 *sa, u8 *da, u8 prio,
	u32 *data)
{
	fpi_wl_metadata_t metadata;
	int ifidx;
	wl_if_t *wlif = WL_DEV_IF(net);
	wl_info_t *wl;
	u32 chainIdx = PKTFWD_INVALID_CHAIN_IDX;


	metadata.wl = 0;
	*data = metadata.wl;

	/*
	 * Input parameter validation
	 */
	/* Check if priority parameter is valid */
	if (prio >= PKTFWD_PRIO_MAX) {
		WL_ERROR(("%s: Invalid prio (%d)\n", __FUNCTION__, prio));
		return BCME_ERROR;
	}

	/* Check if wl public context  is valid */
	wl = (wlif) ? wlif->wl : NULL;
	if (!wl || !wlif) {
		WL_ERROR(("%s: net device (0x%px) wl/wlif NULL\n", __FUNCTION__, net));
		return BCME_ERROR;
	}
	ifidx = wlif->subunit;

	/* Check if interface index is in range */
	if (ifidx >= WL_MAX_IFS) {
		WL_ERROR(("%s: Invalid net device (0x%px) invalid ifidx (%d)\n",
			__FUNCTION__, net, ifidx));
		return BCME_ERROR;
	}

	WL_INFORM(("wl[%d.%d]: %s(0x%px, "MACDBG", "MACDBG" , %d)\n", wl->unit,
		ifidx, __FUNCTION__, net, MAC2STRDBG(sa), MAC2STRDBG(da), prio));

	/* Broadcast/multicast DA is not supported */
	if (ETHER_ISMULTI(da)) {
		WL_ERROR(("%s: ["MACDBG"] is not unicast address\n",
			__FUNCTION__, MAC2STRDBG(da)));
		return BCME_UNSUPPORTED;
	}

	if ((WLAN_WFD_ENABLED(wl->wfd_idx)) && (wl_pktfwd_req_hook)) {
		/* Handle WAN as well as OVS case (including EAP case) */
		chainIdx = wl_pktfwd_req_hook(wl_pktfwd_req_ins_symbol_e, (unsigned long)da,
			(unsigned long)net, 0);

		if (chainIdx != PKTFWD_INVALID_CHAIN_IDX) {
			uint8_t prio4bit = 0;

			metadata.wfd.nic_ucast.is_tx_hw_acc_en = 1;
			metadata.wfd.nic_ucast.is_wfd = 1;
			/*
			 * Convert 3bit prio to 4bit priority for nic blog
			 * ignore iq_prio for now as skb is not available
			 */
			prio4bit = SET_WLAN_PRIORITY(prio4bit, prio);
			metadata.wfd.nic_ucast.priority = prio4bit;
			metadata.wfd.nic_ucast.is_chain = 1;
			metadata.wfd.nic_ucast.wfd_idx =
				((chainIdx & PKTFWD_WFD_IDX_BITMASK) >> PKTFWD_WFD_IDX_BITPOS);
			metadata.wfd.nic_ucast.chain_idx = chainIdx;
#if defined(BCM_WLHDR) && defined(BCM_PKTFWD_LLC_SNAP)
			metadata.wfd.nic_ucast.llcsnap_flag = WLHDR_SUPPORT(wl) ? 1 : 0;
#endif /* BCM_WLHDR && BCM_PKTFWD_LLC_SNAP */
		} else {
			WL_ERROR(("wl[%d.%d]: %s Unable to get da context\n",
					wl->unit, ifidx, __FUNCTION__));
			/* non-WFD/PKTFWD software acceleration is not supported */
			return BCME_ERROR;
		}
	} else {
		WL_ERROR(("wl[%d.%d]: %s nonWFD/PKTFWD mode not supported\n",
				wl->unit, ifidx, __FUNCTION__));
		/* non-WFD/PKTFWD software acceleration is not supported */
		return BCME_UNSUPPORTED;
	}

	*data = metadata.wl;

	WL_INFORM(("wl[%d.%d]: %s(0x%px, "MACDBG" , "MACDBG" , %d) = 0x%x\n",
		wl->unit, ifidx, __FUNCTION__, net, MAC2STRDBG(sa), MAC2STRDBG(da),
		prio, *data));

	return BCME_OK;
}
#endif /* BCM_WLAN_FPI */

int wl_wfd_bind(wl_info_t * wl)
{
	int unit = wl->unit;
	int wfd_idx = -1;
	pktlist_context_t *wl_pktlist_context;

	PKTFWD_TRACE("wl%d", unit);

	if (unit >= WL_PKTFWD_RADIOS) {
		WL_ERROR(("%s: dhd<%d> WFD not supported, disabled\n", __FUNCTION__, unit));
		wfd_idx = WLAN_WFD_DISABLE_IDX;
		goto wl_wfd_bind_failure;
	}

	wl_pktlist_context = (pktlist_context_t *)
	    wl_pktfwd_request(wl_pktfwd_req_pktlist_e, unit, 0, 0);

	if (wl_pktlist_context == PKTLIST_CONTEXT_NULL) {
		WL_ERROR(("%s: wl%d wl_pktlist_context NULL\n", __FUNCTION__, unit));
		goto wl_wfd_bind_failure;
	}
#if defined(BCM_AWL)
	{
		archer_wlan_radio_mode_t mode;

#if defined(BCM_WKB)
		mode = ARCHER_WLAN_RADIO_MODE_WKB | ARCHER_WLAN_RADIO_MODE_SPC_CREDITS;
#else /* ! BCM_WKB */

		mode = ARCHER_WLAN_RADIO_MODE_SKB;

#if defined(WL_SPC_CREDITS)
		mode |= ARCHER_WLAN_RADIO_MODE_SPC_CREDITS;
#endif /* WL_SPC_CREDITS */

#endif /* ! BCM_WKB */

		/* On Archer based platforms, Archer driver directly provides WFD services */
		wfd_idx = archer_wlan_bind(wl->dev, wl_pktlist_context,
			mode, (HOOK32) wl_pktfwd_xfer_callback, unit);
	}
#else /* !BCM_AWL */
	wfd_idx = wfd_bind(wl->dev, wl_pktlist_context,
	    WFD_WL_FWD_HOOKTYPE_SKB, false, (HOOK4PARM) NULL, (HOOK32) wl_pktfwd_xfer_callback,
#ifdef MC_RX_LOOPBACK_ENABLED
	    (HOOK3PARM) NULL, (HOOK3PARM) wl_wfd_rx_mcasthandler, unit);
#else
	    (HOOK3PARM) NULL, (HOOK3PARM) NULL, unit);
#endif

#endif /* !BCM_AWL */

#if defined(BCM_WLHDR) && defined(BCM_PKTFWD_LLC_SNAP)
	if (wl_pktlist_context->add_llcsnap_header) {
		wl->wlhdr_support = true;
	}
#endif /* BCM_WLHDR && BCM_PKTFWD_LLC_SNAP */

	if (wfd_idx == WFD_NOT_SUPPORTED) {
		/* Radio is not supported by WFD, lets disable WFD */
		wfd_idx = WLAN_WFD_DISABLE_IDX;
		PKTFWD_ERROR("%s: wl%d WFD not supported, disabled\n", __FUNCTION__, unit);
		goto wl_wfd_bind_failure;
	} else if (wfd_idx < 0) {
		wfd_idx = WLAN_WFD_INVALID_IDX;
		PKTFWD_ERROR("wl%d wfd_idx %d failure", unit, wfd_idx);
		goto wl_wfd_bind_failure;
	} else
		PKTFWD_PRINT("wl%d wfd_idx %d success", unit, wfd_idx);

	if (wfd_idx != unit) {
		PKTFWD_ERROR("wl%d wfd_idx %d mismatch", unit, wfd_idx);
		wfd_idx = WLAN_WFD_INVALID_IDX;
	} else {
		wfd_set_rx_wait_queue(wfd_idx, &wl->kthread_wqh);
		wl_pktfwd_wfd_ins(wl, wfd_idx);
#if defined(BCM_WLAN_FPI)
		fpi_register_wl_get_metadata(wl_wfd_get_fpi_metadata);
#endif /* BCM_WLAN_FPI */
	}

wl_wfd_bind_failure:
	PKTFWD_ASSERT(wfd_idx != WLAN_WFD_INVALID_IDX);

	return wfd_idx;
}

void wl_wfd_unbind(wl_info_t * wl)
{
	int wfd_idx = wl->wfd_idx;

	if (!WLAN_WFD_ENABLED(wfd_idx)) {
		/* Nothing to do */
		return;
	}

#if defined(BCM_WLAN_FPI)
	fpi_register_wl_get_metadata(NULL);
#endif /* BCM_WLAN_FPI */

#if defined(BCM_AWL)
	archer_wlan_unbind(wl->unit);
#else /* !BCM_AWL */
	wfd_unbind(wfd_idx, WFD_WL_FWD_HOOKTYPE_SKB);
#endif /* !BCM_AWL */
	WL_ERROR(("%s: Unbound WFD, wfd_idx=%d\n", __FUNCTION__, wfd_idx));

	wl_pktfwd_wfd_del(wl);
}

int wl_wfd_registerdevice(int wfd_idx, struct net_device *dev)
{
#if !defined(BCM_AWL)
	int ret = 0;
	int ifidx = WLAN_NETDEVPATH_SSID(netdev_path_get_hw_port(dev));

	if (!WLAN_WFD_ENABLED(wfd_idx)) {
		/* Nothing to do */
		return 0;
	}

	ret = wfd_registerdevice(wfd_idx, ifidx, dev);
	if (ret != 0)
		WL_ERROR(("%s failed wfd_idx %d\n", __FUNCTION__, wfd_idx));
	return ret;
#else /* BCM_AWL */
	return wl_awl_register_dev(dev);
#endif /* BCM_AWL */
}

int wl_wfd_unregisterdevice(int wfd_idx, struct net_device *dev)
{
#if !defined(BCM_AWL)
	int ret = 0;
	int ifidx = WLAN_NETDEVPATH_SSID(netdev_path_get_hw_port(dev));

	if (!WLAN_WFD_ENABLED(wfd_idx)) {
		/* Nothing to do */
		return 0;
	}

	ret = wfd_unregisterdevice(wfd_idx, ifidx);
	if (ret != 0)
		WL_ERROR(("%s failed wfd_idx %d\n", __FUNCTION__, wfd_idx));

	return ret;
#else /* BCM_AWL */
	return wl_awl_unregister_dev(dev);
#endif /* BCM_AWL */
}

#endif /* BCM_PKTFWD */

#endif /* BCM_WFD */
