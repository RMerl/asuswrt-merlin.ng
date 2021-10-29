/*
<:copyright-BRCM:2013:DUAL/GPL:standard

   Copyright (c) 2013 Broadcom 
   All Rights Reserved

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License, version 2, as published by
the Free Software Foundation (the "GPL").

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.


A copy of the GPL is available at http://www.broadcom.com/licenses/GPLv2.php, or by
writing to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
Boston, MA 02111-1307, USA.

:>
*/
#ifndef _BCMENET_ETHTOOL_H_
#define _BCMENET_ETHTOOL_H_

extern const struct ethtool_ops bcm63xx_enet_ethtool_ops;
extern const struct ethtool_ops *bcmenet_private_ethtool_ops;

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
#define ET_PF_2_IFF(x)     (1 << (x))

#endif //_BCMENET_ETHTOOL_H_
