#ifndef __BCM_NETDEVICE_H_INCLUDED__
#define __BCM_NETDEVICE_H_INCLUDED__


/*
<:copyright-BRCM:2013:DUAL/GPL:standard

   Copyright (c) 2013 Broadcom 
   All Rights Reserved

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

#if defined(CONFIG_BLOG)
#include <linux/blog.h>
#if defined(CONFIG_BCM_WLAN_MODULE)
#include <linux/bcm_dslcpe_wlan_info.h>
#endif
#endif
#include <linux/bcm_netdev_path.h>
#if defined(CONFIG_BCM_RDPA_BRIDGE) || defined(CONFIG_BCM_RDPA_BRIDGE_MODULE)
#include <linux/br_fp.h>
#endif
#include <uapi/linux/bcm_maclimit.h>

#if defined(CONFIG_BCM_KF_NETDEV_EXT)
#if defined(CONFIG_BCM_NAT46) || defined(CONFIG_BCM_NAT46_MODULE)
#define NAT46_DEVICE_SIGNATURE 0x544e36dd
#endif
#endif

#define BCM_IFF_WANDEV      (1 << 0)
#define BCM_IFF_VLAN        (1 << 1)
#define BCM_IFF_PPP         (1 << 2)
#define BCM_IFF_HW_FDB      (1 << 3)  // this dev is enabled for hw MAC learning
#define BCM_IFF_HW_SWITCH   (1 << 4)
#define BCM_IFF_WLANDEV     (1 << 5)
#define BCM_IFF_BCM_DEV     (1 << 6)
#define BCM_IFF_WLANDEV_NIC (1 << 7)
#define BCM_IFF_WLANDEV_DHD (1 << 8)
#define BCM_IFF_MCAST_ROUTER (1 << 9)

#define BCM_IFF_TX_PAD       (1 << 20) /* pad tx len to even to workaround Starlink GEN2 connectivity issue */

#define BLOG_DEV_STAT_FLAG_INCLUDE_SW_UC    (1<<0) /* Include SW accelerated Unicast stats */
#define BLOG_DEV_STAT_FLAG_INCLUDE_HW_UC    (1<<1) /* Include HW accelerated Unicast stats */
#define BLOG_DEV_STAT_FLAG_INCLUDE_SW_MC    (1<<2) /* Include SW accelerated Multicast stats */
#define BLOG_DEV_STAT_FLAG_INCLUDE_HW_MC    (1<<3) /* Include HW accelerated Multicast stats */
#define BLOG_DEV_STAT_FLAG_INCLUDE_SW       (BLOG_DEV_STAT_FLAG_INCLUDE_SW_UC|BLOG_DEV_STAT_FLAG_INCLUDE_SW_MC)
#define BLOG_DEV_STAT_FLAG_INCLUDE_HW       (BLOG_DEV_STAT_FLAG_INCLUDE_HW_UC|BLOG_DEV_STAT_FLAG_INCLUDE_HW_MC)
#define BLOG_DEV_STAT_FLAG_INCLUDE_ALL      (BLOG_DEV_STAT_FLAG_INCLUDE_SW|BLOG_DEV_STAT_FLAG_INCLUDE_HW)

/* Info types to ask from different drivers */
typedef enum {
    BCM_NETDEV_TO_RDPA_IF,
} bcm_netdev_priv_info_type_t;
/* Output from driver corresponding to the info type */
typedef union {
    struct {
        int rdpa_if;
    } bcm_netdev_to_rdpa_if;
} bcm_netdev_priv_info_out_t;

typedef	int (*bcm_netdev_priv_info_get_cb_fn_t)(struct net_device *dev, 
                                                bcm_netdev_priv_info_type_t info_type, 
                                                bcm_netdev_priv_info_out_t *info_out);

struct bcm_netdev_ext {
    unsigned int iff_flags;
    struct netdev_path path;
#if defined(CONFIG_BLOG)
	BlogStats_t blog_stats; /* Cummulative stats of accelerated flows */
	unsigned int blog_stats_flags; /* Blog stats collection property for the device */
#if defined(CONFIG_BCM_WLAN_MODULE)
	/* runner multicast acceleration hook */
	wlan_client_get_info_t wlan_client_get_info;
#endif
#endif /* CONFIG_BLOG */
#if defined(CONFIG_BCM_RDPA_BRIDGE) || defined(CONFIG_BCM_RDPA_BRIDGE_MODULE)
    struct bcm_br_ext bcm_br_ext;
#endif
    struct mac_limit mac_limit;
    /* return 0-success, -1:failure (not supported or other error) */
    bcm_netdev_priv_info_get_cb_fn_t bcm_netdev_cb_fn;
};

#if defined(CONFIG_BLOG) && defined(CONFIG_BCM_WLAN_MODULE)
#define netdev_wlan_client_get_info(dev) ((dev)->bcm_nd_ext.wlan_client_get_info)
#else
#define netdev_wlan_client_get_info(dev) NULL
#endif

#define bcm_netdev_ext_field_get(dev, f) ((dev)->bcm_nd_ext.f)
#define bcm_netdev_ext_field_get_ptr(dev, f) (&(dev)->bcm_nd_ext.f)
#define bcm_netdev_ext_field_set(dev, f, val) ((dev)->bcm_nd_ext.f = val)

#define netdev_bcm_dev_set(_dev)    (_dev)->bcm_nd_ext.iff_flags |= BCM_IFF_BCM_DEV
#define netdev_bcm_dev_unset(_dev)  (_dev)->bcm_nd_ext.iff_flags &= ~BCM_IFF_BCM_DEV
#define is_netdev_bcm_dev(_dev)     ((_dev)->bcm_nd_ext.iff_flags & BCM_IFF_BCM_DEV)

#define netdev_wan_set(_dev)        (_dev)->bcm_nd_ext.iff_flags |= BCM_IFF_WANDEV
#define netdev_wan_unset(_dev)      (_dev)->bcm_nd_ext.iff_flags &= ~BCM_IFF_WANDEV
#define is_netdev_wan(_dev)         ((_dev)->bcm_nd_ext.iff_flags & BCM_IFF_WANDEV)

#define netdev_vlan_set(_dev)        (_dev)->bcm_nd_ext.iff_flags |= BCM_IFF_VLAN
#define is_netdev_vlan(_dev)         ((_dev)->bcm_nd_ext.iff_flags & BCM_IFF_VLAN)

#define netdev_ppp_set(_dev)        (_dev)->bcm_nd_ext.iff_flags |= BCM_IFF_PPP
#define is_netdev_ppp(_dev)         ((_dev)->bcm_nd_ext.iff_flags & BCM_IFF_PPP)

#define netdev_hw_fdb_set(_dev)     (_dev)->bcm_nd_ext.iff_flags |= BCM_IFF_HW_FDB
#define netdev_hw_fdb_unset(_dev)   (_dev)->bcm_nd_ext.iff_flags &= ~BCM_IFF_HW_FDB
#define is_netdev_hw_fdb(_dev)      ((_dev)->bcm_nd_ext.iff_flags & BCM_IFF_HW_FDB)

#define netdev_hw_switch_set(_dev)    (_dev)->bcm_nd_ext.iff_flags |= BCM_IFF_HW_SWITCH
#define netdev_hw_switch_unset(_dev)  (_dev)->bcm_nd_ext.iff_flags &= ~BCM_IFF_HW_SWITCH
#define is_netdev_hw_switch(_dev)     ((_dev)->bcm_nd_ext.iff_flags & BCM_IFF_HW_SWITCH)

#define netdev_wlan_set(_dev)       (_dev)->bcm_nd_ext.iff_flags |= BCM_IFF_WLANDEV
#define netdev_wlan_unset(_dev)     (_dev)->bcm_nd_ext.iff_flags &= ~BCM_IFF_WLANDEV
#define is_netdev_wlan(_dev)        ((_dev)->bcm_nd_ext.iff_flags & BCM_IFF_WLANDEV)

#define netdev_wlan_nic_set(_dev)       (_dev)->bcm_nd_ext.iff_flags |= BCM_IFF_WLANDEV_NIC
#define netdev_wlan_nic_unset(_dev)     (_dev)->bcm_nd_ext.iff_flags &= ~BCM_IFF_WLANDEV_NIC
#define is_netdev_wlan_nic(_dev)        ((_dev)->bcm_nd_ext.iff_flags & BCM_IFF_WLANDEV_NIC)

#define netdev_wlan_dhd_set(_dev)       (_dev)->bcm_nd_ext.iff_flags |= BCM_IFF_WLANDEV_DHD
#define netdev_wlan_dhd_unset(_dev)     (_dev)->bcm_nd_ext.iff_flags &= ~BCM_IFF_WLANDEV_DHD
#define is_netdev_wlan_dhd(_dev)        ((_dev)->bcm_nd_ext.iff_flags & BCM_IFF_WLANDEV_DHD)

#define netdev_mcastrouter_set(_dev)        (_dev)->bcm_nd_ext.iff_flags |= BCM_IFF_MCAST_ROUTER
#define netdev_mcastrouter_unset(_dev)      (_dev)->bcm_nd_ext.iff_flags &= ~BCM_IFF_MCAST_ROUTER
#define is_netdev_mcastrouter(_dev)         ((_dev)->bcm_nd_ext.iff_flags & BCM_IFF_MCAST_ROUTER)

#define netdev_tx_pad_set(_dev)         (_dev)->bcm_nd_ext.iff_flags |= BCM_IFF_TX_PAD
#define netdev_tx_pad_unset(_dev)       (_dev)->bcm_nd_ext.iff_flags &= ~BCM_IFF_TX_PAD
#define is_netdev_tx_pad(_dev)          ((_dev)->bcm_nd_ext.iff_flags & BCM_IFF_TX_PAD)

// for NETDEV_CHANGEUPPER
#define is_netdev_br_port_add(_dev, _ptr)   (netif_is_bridge_port(_dev) && ((struct netdev_notifier_changeupper_info *)(ptr))->linking)
#define is_netdev_br_port_del(_dev, _ptr)   (netif_is_bridge_port(_dev) && !((struct netdev_notifier_changeupper_info *)(ptr))->linking)
#define netdev_get_bridge_master(_dev, _ptr) (((struct netdev_notifier_changeupper_info *)(ptr))->upper_dev)

void bcm_netdev_ext_inherit(struct net_device *parent, struct net_device * child);

int bcm_attach_vlan_hook(struct net_device *dev);
void bcm_detach_vlan_hook(struct net_device *dev);

typedef int (*fwdcb_t)(void *skb, struct net_device *dev);
extern int bcm_iqos_enable_g;
extern int br_fwdcb_register(fwdcb_t fwdcb);
extern int enet_fwdcb_register(fwdcb_t fwdcb);
#if defined(CONFIG_BCM_SW_GSO)
extern void bcm_sw_gso_recycle_func(void *pNBuff, unsigned long context, uint32_t flags);
#endif
extern struct sk_buff *bcm_iqoshdl_wrapper(struct net_device *dev, void *pNBuff);

#define BROADSTREAM_IQOS_ENABLE(x) \
    (bcm_iqos_enable_g !=0)

#define BROADSTREAM_IQOS_SET_ENABLE(x) \
    (bcm_iqos_enable_g = x)

#define B_IQOS_LOG_SKBMARK(skb, ct, skbm) \
    do {((struct nf_conn *)(ct))->bcm_ext.cb.skb_mark = (skbm); \
	*(unsigned long *)&((skb)->fkb_mark) = (unsigned long)(ct);} while (0)

#define B_IQOS_RESTORE_SKBMARK(skb, ct) \
    do {(ct) = ((struct nf_conn *)(*(unsigned long *)&(skb)->mark)); \
	(skb)->mark = ((struct nf_conn *)(ct))->bcm_ext.cb.skb_mark;} while (0)

#define FKB_FRM_GSO ((void *)-1)

#define DEVQXMIT  (1 << 15)
#define PKTDEVQXMIT(skb) \
	(skbuff_bcm_ext_wlan_get((struct sk_buff*)(skb), pktc_flags) & DEVQXMIT)

#define PKTSETDEVQXMIT(skb)  \
	({ skbuff_bcm_ext_wlan_get((struct sk_buff*)(skb), pktc_flags) |= DEVQXMIT; })

#define PKTCLRDEVQXMIT(skb)  \
	({ skbuff_bcm_ext_wlan_get((struct sk_buff*)(skb), pktc_flags) &= ~DEVQXMIT; })

#define FC_PKTDONE  (1 << 14)
#define PKTISFCDONE(skb) \
	(skbuff_bcm_ext_wlan_get((struct sk_buff*)(skb), pktc_flags) & FC_PKTDONE)

#define PKTSETFCDONE(skb)  \
	({ skbuff_bcm_ext_wlan_get((struct sk_buff*)(skb), pktc_flags) |= FC_PKTDONE; })

#define PKTCLRFCDONE(skb)  \
	({ skbuff_bcm_ext_wlan_get((struct sk_buff*)(skb), pktc_flags) &= ~FC_PKTDONE; })


#endif /* __BCM_NETDEVICE_H_INCLUDED__ */
