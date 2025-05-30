/*
    Copyright (c) 2019 Broadcom
    All Rights Reserved

    <:label-BRCM:2019:DUAL/GPL:standard

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

#include <linux/version.h>
#include <linux/netdevice.h>
#include <linux/etherdevice.h>
#include <linux/skbuff.h>

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 19, 0))

#include <wl_pktc.h>
#include <dhd_br_d3lut.h>

#include "br_private.h"

/* The d3lut generate/update function for wlan DHD driver. */
void dhd_update_d3lut(struct net_device *dst_dev, struct sk_buff *skb)
{
	unsigned char *da = eth_hdr(skb)->h_dest;

	if (dhd_pktc_req_hook != NULL)
	{
		dhd_pktc_req_hook(PKTC_TBL_UPDATE, (unsigned long)da, (unsigned long)dst_dev, 0);
	}
}

/* The d3lut creation function for wlan DHD driver.
 * @skb: incoming packet.
 * @src_dev: device that the packet came from.
 * @dst_dev: device that the packet destined to.
 *
 * This function is shared for both downstream and upstream path.
 * The d3lut creation happens when
 *  1. An unicast packet is sent to wlan (downstream). The function is invoked in wl tx entrance
 *     (slow path). Packets forwarded by accelerator with d3lut generated chainidx/flowid can be
 *     chained together.
 *  2. An unicast packet is received from wlan (upstream) via bridge. The function is invoked in
 *     bridge to ensure packets without content modification can be chained.
 */
void
dhd_handle_br_d3lut(struct net_device *src_dev, struct net_device *dst_dev, struct sk_buff *skb)
{
	struct net_device *root_src_dev, *root_dst_dev;
	unsigned char *da = eth_hdr(skb)->h_dest;

#if defined(PKTC_TBL) || defined(BCM_PKTFWD)
	/* dhd module is not ready/loaded */
	if (dhd_pktc_req_hook == NULL)
		return;
#endif

	if (skbuff_bcm_ext_br_flood_get(skb))
		return;

	if (is_broadcast_ether_addr(da) || is_multicast_ether_addr(da))
		return;

	/* Get the root dest device and make sure that we
	 * are always transmitting to a root device
	 */
	root_dst_dev = netdev_path_get_root(dst_dev);
	root_src_dev = src_dev ? netdev_path_get_root(src_dev) : NULL;

	/* only create when dst_dev is wlan (downstream) or src_dev is wlan (upstream) */
	if (!is_netdev_wlan(root_dst_dev) && (root_src_dev && !is_netdev_wlan(root_src_dev)))
		return;

	if (!is_netdev_wan(root_dst_dev) && !is_netdev_wlan_nic(root_dst_dev))
	{
		/* Also check for non-WAN cases.
		 * For the Rx direction, VLAN cases are allowed as long
		 * as the packets are untagged.
		 *
		 * Tagged packets are not forwarded through the chaining
		 * path by WLAN driver. Tagged packets go through the
		 * flowcache path.
		 * see wlc_sendup_chain() function for reference.
		 *
		 * For the Tx direction, there are no VLAN interfaces
		 * created on wl device when LAN_VLAN flag is enabled
		 * in the build.
		 */
		/* Various path handled by PKTFWD
		 *                                              Dst
		 *----------------------------------------------------------------------------------
		 *                  |    LAN           VLAN_LAN      WLAN             VLAN_WLAN
		 *----------------------------------------------------------------------------------
		 *  S   LAN         |    Dont Care     Dont Care     Runner-Pktfwd    Runner-Pktfwd
		 *  R   VLAN_LAN    |    Dont Care     Dont Care     Runner-Pktfwd    Runner-Pktfwd
		 *  C   WLAN        |    PKTFWD        NO PKTFWD     PKTFWD           NO PKTFWD
		 *      VLAN_WLAN   |    NO PKTFWD     NO PKTFWD     NO PKTFWD        NO PKTFWD
		 *----------------------------------------------------------------------------------
		 */
#if defined(PKTC_TBL) || defined(BCM_PKTFWD)
		/* Update chaining table for DHD on the wl to switch direction only */
		dhd_update_d3lut(dst_dev, skb);
#endif
	}
}

EXPORT_SYMBOL(dhd_update_d3lut);

#endif /* kernel >= 4.19.0 */
