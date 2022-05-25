
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

/*
 * WLAN shared module between system BSP and wlan driver.
 */

#include <linux/version.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/if_bridge.h>
#include <linux/netfilter_bridge.h>
#include <linux/netdevice.h>
#include <linux/notifier.h>
#include <net/switchdev.h>
#include <linux/bcm_log.h>
#include "br_private.h"

#include "wl_br_d3lut.h"
#include "dhd_br_d3lut.h"

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 19, 0))

#include <wl_pktc.h>

/* request command hooks to pktc/pktfwd */
unsigned long (*wl_pktc_req_hook)(int req_id, unsigned long param0, unsigned long param1, unsigned long param2) = NULL;
unsigned long (*dhd_pktc_req_hook)(int req_id, unsigned long param0, unsigned long param1, unsigned long param2) = NULL;
EXPORT_SYMBOL(wl_pktc_req_hook);
EXPORT_SYMBOL(dhd_pktc_req_hook);

/* pktc/pktfwd delete hooks */
void (*wl_pktc_del_hook)(unsigned long addr, struct net_device * net_device) = NULL;
void (*dhd_pktc_del_hook)(unsigned long addr, struct net_device * net_device) = NULL;
EXPORT_SYMBOL(wl_pktc_del_hook);
EXPORT_SYMBOL(dhd_pktc_del_hook);

/* bridge fdb hooks to check ageing */
int (*fdb_check_expired_wl_hook)(unsigned char *addr, struct net_device * net_device) = NULL;
int (*fdb_check_expired_dhd_hook)(unsigned char *addr, struct net_device * net_device) = NULL;
EXPORT_SYMBOL(fdb_check_expired_wl_hook);
EXPORT_SYMBOL(fdb_check_expired_dhd_hook);

static int wl_swdev_event(struct notifier_block *nb, unsigned long event, void *_neigh);
static struct notifier_block wl_swdev_notifier =
{
    .notifier_call = wl_swdev_event,
};

static int dhd_swdev_event(struct notifier_block *nb, unsigned long event, void *_neigh);
static struct notifier_block dhd_swdev_notifier =
{
    .notifier_call = dhd_swdev_event,
};

static int wl_swdev_event(struct notifier_block *unused, unsigned long event, void *ptr)
{
	struct net_device *dev = switchdev_notifier_info_to_dev(ptr);
	struct switchdev_notifier_fdb_info *fdb_info = ptr;

	/* function is for nic device */
	if ( wl_pktc_req_hook && is_netdev_wlan_nic(dev) )
		wl_pktc_req_hook(PKTC_TBL_BRIDGE_EVENT, (unsigned long)(fdb_info->addr), (unsigned long)dev, (unsigned long)event);

	return 0;
}

static int dhd_swdev_event(struct notifier_block *unused, unsigned long event, void *ptr)
{
	struct net_device *dev = switchdev_notifier_info_to_dev(ptr);
	struct switchdev_notifier_fdb_info *fdb_info = ptr;

	/* function is for dhd device */
	if ( dhd_pktc_req_hook && is_netdev_wlan_dhd(dev) )
		dhd_pktc_req_hook(PKTC_TBL_BRIDGE_EVENT, (unsigned long)(fdb_info->addr), (unsigned long)dev, (unsigned long)event);
	
	return 0;
}

#ifdef CONFIG_NETFILTER
static unsigned int bcm_br_handle_wl_d3lut(void *priv, struct sk_buff *skb, const struct nf_hook_state *state);
static const struct nf_hook_ops wl_br_nf_ops = {
        .hook           = bcm_br_handle_wl_d3lut,
        .pf             = NFPROTO_BRIDGE,
        .hooknum        = NF_BR_FORWARD,
        .priority       = NF_BR_PRI_FIRST
};

static unsigned int bcm_br_handle_wl_d3lut(void *priv, struct sk_buff *skb, const struct nf_hook_state *state)
{
	unsigned char *dest = eth_hdr(skb)->h_dest;

	if (is_broadcast_ether_addr(dest) || is_multicast_ether_addr(dest))
		return NF_ACCEPT;

#if defined(PKTC_TBL) || defined(BCM_PKTFWD)
		if (wl_pktc_req_hook != NULL)
			wl_handle_br_d3lut(state->in, state->out, skb);

		if (dhd_pktc_req_hook != NULL)
			dhd_handle_br_d3lut(state->in, state->out, skb);
#endif

	return NF_ACCEPT;
}
#endif

static int bcm_br_wl_query_bridgefdb(void *f)
{
	int exist_state = 0;
	struct net_bridge_fdb_entry *fdb = (struct net_bridge_fdb_entry *)f;
	struct net_device *dev = ((fdb->dst == NULL) || (fdb->dst->dev == NULL)) ? NULL : fdb->dst->dev;

	if (dev == NULL)
		return exist_state;

	/* get vlan root dev if any */
	dev = netdev_path_get_root(dev);

	/* dev could be either eth or wlan domain */
	if ((fdb_check_expired_wl_hook && !is_netdev_wlan_dhd(dev) &&
		(fdb_check_expired_wl_hook(fdb->key.addr.addr, dev) == 0)) ||
	    (fdb_check_expired_dhd_hook && !is_netdev_wlan_nic(dev) &&
		(fdb_check_expired_dhd_hook(fdb->key.addr.addr, dev) == 0)))
	{
		/* not expired, retain fdb */
		exist_state = 1;
	}
	return exist_state;
}

static int bcm_br_wl_update_bridgefdb(void *f)
{
	struct net_bridge_fdb_entry *fdb = (struct net_bridge_fdb_entry *)f;
	struct net_device *dev = ((fdb->dst == NULL) || (fdb->dst->dev == NULL)) ? NULL : fdb->dst->dev;

	if (dev == NULL)
		return 0;

	/* get vlan root dev if any */
	dev = netdev_path_get_root(dev);

	if (wl_pktc_del_hook && !is_netdev_wlan_dhd(dev))
	{
		/* address to be removed from this device */
		wl_pktc_del_hook((unsigned long)(fdb->key.addr.addr), dev);
	}

	if (dhd_pktc_del_hook && !is_netdev_wlan_nic(dev))
	{
		/* address to be removed from this device */
		dhd_pktc_del_hook((unsigned long)(fdb->key.addr.addr), dev);
	}

	return 0;
}

#if defined(CONFIG_BCM_OVS)
//For OVS bridge that not use linux net_bridge_fdb_entry to mantain FDB
static int bcm_br_wl_pktc_del_by_mac(void *f)
{
	if(f)
	{
		unsigned char *mac_addr=NULL;
		mac_addr = (unsigned char *)f;
		if (wl_pktc_del_hook)
		{
			/* address to be removed from this device */
			wl_pktc_del_hook((unsigned long)(mac_addr), NULL);
		}

		if (dhd_pktc_del_hook)
		{
			/* address to be removed from this device */
			dhd_pktc_del_hook((unsigned long)(mac_addr), NULL);
		}
	}
	return 0;
}
#endif /* CONFIG_BCM_OVS */
#endif /* kernel >= 4.19.0 */

static int __init
wlshared_module_init(void)
{
	printk("Loading wlshared Module...\n");

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 19, 0))	
	/* register swdev event */
	register_switchdev_notifier(&wl_swdev_notifier);
	register_switchdev_notifier(&dhd_swdev_notifier);

#ifdef CONFIG_NETFILTER
	/* register bridge d3lut handle */
	nf_register_net_hook(&init_net, &wl_br_nf_ops);
#endif

	/* register bridge ageing timeout handle */
	bcmFun_reg(BCM_FUN_ID_WLAN_QUERY_BRIDGEFDB, bcm_br_wl_query_bridgefdb);

	/* register bridge fdb update handle */
	bcmFun_reg(BCM_FUN_ID_WLAN_UPDATE_BRIDGEFDB, bcm_br_wl_update_bridgefdb);

#if defined(CONFIG_BCM_OVS)
	/* register pktc del handle : For OVS bridge that not use linux net_bridge_fdb_entry to mantain FDB */
	bcmFun_reg(BCM_FUN_ID_WLAN_PKTC_DEL_BY_MAC, bcm_br_wl_pktc_del_by_mac);
#endif /* CONFIG_BCM_OVS */
#endif

	return (0);
}

static void __exit
wlshared_module_exit(void)
{
	printk("Exiting wlshared Module\n");

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 19, 0))
	/* unregister swdev event */
	unregister_switchdev_notifier(&wl_swdev_notifier);
	unregister_switchdev_notifier(&dhd_swdev_notifier);

#ifdef CONFIG_NETFILTER
	/* unregister bridge d3lut handle */
	nf_unregister_net_hook(&init_net, &wl_br_nf_ops);
#endif

	/* deregister bridge ageing timeout handle */
	bcmFun_dereg(BCM_FUN_ID_WLAN_QUERY_BRIDGEFDB);

	/* deregister bridge fdb update handle */
	bcmFun_dereg(BCM_FUN_ID_WLAN_UPDATE_BRIDGEFDB);

#if defined(CONFIG_BCM_OVS)
	/* deregister pktc del handle */
	bcmFun_dereg(BCM_FUN_ID_WLAN_PKTC_DEL_BY_MAC);
#endif /* CONFIG_BCM_OVS */
#endif

	return;
}

module_init(wlshared_module_init);
module_exit(wlshared_module_exit);

MODULE_LICENSE("GPL and additional rights");
MODULE_DESCRIPTION("WLAN shared module between system BSP and wlan driver");

