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
#include <linux/bcm_dslcpe_wlan_info.h>
#endif
#include <linux/bcm_netdev_path.h>
#if defined(CONFIG_BCM_RDPA_BRIDGE) || defined(CONFIG_BCM_RDPA_BRIDGE_MODULE)
#include <linux/br_fp.h>
#endif
#include <uapi/linux/bcm_maclimit.h>

#if defined(CONFIG_BCM_KF_NETDEV_EXT)
#if defined(CONFIG_BCM_MAP) || defined(CONFIG_BCM_MAP_MODULE)
#define NAT46_DEVICE_SIGNATURE 0x544e36dd
#endif

// only enable for these chips for now
#if defined(CONFIG_BCM94912) || defined(CONFIG_BCM963146) || defined(CONFIG_BCM963158) 
#define CONFIG_FCACHE_TX_THREAD
#endif
#endif

typedef int (*HardStartXmitArgsFuncP) (void *pNBuff,
                                       struct net_device *txdev_p,
                                       void *args);

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
#define BCM_IFF_VFRWD       (1 << 10)

/*bits BCM_IFF_SWACCEL_GENERIC & BCM_IFF_HWACCEL_GENERIC are 
 *used together as 2 bit value*/
#define BCM_IFF_ACCEL_GDX_RX (1 << 11)
#define BCM_IFF_ACCEL_GDX_TX (1 << 12)
#define BCM_IFF_ACCEL_GDX_HW (1 << 13)
#define BCM_IFF_ACCEL_GDX_DEBUG	(1 << 14)
#define BCM_IFF_ACCEL_TC_EGRESS (1 << 15)
#define BCM_IFF_ACCEL_TX_FKB (1 << 16)
#define BCM_IFF_ACCEL_FC_TX_THREAD (1 << 17)
#define BCM_IFF_DUMMY_DEV   (1 << 18)

/* ASUS defined */
#define BCM_IFF_SDN_IGNORE   (1 << 19)

#define BLOG_DEV_STAT_FLAG_INCLUDE_SW_UC    (1<<0) /* Include SW accelerated Unicast stats */
#define BLOG_DEV_STAT_FLAG_INCLUDE_HW_UC    (1<<1) /* Include HW accelerated Unicast stats */
#define BLOG_DEV_STAT_FLAG_INCLUDE_SW_MC    (1<<2) /* Include SW accelerated Multicast stats */
#define BLOG_DEV_STAT_FLAG_INCLUDE_HW_MC    (1<<3) /* Include HW accelerated Multicast stats */
#define BLOG_DEV_STAT_FLAG_INCLUDE_SW       (BLOG_DEV_STAT_FLAG_INCLUDE_SW_UC|BLOG_DEV_STAT_FLAG_INCLUDE_SW_MC)
#define BLOG_DEV_STAT_FLAG_INCLUDE_HW       (BLOG_DEV_STAT_FLAG_INCLUDE_HW_UC|BLOG_DEV_STAT_FLAG_INCLUDE_HW_MC)
#define BLOG_DEV_STAT_FLAG_INCLUDE_ALL      (BLOG_DEV_STAT_FLAG_INCLUDE_SW|BLOG_DEV_STAT_FLAG_INCLUDE_HW)

/* Info types to ask from different drivers */
typedef enum {
    BCM_NETDEV_TO_RDPA_PORT_OBJ,
} bcm_netdev_priv_info_type_t;
/* Output from driver corresponding to the info type */
typedef union {
    struct {
        void *rdpa_port_obj;
    } bcm_netdev_to_rdpa_port_obj;
} bcm_netdev_priv_info_out_t;

typedef	int (*bcm_netdev_priv_info_get_cb_fn_t)(struct net_device *dev, 
                                                bcm_netdev_priv_info_type_t info_type, 
                                                bcm_netdev_priv_info_out_t *info_out);

struct bcm_netdev_ext {
    uint16_t devid;
    unsigned int iff_flags;
    struct netdev_path path;
#if defined(CONFIG_BLOG)
	BlogStats_t blog_stats; /* Cummulative stats of accelerated flows */
	unsigned int blog_stats_flags; /* Blog stats collection property for the device */
    void *dev_xmit_args;
    wlan_client_get_info_t wlan_client_get_info; /* runner multicast acceleration hook */
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

#define netdev_vfrwd_set(_dev)        (_dev)->bcm_nd_ext.iff_flags |= BCM_IFF_VFRWD
#define is_netdev_vfrwd(_dev)         ((_dev)->bcm_nd_ext.iff_flags & BCM_IFF_VFRWD)

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

// for NETDEV_CHANGEUPPER
#define is_netdev_br_port_add(_dev, _ptr)   (netif_is_bridge_port(_dev) && ((struct netdev_notifier_changeupper_info *)(ptr))->linking)
#define is_netdev_br_port_del(_dev, _ptr)   (netif_is_bridge_port(_dev) && !((struct netdev_notifier_changeupper_info *)(ptr))->linking)
#define changeupper_get_upper(_dev, _ptr)   (((struct netdev_notifier_changeupper_info *)(ptr))->upper_dev)


#define netdev_accel_gdx_rx_set(_dev)        (_dev)->bcm_nd_ext.iff_flags |= BCM_IFF_ACCEL_GDX_RX
#define netdev_accel_gdx_rx_unset(_dev)      (_dev)->bcm_nd_ext.iff_flags &= ~BCM_IFF_ACCEL_GDX_RX
#define is_netdev_accel_gdx_rx(_dev)         ((_dev)->bcm_nd_ext.iff_flags & BCM_IFF_ACCEL_GDX_RX)

#define netdev_accel_gdx_tx_set(_dev)        (_dev)->bcm_nd_ext.iff_flags |= BCM_IFF_ACCEL_GDX_TX
#define netdev_accel_gdx_tx_unset(_dev)      (_dev)->bcm_nd_ext.iff_flags &= ~BCM_IFF_ACCEL_GDX_TX
#define is_netdev_accel_gdx_tx(_dev)         ((_dev)->bcm_nd_ext.iff_flags & BCM_IFF_ACCEL_GDX_TX)

#define netdev_hw_accel_gdx_set(_dev)        (_dev)->bcm_nd_ext.iff_flags |= BCM_IFF_ACCEL_GDX_HW
#define netdev_hw_accel_gdx_unset(_dev)      (_dev)->bcm_nd_ext.iff_flags &= ~BCM_IFF_ACCEL_GDX_HW
#define is_netdev_hw_accel_gdx(_dev)         ((_dev)->bcm_nd_ext.iff_flags & BCM_IFF_ACCEL_GDX_HW)

extern int (*bcm_netdev_gen_hwaccel_notfier_cb)(struct net_device *dev, 
		int event, int group);

static inline int bcm_netdev_gen_hwaccel_notfier(struct net_device *dev,
		int event, int group)
{
	if(bcm_netdev_gen_hwaccel_notfier_cb)
		return bcm_netdev_gen_hwaccel_notfier_cb(dev, event, group);
	else {
		printk(" Generic HW accel not supported \n");
		return -1;
	}

	return 0;
}               
#define netdev_accel_gdx_debug_set(_dev)        (_dev)->bcm_nd_ext.iff_flags |= BCM_IFF_ACCEL_GDX_DEBUG
#define netdev_accel_gdx_debug_unset(_dev)      (_dev)->bcm_nd_ext.iff_flags &= ~BCM_IFF_ACCEL_GDX_DEBUG
#define is_netdev_accel_gdx_debug(_dev)         ((_dev)->bcm_nd_ext.iff_flags & BCM_IFF_ACCEL_GDX_DEBUG)

#define netdev_accel_tc_egress_set(_dev)        (_dev)->bcm_nd_ext.iff_flags |= BCM_IFF_ACCEL_TC_EGRESS
#define netdev_accel_tc_egress_unset(_dev)      (_dev)->bcm_nd_ext.iff_flags &= ~BCM_IFF_ACCEL_TC_EGRESS
#define is_netdev_accel_tc_egress(_dev)         ((_dev)->bcm_nd_ext.iff_flags & BCM_IFF_ACCEL_TC_EGRESS)

#define netdev_accel_tx_fkb_set(_dev)        (_dev)->bcm_nd_ext.iff_flags |= BCM_IFF_ACCEL_TX_FKB
#define netdev_accel_tx_fkb_unset(_dev)      (_dev)->bcm_nd_ext.iff_flags &= ~BCM_IFF_ACCEL_TX_FKB
#define is_netdev_accel_tx_fkb(_dev)         ((_dev)->bcm_nd_ext.iff_flags & BCM_IFF_ACCEL_TX_FKB)

#if defined(CONFIG_FCACHE_TX_THREAD)
#define netdev_accel_fc_tx_thread_set(_dev)     (_dev)->bcm_nd_ext.iff_flags |= BCM_IFF_ACCEL_FC_TX_THREAD
#define netdev_accel_fc_tx_thread_unset(_dev)   (_dev)->bcm_nd_ext.iff_flags &= ~BCM_IFF_ACCEL_FC_TX_THREAD
#define is_netdev_accel_fc_tx_thread(_dev)      ((_dev)->bcm_nd_ext.iff_flags & BCM_IFF_ACCEL_FC_TX_THREAD)
#endif

#define netdev_dummy_dev_set(_dev)           (_dev)->bcm_nd_ext.iff_flags |= BCM_IFF_DUMMY_DEV
#define netdev_dummy_dev_unset(_dev)         (_dev)->bcm_nd_ext.iff_flags &= ~BCM_IFF_DUMMY_DEV
#define is_netdev_dummy_dev(_dev)            ((_dev)->bcm_nd_ext.iff_flags & BCM_IFF_DUMMY_DEV)

/* ASUS defined */
#define netdev_sdn_ignore_set(_dev)        (_dev)->bcm_nd_ext.iff_flags |= BCM_IFF_SDN_IGNORE
#define netdev_sdn_ignore_unset(_dev)      (_dev)->bcm_nd_ext.iff_flags &= ~BCM_IFF_SDN_IGNORE
#define is_netdev_sdn_ignore(_dev)         ((_dev)->bcm_nd_ext.iff_flags & BCM_IFF_SDN_IGNORE)

void bcm_netdev_ext_inherit(struct net_device *parent, struct net_device * child);

int bcm_attach_vlan_hook(struct net_device *dev);
void bcm_detach_vlan_hook(struct net_device *dev);

#define BCM_NETDEVOFFSETOF(member)  ((size_t)&((struct net_device*)0)->member)
#define BCM_NETDEV_EXTOFFSETOF(member)  ((size_t)&((struct net_device*)0)->bcm_nd_ext.member)

#define BCM_NETDEV_DEVID_MAX_BITS      (10)
#define BCM_NETDEV_DEVID_MAX_ENTRIES   (1 << BCM_NETDEV_DEVID_MAX_BITS)
struct net_device *bcm_get_netdev_by_id(uint16_t devid);
struct net_device *bcm_get_netdev_by_id_nohold(uint16_t devid);
void bcm_put_netdev_by_id(uint16_t devid);
char *bcm_get_netdev_name_by_id(uint16_t devid);
int16_t bcm_get_idx_from_id(uint16_t udevid);
bool bcm_is_devid_valid(uint16_t udevid);

#endif /* __BCM_NETDEVICE_H_INCLUDED__ */
