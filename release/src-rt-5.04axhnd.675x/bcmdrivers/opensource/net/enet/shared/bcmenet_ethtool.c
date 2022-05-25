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

#include <linux/netdevice.h>
#include <linux/ethtool.h>
#include "bcmnet.h"
#include "bcmenet_ethtool.h"

#if defined(CONFIG_BCM_ETHTOOL)

/* Implementation specific Ethernet Driver private ethtool_ops
 * Driver should register private ethtool_ops here for any
 * functionality that differs from generic implementation */
const struct ethtool_ops *bcmenet_private_ethtool_ops = NULL;


static char ethtool_stats_strings[][ETH_GSTRING_LEN] = {
    [ET_TX_BYTES] =     "TxOctetsOK          ",
    [ET_TX_PACKETS] =   "TxPacketsOK         ",
    [ET_TX_ERRORS] =    "TxErrors            ",
    [ET_TX_CAPACITY] =  "TxCapacity          ",
    [ET_RX_BYTES] =     "RxOctetsOK          ",
    [ET_RX_PACKETS] =   "RxPacketsOK         ",
    [ET_RX_ERRORS] =    "RxErrors            "
};

static char ethtool_pflags_strings[ET_PF_MAX][ETH_GSTRING_LEN] = {
    [ET_PF_802_1Q_VLAN]             = "802_1Q_VLAN          ",
    [ET_PF_EBRIDGE]                 = "EBRIDGE              ",
    [ET_PF_BONDING]                 = "BONDING              ",
    [ET_PF_ISATAP]                  = "ISATAP               ",
    [ET_PF_WAN_HDLC]                = "WAN_HDLC             ",
    [ET_PF_XMIT_DST_RELEASE]        = "XMIT_DST_RELEASE     ",
    [ET_PF_DONT_BRIDGE]             = "DONT_BRIDGE          ",
    [ET_PF_DISABLE_NETPOLL]         = "DISABLE_NETPOLL      ",
    [ET_PF_MACVLAN_PORT]            = "MACVLAN_PORT         ",
    [ET_PF_BRIDGE_PORT]             = "BRIDGE_PORT          ",
    [ET_PF_OVS_DATAPATH]            = "OVS_DATAPATH         ",
    [ET_PF_TX_SKB_SHARING]          = "TX_SKB_SHARING       ",
    [ET_PF_UNICAST_FLT]             = "UNICAST_FLT          ",
    [ET_PF_TEAM_PORT]               = "TEAM_PORT            ",
    [ET_PF_SUPP_NOFCS]              = "SUPP_NOFCS           ",
    [ET_PF_LIVE_ADDR_CHANGE]        = "LIVE_ADDR_CHANGE     ",
    [ET_PF_MACVLAN]                 = "UNICAST_FLT          ",
    [ET_PF_XMIT_DST_RELEASE_PERM]   = "XMIT_DST_RELEASE_PERM",
    [ET_PF_L3MDEV_MASTER]           = "L3MDEV_MASTER        ",
    [ET_PF_NO_QUEUE]                = "NO_QUEUE             ",
    [ET_PF_OPENVSWITCH]             = "OPENVSWITCH          ",
    [ET_PF_L3MDEV_SLAVE]            = "L3MDEV_SLAVE         ",
    [ET_PF_TEAM]                    = "TEAM                 ",
    [ET_PF_RXFH_CONFIGURED]         = "RXFH_CONFIGURED      ",
    [ET_PF_PHONY_HEADROOM]          = "PHONY_HEADROOM       ",
    [ET_PF_MACSEC]                  = "MACSEC               ",
    [ET_PF_NO_RX_HANDLER]           = "NO_RX_HANDLER        ",
    [ET_PF_FAILOVER]                = "FAILOVER             ",
    [ET_PF_FAILOVER_SLAVE]          = "FAILOVER_SLAVE       ",
    [ET_PF_L3MDEV_RX_HANDLER]       = "L3MDEV_RX_HANDLER    ",
    [ET_PF_LIVE_RENAME_OK]          = "LIVE_RENAME_OK       ",
};

static int bcm63xx_get_ts_info(struct net_device *dev, struct ethtool_ts_info *info)
{
    if (bcmenet_private_ethtool_ops && bcmenet_private_ethtool_ops->get_ts_info) {
        return bcmenet_private_ethtool_ops->get_ts_info(dev, info);
    }
    return -EOPNOTSUPP;
}

static u32 bcm63xx_ethtool_get_priv_flags(struct net_device *dev)
{
    if (bcmenet_private_ethtool_ops && bcmenet_private_ethtool_ops->get_priv_flags) {
        return bcmenet_private_ethtool_ops->get_priv_flags(dev);
    }
    return (u32)(dev->priv_flags);
}

static void bcm63xx_ethtool_get_drvinfo(struct net_device *dev, struct ethtool_drvinfo *info)
{
    if (bcmenet_private_ethtool_ops && bcmenet_private_ethtool_ops->get_drvinfo) {
        return bcmenet_private_ethtool_ops->get_drvinfo(dev, info);
    }
}

static int bcm63xx_ethtool_get_ksettings(struct net_device *dev, struct ethtool_link_ksettings *ecmd)
{
    if (bcmenet_private_ethtool_ops && bcmenet_private_ethtool_ops->get_link_ksettings) {
        return bcmenet_private_ethtool_ops->get_link_ksettings(dev, ecmd);
    }
    return -EOPNOTSUPP;
}

static int bcm63xx_ethtool_set_ksettings(struct net_device *dev, const struct ethtool_link_ksettings *ecmd)
{
    if (bcmenet_private_ethtool_ops && bcmenet_private_ethtool_ops->set_link_ksettings) {
        return bcmenet_private_ethtool_ops->set_link_ksettings(dev, ecmd);
    }
    return -EOPNOTSUPP;
}

static int bcm63xx_get_ethtool_sset_count(struct net_device *dev, int sset)
{
    switch (sset) {
    case ETH_SS_STATS:
        return ARRAY_SIZE(ethtool_stats_strings);
    case ETH_SS_PRIV_FLAGS:
        return ARRAY_SIZE(ethtool_pflags_strings);
    default:
        return -EOPNOTSUPP;
    }
}

static void bcm63xx_get_ethtool_strings(struct net_device *netdev, u32 stringset, u8 *data)
{
    switch (stringset) {
    case ETH_SS_STATS:
        memcpy(data, *ethtool_stats_strings, sizeof(ethtool_stats_strings));
        break;
    case ETH_SS_PRIV_FLAGS:
        memcpy(data, *ethtool_pflags_strings, sizeof(ethtool_pflags_strings));
        break;
    }
}

static void bcm63xx_get_ethtool_stats(struct net_device *dev, struct ethtool_stats *stats, u64 *data)
{
    if (bcmenet_private_ethtool_ops && bcmenet_private_ethtool_ops->get_ethtool_stats) {
        return bcmenet_private_ethtool_ops->get_ethtool_stats(dev, stats, data);
    }
}

static void bcm63xx_ethtool_get_pauseparam(struct net_device *dev, struct ethtool_pauseparam *info)
{
    if (bcmenet_private_ethtool_ops && bcmenet_private_ethtool_ops->get_pauseparam) {
        bcmenet_private_ethtool_ops->get_pauseparam(dev, info);
    }
}

static int bcm63xx_ethtool_set_pauseparam(struct net_device *dev, struct ethtool_pauseparam *info)
{
    if (bcmenet_private_ethtool_ops && bcmenet_private_ethtool_ops->set_pauseparam) {
        return bcmenet_private_ethtool_ops->set_pauseparam(dev, info);
    }
    return -EOPNOTSUPP;
}

#if defined(CONFIG_BCM_ENERGY_EFFICIENT_ETHERNET)
static int bcm63xx_ethtool_get_eee(struct net_device *dev, struct ethtool_eee *info)
{
    if (bcmenet_private_ethtool_ops && bcmenet_private_ethtool_ops->get_eee) {
        return bcmenet_private_ethtool_ops->get_eee(dev, info);
    }
    return -EOPNOTSUPP;
}

static int bcm63xx_ethtool_set_eee(struct net_device *dev, struct ethtool_eee *info)
{
    if (bcmenet_private_ethtool_ops && bcmenet_private_ethtool_ops->set_eee) {
        return bcmenet_private_ethtool_ops->set_eee(dev, info);
    }
    return -EOPNOTSUPP;
}
#endif

static int bcm63xx_ethtool_get_module_info(struct net_device *dev, struct ethtool_modinfo *modinfo)
{
    if (bcmenet_private_ethtool_ops && bcmenet_private_ethtool_ops->get_module_info) {
        return bcmenet_private_ethtool_ops->get_module_info(dev, modinfo);
    }
    return -EOPNOTSUPP;
}

static int bcm63xx_ethtool_get_module_eeprom(struct net_device *dev, struct ethtool_eeprom *ee, u8 *data)
{
    if (bcmenet_private_ethtool_ops && bcmenet_private_ethtool_ops->get_module_eeprom) {
        return bcmenet_private_ethtool_ops->get_module_eeprom(dev, ee, data);
    }
    return -EOPNOTSUPP;
}

#define COMPILE_TIME_CHECK(condition) ((void)sizeof(char[1 - 2*(!(condition))]))

void bcm63xx_ethtool_dummy(void)
{
    // this function is never invoked.  It is being used as a placeholder for the
    // compile time check
    COMPILE_TIME_CHECK(ARRAY_SIZE(ethtool_stats_strings) == ET_MAX);  // these two should be kept in sync

    COMPILE_TIME_CHECK(ARRAY_SIZE(ethtool_pflags_strings) == ET_PF_MAX);  // these two should be kept in sync
    COMPILE_TIME_CHECK(ET_PF_2_IFF(ET_PF_802_1Q_VLAN) == IFF_802_1Q_VLAN);
    COMPILE_TIME_CHECK(ET_PF_2_IFF(ET_PF_EBRIDGE) == IFF_EBRIDGE);
    COMPILE_TIME_CHECK(ET_PF_2_IFF(ET_PF_BONDING) == IFF_BONDING);
    COMPILE_TIME_CHECK(ET_PF_2_IFF(ET_PF_ISATAP) == IFF_ISATAP);
    COMPILE_TIME_CHECK(ET_PF_2_IFF(ET_PF_WAN_HDLC) == IFF_WAN_HDLC);
    COMPILE_TIME_CHECK(ET_PF_2_IFF(ET_PF_XMIT_DST_RELEASE) == IFF_XMIT_DST_RELEASE);
    COMPILE_TIME_CHECK(ET_PF_2_IFF(ET_PF_DONT_BRIDGE) == IFF_DONT_BRIDGE);
    COMPILE_TIME_CHECK(ET_PF_2_IFF(ET_PF_DISABLE_NETPOLL) == IFF_DISABLE_NETPOLL);
    COMPILE_TIME_CHECK(ET_PF_2_IFF(ET_PF_MACVLAN_PORT) == IFF_MACVLAN_PORT);
    COMPILE_TIME_CHECK(ET_PF_2_IFF(ET_PF_BRIDGE_PORT) == IFF_BRIDGE_PORT);
    COMPILE_TIME_CHECK(ET_PF_2_IFF(ET_PF_OVS_DATAPATH) == IFF_OVS_DATAPATH);
    COMPILE_TIME_CHECK(ET_PF_2_IFF(ET_PF_TX_SKB_SHARING) == IFF_TX_SKB_SHARING);
    COMPILE_TIME_CHECK(ET_PF_2_IFF(ET_PF_UNICAST_FLT) == IFF_UNICAST_FLT);
    COMPILE_TIME_CHECK(ET_PF_2_IFF(ET_PF_TEAM_PORT) == IFF_TEAM_PORT);
    COMPILE_TIME_CHECK(ET_PF_2_IFF(ET_PF_SUPP_NOFCS) == IFF_SUPP_NOFCS);
    COMPILE_TIME_CHECK(ET_PF_2_IFF(ET_PF_LIVE_ADDR_CHANGE) == IFF_LIVE_ADDR_CHANGE);
    COMPILE_TIME_CHECK(ET_PF_2_IFF(ET_PF_MACVLAN) == IFF_MACVLAN);
    COMPILE_TIME_CHECK(ET_PF_2_IFF(ET_PF_XMIT_DST_RELEASE_PERM) == IFF_XMIT_DST_RELEASE_PERM);
    COMPILE_TIME_CHECK(ET_PF_2_IFF(ET_PF_L3MDEV_MASTER) == IFF_L3MDEV_MASTER);
    COMPILE_TIME_CHECK(ET_PF_2_IFF(ET_PF_NO_QUEUE) == IFF_NO_QUEUE);
    COMPILE_TIME_CHECK(ET_PF_2_IFF(ET_PF_OPENVSWITCH) == IFF_OPENVSWITCH);
    COMPILE_TIME_CHECK(ET_PF_2_IFF(ET_PF_L3MDEV_SLAVE) == IFF_L3MDEV_SLAVE);
    COMPILE_TIME_CHECK(ET_PF_2_IFF(ET_PF_TEAM) == IFF_TEAM);
    COMPILE_TIME_CHECK(ET_PF_2_IFF(ET_PF_RXFH_CONFIGURED) == IFF_RXFH_CONFIGURED);
    COMPILE_TIME_CHECK(ET_PF_2_IFF(ET_PF_PHONY_HEADROOM) == IFF_PHONY_HEADROOM);
    COMPILE_TIME_CHECK(ET_PF_2_IFF(ET_PF_MACSEC) == IFF_MACSEC);
    COMPILE_TIME_CHECK(ET_PF_2_IFF(ET_PF_NO_RX_HANDLER) == IFF_NO_RX_HANDLER);
    COMPILE_TIME_CHECK(ET_PF_2_IFF(ET_PF_FAILOVER) == IFF_FAILOVER);
    COMPILE_TIME_CHECK(ET_PF_2_IFF(ET_PF_FAILOVER_SLAVE) == IFF_FAILOVER_SLAVE);
	COMPILE_TIME_CHECK(ET_PF_2_IFF(ET_PF_L3MDEV_RX_HANDLER) == IFF_L3MDEV_RX_HANDLER);
    COMPILE_TIME_CHECK(ET_PF_2_IFF(ET_PF_LIVE_RENAME_OK) == IFF_LIVE_RENAME_OK);
}

const struct ethtool_ops bcm63xx_enet_ethtool_ops = {
    .get_drvinfo =          bcm63xx_ethtool_get_drvinfo,
    .get_link_ksettings =   bcm63xx_ethtool_get_ksettings,
    .set_link_ksettings =   bcm63xx_ethtool_set_ksettings,
    .get_ethtool_stats =    bcm63xx_get_ethtool_stats,
    .get_sset_count =       bcm63xx_get_ethtool_sset_count,
    .get_strings =          bcm63xx_get_ethtool_strings,
    .get_link     =         ethtool_op_get_link,
    .get_priv_flags =       bcm63xx_ethtool_get_priv_flags,
    .get_ts_info =          bcm63xx_get_ts_info,
    .get_pauseparam =       bcm63xx_ethtool_get_pauseparam,
    .set_pauseparam =       bcm63xx_ethtool_set_pauseparam,
#if defined(CONFIG_BCM_ENERGY_EFFICIENT_ETHERNET)
    .get_eee =              bcm63xx_ethtool_get_eee,
    .set_eee =              bcm63xx_ethtool_set_eee,
#endif
    .get_module_info =      bcm63xx_ethtool_get_module_info,
    .get_module_eeprom =    bcm63xx_ethtool_get_module_eeprom,
};

#endif // ETHTOOL_SUPPORT

