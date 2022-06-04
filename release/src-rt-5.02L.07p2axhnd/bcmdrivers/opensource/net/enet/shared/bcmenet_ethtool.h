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
#ifndef _BCMENET_ETHTOOL_H_
#define _BCMENET_ETHTOOL_H_

#include <linux/version.h>

extern const struct ethtool_ops bcm63xx_enet_ethtool_ops;
extern const struct ethtool_ops *bcmenet_private_ethtool_ops;


#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4,19,0))
enum {
	ET_PF_802_1Q_VLAN			= 0,
	ET_PF_EBRIDGE,
	ET_PF_BONDING,
	ET_PF_ISATAP,
	ET_PF_WAN_HDLC,
	ET_PF_XMIT_DST_RELEASE,
	ET_PF_DONT_BRIDGE,	
	ET_PF_DISABLE_NETPOLL,
	ET_PF_MACVLAN_PORT,
	ET_PF_BRIDGE_PORT,
	ET_PF_OVS_DATAPATH,
	ET_PF_TX_SKB_SHARING,
	ET_PF_UNICAST_FLT,
	ET_PF_TEAM_PORT,
	ET_PF_SUPP_NOFCS,
	ET_PF_LIVE_ADDR_CHANGE,
	ET_PF_MACVLAN,
	ET_PF_XMIT_DST_RELEASE_PERM,
	ET_PF_L3MDEV_MASTER,
	ET_PF_NO_QUEUE,
	ET_PF_OPENVSWITCH,
	ET_PF_L3MDEV_SLAVE,
	ET_PF_TEAM,
	ET_PF_RXFH_CONFIGURED,
	ET_PF_PHONY_HEADROOM,
	ET_PF_MACSEC,
	ET_PF_NO_RX_HANDLER,
	ET_PF_FAILOVER,
	ET_PF_FAILOVER_SLAVE,
	ET_PF_L3MDEV_RX_HANDLER,
#if defined(CONFIG_BCM_KF_TEMP_FIX)
//#if defined(CONFIG_BCM_KF_WANDEV)
	ET_PF_WANDEV,            /* avoid WAN bridge traffic leaking */
#endif
    ET_PF_MAX
};

#else
enum {
    ET_PF_802_1Q_VLAN = 0,
    ET_PF_EBRIDGE,
    ET_PF_SLAVE_INACTIVE,
    ET_PF_MASTER_8023AD,
    ET_PF_MASTER_ALB,
    ET_PF_BONDING,
    ET_PF_SLAVE_NEEDARP,
    ET_PF_ISATAP,
    ET_PF_MASTER_ARPMON,
    ET_PF_WAN_HDLC,
    ET_PF_XMIT_DST_RELEASE,
    ET_PF_DONT_BRIDGE,
    ET_PF_DISABLE_NETPOLL,
    ET_PF_MACVLAN_PORT,
    ET_PF_BRIDGE_PORT,
    ET_PF_OVS_DATAPATH,
    ET_PF_TX_SKB_SHARING,
    ET_PF_UNICAST_FLT,
    ET_PF_TEAM_PORT,
    ET_PF_SUPP_NOFCS,
    ET_PF_LIVE_ADDR_CHANGE,
    ET_PF_MACVLAN,
    ET_PF_XMIT_DST_RELEASE_PERM,
    ET_PF_IPVLAN_MASTER,
    ET_PF_IPVLAN_SLAVE,
#if defined(CONFIG_BCM_KF_WL)
    ET_PF_BCM_WLANDEV,
#endif
#if defined(CONFIG_BCM_KF_WANDEV)
    ET_PF_WANDEV,
#endif
#if defined(CONFIG_BCM_KF_VLAN)
    ET_PF_BCM_VLAN,
#endif
#if defined(CONFIG_BCM_KF_PPP)
    ET_PF_PPP,
#endif
#if defined(CONFIG_BCM_KF_RUNNER)
    ET_PF_RNR,
#endif
#if defined(CONFIG_BCM_KF_ENET_SWITCH)  
    ET_PF_HW_SWITCH,
    ET_PF_EXT_SWITCH,
#endif /* CONFIG_BCM_KF_ENET_SWITCH */
    ET_PF_MAX
};
#endif

#define ET_PF_2_IFF(x)     (1 << (x))

#endif //_BCMENET_ETHTOOL_H_
